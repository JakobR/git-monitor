#include "reposettings.h"
#include "git/repository.h"
#include <QCoreApplication>
#include <QDir>

namespace {
    inline constexpr char const* k_path = "path";
    inline constexpr char const* k_warnOnUncommittedChanges = "warnOnUncommittedChanges";
    inline constexpr char const* k_warnOnUnpushedCommits    = "warnOnUnpushedCommits";
    inline constexpr char const* k_warnOnUnmergedCommits    = "warnOnUnmergedCommits";
    inline constexpr char const* k_warnOnUnfetchedCommits   = "warnOnUnfetchedCommits";
}

QVariantMap RepoSettings::toVariantMap() const
{
    QVariantMap map;
    map[k_path] = path;
    map[k_warnOnUncommittedChanges] = warnOnUncommittedChanges;
    map[k_warnOnUnpushedCommits   ] = warnOnUnpushedCommits;
    map[k_warnOnUnmergedCommits   ] = warnOnUnmergedCommits;
    map[k_warnOnUnfetchedCommits  ] = warnOnUnfetchedCommits;
    return map;
}

RepoSettings RepoSettings::fromVariantMap(QVariantMap const& map)
{
    RepoSettings rs;
    rs.path = map[k_path].toString();
    rs.warnOnUncommittedChanges = map[k_warnOnUncommittedChanges].toBool();
    rs.warnOnUnpushedCommits    = map[k_warnOnUnpushedCommits   ].toBool();
    rs.warnOnUnmergedCommits    = map[k_warnOnUnmergedCommits   ].toBool();
    rs.warnOnUnfetchedCommits   = map[k_warnOnUnfetchedCommits  ].toBool();
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
