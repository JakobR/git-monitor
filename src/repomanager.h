#ifndef REPOMANAGER_H
#define REPOMANAGER_H

#include "repo.h"
#include <QObject>
#include <QList>

class RepoManager : public QObject
{
    Q_OBJECT

    Repo const* addRepo(RepoSettings settings);

public:
    explicit RepoManager(QObject* parent = nullptr);

    void readSettings();
    void writeSettings();

    Repo const* addNewRepo(RepoSettings settings);

    QList<Repo*> const& repos() const { return m_repos; }

signals:
    void repoChanged(Repo* repo);

private:
    QList<Repo*> m_repos;
};

#endif // REPOMANAGER_H
