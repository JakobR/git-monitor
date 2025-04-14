#include "reposettings.h"

QVariantMap RepoSettings::toVariantMap() const
{
    QVariantMap map;
    map["path"] = path;
    map["warnOnUncommittedChanges"] = warnOnUncommittedChanges;
    map["warnOnUnpushedCommits"   ] = warnOnUnpushedCommits;
    map["warnOnUnmergedCommits"   ] = warnOnUnmergedCommits;
    map["warnOnUnfetchedCommits"  ] = warnOnUnfetchedCommits;
    return map;
}

RepoSettings RepoSettings::fromVariantMap(QVariantMap const& map)
{
    RepoSettings rs;
    rs.path = map["path"].toString();
    rs.warnOnUncommittedChanges = map["warnOnUncommittedChanges"].toBool();
    rs.warnOnUnpushedCommits    = map["warnOnUnpushedCommits"   ].toBool();
    rs.warnOnUnmergedCommits    = map["warnOnUnmergedCommits"   ].toBool();
    rs.warnOnUnfetchedCommits   = map["warnOnUnfetchedCommits"  ].toBool();
    return rs;
}
