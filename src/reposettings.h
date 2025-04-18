#ifndef REPOSETTINGS_H
#define REPOSETTINGS_H

#include <QList>
#include <QString>
#include <QVariantMap>

struct RepoSettings
{
    QString path;
    bool warnOnUncommittedChanges = true;
    bool warnOnUnpushedCommits = true;
    bool warnOnUnmergedCommits = true;
    bool warnOnUnfetchedCommits = true;

    // TODO: maybe we want to add a setting to select a subset of branches to monitor?

    QVariantMap toVariantMap() const;
    [[nodiscard]] static RepoSettings fromVariantMap(QVariantMap const& map);

    /// Validate settings.
    /// @returns a list of error messages. Successful validation is indicated by an empty list.
    QList<QString> validate() const;

private:
    static QString tr(char const* sourceText);
};

#endif // REPOSETTINGS_H
