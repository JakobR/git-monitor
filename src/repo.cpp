#include "repo.h"
#include <QHash>
#include <QThread>
#include <QtConcurrent>
#include <algorithm>

Repo::Repo(QObject *parent)
    : QObject{parent}
{
    m_recheck_timer = new QTimer(this);
    m_recheck_timer->setInterval(m_recheck_interval);
    connect(m_recheck_timer, &QTimer::timeout, this, &Repo::startCheck);

    connect(&m_check_watcher, &QFutureWatcher<check_result_t>::finished, this, &Repo::checkCompleted);
}

RepoSettings const& Repo::settings() const
{
    return m_settings;
}

void Repo::updateSettings(RepoSettings new_settings)
{
    bool const was_enabled = m_enabled;
    if (m_enabled)
        disable();

    reset();
    m_settings = std::move(new_settings);

    if (was_enabled)
        enable();
    emit changed();
}

void Repo::setActivity(RepoActivity activity)
{
    if (m_activity == activity)
        return;
    m_activity = activity;
    // emit activityChanged();
}

void Repo::reset()
{
    m_status = RepoStatus::Unknown;
    setActivity(RepoActivity::Idle);
    m_statistics = RepoStatistics{};
    m_errors.clear();
}

void Repo::enable()
{
    if (m_enabled)
        return;
    m_enabled = true;

    qDebug() << "Enabling checking for repository " << m_settings.path;

    // initial check should be done immediately
    // TODO: add random delay to avoid filesystem burst? (maybe index * 0.5 seconds on startup)
    if (m_status == RepoStatus::Unknown)
        m_recheck_timer->setInterval(std::chrono::milliseconds(500));

    m_recheck_timer->start();

    // TODO: start filesystem watcher; see QFileSystemWatcher.
    // but there doesn't seem to be an efficient way to check whole subtrees for changes,
    // so maybe as a heuristic we simply watch the first two or three levels?
    // we should make sure that our checking does not trigger the watcher itself.
    // maybe we should watch the .git folder after all, because that will trigger detection of changes for unpushed/unmerged commits?
    // or the git index file, as well as the .git/refs folder?

    // QFileSystemWatcher* watcher = new QFileSystemWatcher(this);
    // watcher->addPath(m_settings.path());
    // ...

    emit changed();
}

void Repo::disable()
{
    if (!m_enabled)
        return;
    m_enabled = false;

    m_recheck_timer->stop();
    m_check_future.cancel();
    m_check_watcher.cancel();
    reset();

    emit changed();
}

void Repo::startCheck()
{
    if (activity() == RepoActivity::Checking)
        return;
    setActivity(RepoActivity::Checking);
    qDebug() << "Starting check for repository " << m_settings.path;

    m_check_future = QtConcurrent::run([this]() -> check_result_t {
        return check();
    });

    m_check_watcher.setFuture(m_check_future);

    emit changed();
}

// NOTE: this function runs in a separate thread
Repo::check_result_t Repo::check()
{
    RepoStatistics stats;
    QList<QString> errors;
    stats.timestamp = QDateTime::currentDateTime();
    qDebug() << "Checking repository " << m_settings.path;

    std::optional<git::repository> repo_opt;
    try {
        repo_opt = git::repository::open(m_settings.path.toStdString().c_str());
    }
    catch (std::exception const& e) {
        errors.push_back(tr("Unable to open repository: %1").arg(e.what()));
        return {stats, errors};  // there's nothing else we can do in this case
    }

    Q_ASSERT(repo_opt.has_value());
    git::repository& repo = *repo_opt;

    try {
        stats.uncommitted = repo.uncommitted_changes();
    }
    catch (std::exception const& e) {
        errors.push_back(tr("Unable to check uncommitted changes: %1").arg(e.what()));
    }

    try {
        stats.head_ahead_behind = repo.head_ahead_behind();
    }
    catch (std::exception const& e) {
        errors.push_back(tr("Unable to check HEAD ahead/behind: %1").arg(e.what()));
    }

    try {
        stats.total_ahead_behind = repo.total_ahead_behind();
    }
    catch (std::exception const& e) {
        errors.push_back(tr("Unable to check total ahead/behind: %1").arg(e.what()));
    }

    try {
        auto remote_state = repo.check_remote_state();
        stats.head_state = remote_state.head_state;
        stats.branches_outdated = remote_state.branches_outdated;
        for (auto const& error : remote_state.errors)
            errors.push_back(tr("Error checking remote state: %1").arg(QString::fromStdString(error)));
    }
    catch (std::exception const& e) {
        errors.push_back(tr("Unable to check remote state: %1").arg(e.what()));
    }

#ifdef QT_DEBUG
    QThread::sleep(1);  // sleep for 1 second to simulate a long-running operation
#endif

    return {stats, errors};
}

void Repo::checkCompleted()
{
    if (m_check_watcher.isCanceled())
        return;  // the canceller should reset the activity to Idle

    qDebug() << "Completed check for repository " << m_settings.path;
    auto [stats, errors] = m_check_watcher.result();
    m_statistics = stats;
    dropOldErrors(stats.timestamp);  // use the timestamp of the current check as base

    if (!errors.isEmpty()) {
        m_status = RepoStatus::Error;
        qDebug() << "Errors while checking repository " << m_settings.path << ":";
        for (auto const& error : errors) {
            qDebug() << "Error:" << error;
            m_errors.push_back({m_statistics.timestamp, error});
        }
        deduplicateErrors();
    }
    else {
        m_status = stats.isOk() ? RepoStatus::Ok : RepoStatus::DirtyOrOutdated;
    }

    setActivity(RepoActivity::Idle);

    // reset interval until next check
    m_recheck_timer->setInterval(m_recheck_interval);

    emit changed();
}

bool RepoStatistics::isOk() const
{
    if (uncommitted > 0)
        return false;
    // head stats are subsumed by the total stats so don't really need to check both
    if (head_ahead_behind && head_ahead_behind->ahead > 0)
        return false;
    if (head_ahead_behind && head_ahead_behind->behind > 0)
        return false;
    if (total_ahead_behind.ahead > 0)
        return false;
    if (total_ahead_behind.behind > 0)
        return false;
    // unknown and up_to_date are both ok
    if (head_state == git::branch_state::outdated)
        return false;
    if (head_state == git::branch_state::connection_error)
        return false;
    if (branches_outdated > 0)
        return false;
    return true;
}

void Repo::dropOldErrors(QDateTime const& now)
{
    // drop errors older than 1 hour
    auto it = std::remove_if(m_errors.begin(), m_errors.end(),
        [&now](auto const &error) { return error.timestamp.secsTo(now) > 3600; });
    m_errors.erase(it, m_errors.end());
}

void Repo::deduplicateErrors()
{
    // error message -> timestamp of latest occurrence
    QHash<QString, QDateTime> error_map;
    for (auto const& e : m_errors) {
        auto it = error_map.find(e.message);
        if (it == error_map.end())
            error_map.insert(e.message, e.timestamp);
        else if (e.timestamp > it.value())
            it.value() = e.timestamp;
    }

    // drop all earlier duplicate messages from the error list
    auto it = std::remove_if(m_errors.begin(), m_errors.end(),
        [&error_map](auto const &e) {
            Q_ASSERT(error_map.contains(e.message));
            return e.timestamp < error_map.value(e.message);
        });
    m_errors.erase(it, m_errors.end());
}
