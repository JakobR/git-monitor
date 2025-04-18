#ifndef REPO_H
#define REPO_H

#include "reposettings.h"
#include "git/repository.h"
#include <QDateTime>
#include <QFileSystemWatcher>
#include <QFuture>
#include <QFutureWatcher>
#include <QList>
#include <QObject>
#include <QString>
#include <QTimer>
#include <chrono>
#include <optional>
#include <utility>

enum class RepoStatus {
    /// new and not yet checked, or checking disabled for this repo
    Unknown,
    /// everything committed and synchronized
    Ok,
    /// there some uncommitted or unpushed changes, or
    /// there are some unmerged or unfetched changes
    DirtyOrOutdated,
    /// unable to access the repository, not a git repository, some other error...
    Error,
};

enum class RepoActivity {
    /// doing nothing, waiting for timeout before re-checking
    Idle,
    /// waiting for filesystem updates (via inotify) to quiesce for 2 seconds before re-checking
    Waiting,
    /// repo status is being checked
    Checking,
};

struct RepoStatistics {
    /// when the check was started
    QDateTime timestamp;
    /// number of uncommitted changes
    size_t uncommitted = 0;
    /// number of unpushed and unmerged commits on HEAD branch
    /// is empty if HEAD is detached or does not have a remote-tracking branch
    std::optional<git::ahead_behind_t> head_ahead_behind;
    /// number of unpushed and unmerged commits across all branches
    git::ahead_behind_t total_ahead_behind;
    /// whether HEAD's remote-tracking branch differs from the commit advertised by the remote repository
    git::branch_state head_state = git::branch_state::unknown;
    /// number of remote-tracking branches that differ from their remote repository
    size_t branches_outdated = 0;

    bool isOk() const;
};

struct RepoCheckError {
    QDateTime timestamp;
    QString message;
};

class Repo : public QObject
{
    Q_OBJECT
public:
    explicit Repo(size_t index, QObject* parent = nullptr);

    RepoSettings const& settings() const;
    void updateSettings(RepoSettings new_settings);

    bool isEnabled() const { return m_enabled; }

    /// enable automatic checking (triggered by timeout and filesystem changes)
    void enable();

    /// disable automatic checking
    void disable();

    size_t index() const { return m_index; }

    RepoStatus status() const { return m_status; }
    RepoActivity activity() const { return m_activity; }
    RepoStatistics const& statistics() const { return m_statistics; }
    QList<RepoCheckError> const& errors() const { return m_errors; }

private:
    void reset();
    void startCheck();

    /// check was successful if the second element of the pair is empty
    using check_result_t = std::pair<RepoStatistics, QList<QString>>;
    /// NOTE: do not call this directly, use startCheck() instead to perform the check in a background thread
    check_result_t check();

    void dropOldErrors(QDateTime const& now);
    void deduplicateErrors();

    void setActivity(RepoActivity activity);

private slots:
    void checkCompleted();

signals:
    // void activityChanged();
    void changed();

private:
    size_t m_index;  //< index in the RepoManager
    RepoSettings m_settings;

    RepoStatus m_status = RepoStatus::Unknown;
    RepoActivity m_activity = RepoActivity::Idle;
    RepoStatistics m_statistics;

    bool m_enabled = false;

    std::chrono::milliseconds m_recheck_interval = std::chrono::minutes(5);
    QTimer* m_recheck_timer = nullptr;

    QFileSystemWatcher* m_watcher = nullptr;

    QList<RepoCheckError> m_errors;

    QFuture<check_result_t> m_check_future;
    QFutureWatcher<check_result_t> m_check_watcher;
};

#endif // REPO_H
