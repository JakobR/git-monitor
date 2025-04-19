#include "repotablemodel.h"
#include <QSize>

RepoTableModel::RepoTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{}

void RepoTableModel::setRepoManager(RepoManager* newRepoManager)
{
    if (m_repoManager)
        disconnect(m_repoManager, &RepoManager::repoChanged, this, &RepoTableModel::on_repo_changed);

    beginResetModel();
    m_repoManager = newRepoManager;
    endResetModel();

    if (m_repoManager)
        connect(m_repoManager, &RepoManager::repoChanged, this, &RepoTableModel::on_repo_changed);
}

void RepoTableModel::addRepo(RepoSettings settings)
{
    Q_ASSERT(m_repoManager);
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_repoManager->addNewRepo(std::move(settings));
    endInsertRows();
}

QVariant RepoTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal)
        return QVariant();

    if (role == Qt::DisplayRole) {
        switch (section) {
        case Column::Path:
            return tr("Repository Path");
        case Column::Status:
            return tr("Status");
        case Column::Uncommitted:
            return tr("Uncommitted");
        case Column::HEAD:
            return tr("HEAD");
        case Column::Branches:
            return tr("All Branches");
        case Column::Remote:
            return tr("Remote Branches");
        }
    }

    // TODO: implement Qt::ToolTipRole with explanations

    return QVariant();
}

int RepoTableModel::columnCount(QModelIndex const& parent) const
{
    if (parent.isValid())
        return 0;

    return Column::COUNT;
}

int RepoTableModel::rowCount(QModelIndex const& parent) const
{
    if (!m_repoManager)
        return 0;
    if (parent.isValid())
        return 0;
    return m_repoManager->repos().size();
}

QVariant RepoTableModel::data(QModelIndex const& index, int role) const
{
    Q_ASSERT(m_repoManager);
    Q_ASSERT(index.isValid());
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole) {
        Repo const* repo = m_repoManager->repos().at(index.row());

        switch (index.column()) {
        case Column::Path:
            return getPathData(repo);
        case Column::Status:
            return getStatusData(repo, role);
        case Column::Uncommitted:
            return getUncommittedData(repo);
        case Column::HEAD:
            return getHEADData(repo);
        case Column::Branches:
            return getBranchesData(repo);
        case Column::Remote:
            return getRemoteData(repo);
        }
    }

    // TODO: implement Qt::ToolTipRole with more detailed messages

    return QVariant();
}

QVariant RepoTableModel::getPathData(Repo const* repo) const
{
    return repo->settings().path;
}

QVariant RepoTableModel::getStatusData(Repo const* repo, int role) const
{
    // TODO: red bold font on error
    if (role != Qt::DisplayRole)
        return QVariant();
    if (repo->activity() == RepoActivity::Checking)
        return tr("Checking...");
    switch (repo->status()) {
        case RepoStatus::Ok:
            return tr("OK");
        case RepoStatus::DirtyOrOutdated:
            return tr("Dirty");
        case RepoStatus::Unknown:
            return tr("???");
        case RepoStatus::Error:
            return tr("Error");
    }
    return QVariant();
}

QVariant RepoTableModel::getUncommittedData(Repo const* repo) const
{
    auto const& uncommitted = repo->statistics().uncommitted;
    if (!uncommitted)
        return QVariant();
    return tr("%1 changes").arg(*uncommitted);
}

QVariant RepoTableModel::getHEADData(Repo const* repo) const
{
    auto const& head_ab = repo->statistics().head_ahead_behind;
    auto const& head_state = repo->statistics().head_state;
    // qDebug() << "repo " << repo->settings().path << "HEAD state:" << (int)head_state << " head_ab:" << head_ab.has_value();
    // if (!head_ab && head_state == git::branch_state::unknown)
    //     return QVariant();
    // if (!head_ab && head_state == git::branch_state::connection_error)
    //     return QVariant();
    QString result;
    if (head_ab && head_ab->ahead > 0)
        result += tr("%1 ahead").arg(head_ab->ahead);
    if (head_ab && head_ab->behind > 0) {
        if (!result.isEmpty())
            result += ", ";
        result += tr("%1 behind").arg(head_ab->behind);
    }
    if (head_state == git::branch_state::outdated) {
        if (!result.isEmpty())
            result += ", ";
        result += tr("outdated");
    }
    if (result.isEmpty())
        return tr("OK");
    return result;
}

QVariant RepoTableModel::getBranchesData(Repo const* repo) const
{
    auto const& total_ab = repo->statistics().total_ahead_behind;
    if (!total_ab)
        return QVariant();
    QString result;
    if (total_ab->ahead > 0)
        result += tr("%1 ahead").arg(total_ab->ahead);
    if (total_ab->behind > 0) {
        if (!result.isEmpty())
            result += ", ";
        result += tr("%1 behind").arg(total_ab->behind);
    }
    if (result.isEmpty())
        return tr("OK");
    return result;
}

QVariant RepoTableModel::getRemoteData(Repo const* repo) const
{
    auto const& branches_outdated = repo->statistics().branches_outdated;
    if (!branches_outdated)
        return QVariant();
    if (*branches_outdated == 0)
        return tr("Up-to-date");
    return tr("%1 outdated").arg(*branches_outdated);
}

void RepoTableModel::on_repo_changed(Repo* repo)
{
    if (sender() != m_repoManager)
        return;
    auto const row = repo->index();
    auto const topLeft = index(row, Column::MIN);
    auto const bottomRight = index(row, Column::MAX);
    emit dataChanged(topLeft, bottomRight);
}
