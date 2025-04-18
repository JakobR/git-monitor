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
        }
    }

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
        auto const& repo = m_repoManager->repos().at(index.row());

        switch (index.column()) {
        case Column::Path:
            return repo->settings().path;
        case Column::Status:
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
        }
    }

    return QVariant();
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
