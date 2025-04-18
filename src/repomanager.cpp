#include "repomanager.h"
#include "settings.h"

RepoManager::RepoManager(QObject* parent)
    : QObject{parent}
{
    // Make sure that QList<QVariantMap> is registered.
    // Otherwise, readSettings will fail due to "unknown user type with name QList<QVariantMap>".
    QVariant::fromValue<QList<QVariantMap>>({});
}

void RepoManager::readSettings()
{
    if (!m_repos.isEmpty()) {
        qDebug() << "RepoManager::readSettings: refusing to read settings multiple times";
        return;  // already read; we do not want to set up the Repo objects multiple times
    }

    QSettings settings;

    auto const allRepoSettingsVariant = settings.value(Settings::RepoManager::Repos);
    qDebug() << "got value" << allRepoSettingsVariant;
    if (allRepoSettingsVariant.isValid() && !allRepoSettingsVariant.canConvert<QList<QVariantMap>>()) {
        qDebug() << "RepoManager::readSettings: unable to deserialize repo settings";
        return;
    }

    auto const allRepoSettings = allRepoSettingsVariant.value<QList<QVariantMap>>();
    for (auto const& repoSettingsMap : allRepoSettings)
        addRepo(RepoSettings::fromVariantMap(repoSettingsMap));

    qDebug() << "RepoManager::readSettings: loaded" << m_repos.size() << "repositories";
}

void RepoManager::writeSettings()
{
    QSettings settings;

    QList<QVariantMap> allRepoSettings;
    for (Repo const* repo : m_repos) {
        qDebug() << "RepoManager::writeSettings: saving repository" << repo->settings().path;
        allRepoSettings.push_back(repo->settings().toVariantMap());
    }

    settings.setValue(Settings::RepoManager::Repos, QVariant::fromValue<QList<QVariantMap>>(allRepoSettings));
    qDebug() << "RepoManager::writeSettings: saved" << allRepoSettings.size() << "repositories";
}

Repo const* RepoManager::addRepo(RepoSettings settings)
{
    qDebug() << "Adding repository:" << settings.path;
    Repo* repo = new Repo(m_repos.size(), this);
    m_repos.push_back(repo);
    Q_ASSERT(m_repos.at(repo->index()) == repo);
    repo->updateSettings(std::move(settings));
    repo->enable();

    connect(repo, &Repo::changed, this, [this, repo]() {
        emit repoChanged(repo);
    });

    return repo;
}

Repo const* RepoManager::addNewRepo(RepoSettings settings)
{
    Repo const* repo = addRepo(std::move(settings));
    writeSettings();
    return repo;
}
