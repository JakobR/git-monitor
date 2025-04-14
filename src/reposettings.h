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

    QVariantMap toVariantMap() const;
    static RepoSettings fromVariantMap(QVariantMap const& map);

    /// Validate settings.
    /// @returns a list of error messages. Successful validation is indicated by an empty list.
    QList<QString> validate() const;

private:
    static QString tr(char const* sourceText);
};

#endif // REPOSETTINGS_H
