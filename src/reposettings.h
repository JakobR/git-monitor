#ifndef REPOSETTINGS_H
#define REPOSETTINGS_H

#include <QVariantMap>

struct RepoSettings
{
    QString path;
    bool warnOnUncommittedChanges = true;
    bool warnOnUnpushedCommits = true;
    bool warnOnUnmergedCommits = true;
    bool warnOnUnfetchedCommits = true;

    QVariantMap toVariantMap() const;
    static RepoSettings fromVariantMap(QVariantMap const& map);
};

#endif // REPOSETTINGS_H
