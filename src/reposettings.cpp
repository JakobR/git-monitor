#include "reposettings.h"
#include "git/repository.h"
#include <QCoreApplication>
#include <QDir>

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

QList<QString> RepoSettings::validate() const
{
    QList<QString> errors;

    if (!QDir(path).exists()) {
        errors.push_back(tr("Directory does not exist: %1").arg(path));
        return errors;
    }

    std::string path_terminated = path.toStdString();
    try {
        auto repo = git::repository::open(path_terminated.c_str());
    }
    catch (std::exception const& e) {
        errors.push_back(tr("Unable to open git repository: %1").arg(e.what()));
        return errors;
    }

    return errors;
}

QString RepoSettings::tr(char const* sourceText)
{
    return QCoreApplication::translate("RepoSettings", sourceText);
}
