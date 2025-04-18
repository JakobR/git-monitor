#ifndef TRAYICON_H
#define TRAYICON_H

#include "repomanager.h"
#include <QObject>
#include <QSystemTrayIcon>

class TrayIcon : public QObject
{
    Q_OBJECT
public:
    explicit TrayIcon(QObject* parent = nullptr);

    void setRepoManager(RepoManager* repoManager);

public slots:
    void hide();
    void show();

private slots:
    void on_systemTrayIcon_activated(QSystemTrayIcon::ActivationReason reason);
    void on_repo_changed(Repo* repo);

signals:
    void showSettings();

private:
    RepoManager* m_repoManager = nullptr;
    QSystemTrayIcon* m_systemTrayIcon = nullptr;
};

#endif // TRAYICON_H
