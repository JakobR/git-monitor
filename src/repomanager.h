#ifndef REPOMANAGER_H
#define REPOMANAGER_H

#include "repo.h"
#include <QObject>
#include <QList>

class RepoManager : public QObject
{
    Q_OBJECT
public:
    explicit RepoManager(QObject *parent = nullptr);

    void readSettings();
    void writeSettings();

signals:

private:
    QList<Repo> m_repos;
};

#endif // REPOMANAGER_H
