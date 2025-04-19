#ifndef REPOTABLEMODEL_H
#define REPOTABLEMODEL_H

#include "repomanager.h"
#include <QAbstractTableModel>

class RepoTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit RepoTableModel(QObject* parent = nullptr);

    // RepoManager* repoManager() const { return m_repoManager; }
    void setRepoManager(RepoManager* repoManager);

    void addRepo(RepoSettings settings);

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int rowCount(QModelIndex const& parent = QModelIndex()) const override;
    int columnCount(QModelIndex const& parent = QModelIndex()) const override;

    QVariant data(QModelIndex const& index, int role = Qt::DisplayRole) const override;

    struct Column {
        enum ColumnIndices : int {
            Path = 0,
            Status,
            Uncommitted,
            HEAD,
            Branches,
            Remote,
            COUNT,
        };
        inline static constexpr int MIN = 0;
        inline static constexpr int MAX = COUNT - 1;
    };

private:
    QVariant getPathData(Repo const* repo) const;
    QVariant getStatusData(Repo const* repo, int role) const;
    QVariant getUncommittedData(Repo const* repo) const;
    QVariant getHEADData(Repo const* repo) const;
    QVariant getBranchesData(Repo const* repo) const;
    QVariant getRemoteData(Repo const* repo) const;

private slots:
    void on_repo_changed(Repo* repo);

private:
    RepoManager* m_repoManager = nullptr;
};

#endif // REPOTABLEMODEL_H
