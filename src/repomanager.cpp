#include "repomanager.h"
#include "settings.h"
#include <utility>

RepoManager::RepoManager(QObject *parent)
    : QObject{parent}
{}

void RepoManager::readSettings()
{
    QSettings settings;
    // TODO
}

void RepoManager::writeSettings()
{
    QSettings settings;

    QList<QVariantMap> repoSettings;
    for (Repo const& repo : std::as_const(m_repos))
        repoSettings.push_back(repo.settings().toVariantMap());

    settings.setValue(Settings::RepoManager::Repos, QVariant::fromValue(repoSettings));
}
