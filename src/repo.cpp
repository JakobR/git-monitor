#include "repo.h"
#include <QtConcurrent>
#include <chrono>

Repo::Repo(QObject *parent)
    : QObject{parent}
{
    using namespace std::literals::chrono_literals;

    m_recheck_timer = new QTimer(this);
    auto recheck_interval = 5min;
    auto recheck_interval_ms = std::chrono::duration_cast<std::chrono::milliseconds>(recheck_interval);
    m_recheck_timer->setInterval(recheck_interval_ms);

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
    m_settings = new_settings;

    if (was_enabled)
        enable();
}

void Repo::reset()
{
    m_status = RepoStatus::Unknown;
    m_activity = RepoActivity::Idle;
    m_statistics = RepoStatistics{};
    m_errors.clear();
}

void Repo::enable()
{
    if (m_enabled)
        return;
    m_enabled = true;

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
}

void Repo::disable()
{
    if (!m_enabled)
        return;
    m_enabled = false;

    m_recheck_timer->stop();
    m_check_future.cancel();
    reset();
}

void Repo::startCheck()
{
    if (m_activity == RepoActivity::Checking)
        return;
    m_activity = RepoActivity::Checking;

    m_check_future = QtConcurrent::run([this]() -> check_result_t {
        return check();
    });

    m_check_watcher.setFuture(m_check_future);
}

Repo::check_result_t Repo::check()
{
    RepoStatistics stats;
    QList<QString> errors;
    stats.timestamp = QDateTime::currentDateTime();

    std::optional<git::repository> repo_opt;
    try {
        git::repository repo = git::repository::open(m_settings.path.toStdString().c_str());
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

    return {stats, errors};
}

void Repo::checkCompleted()
{
    if (m_check_watcher.isCanceled())
        return;  // the canceller should reset the activity to Idle

    auto [stats, errors] = m_check_watcher.result();

    m_statistics = stats;

    if (!errors.isEmpty()) {
        m_status = RepoStatus::Error;

        qDebug() << "Errors while checking repository " << m_settings.path << ":";

        for (auto const& error : errors) {
            qDebug() << "Error:" << error;
            m_errors.push_back({m_statistics.timestamp, error});
        }

        // TODO: deduplicate error messages and drop old ones eventually
    }
    else {
        m_status = stats.isOk() ? RepoStatus::Ok : RepoStatus::DirtyOrOutdated;
    }

    m_activity = RepoActivity::Idle;

    // TODO: emit signal to notify about the new status
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
