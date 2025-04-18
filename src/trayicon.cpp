#include "trayicon.h"
#include <QApplication>
#include <QMenu>

TrayIcon::TrayIcon(QObject* parent)
    : QObject{parent}
{
    m_systemTrayIcon = new QSystemTrayIcon(this);
    connect(m_systemTrayIcon, &QSystemTrayIcon::activated, this, &TrayIcon::on_systemTrayIcon_activated);

    QIcon icon(":/images/Git-Icon-Black.png");
    m_systemTrayIcon->setIcon(icon);

    m_systemTrayIcon->setToolTip(tr("Git Monitor: all repositories are ok"));

    QAction* titleAction = new QAction(tr("Git Monitor"), this);
    titleAction->setEnabled(false);

    QAction* settingsAction = new QAction(tr("&Settings"), this);
    connect(settingsAction, &QAction::triggered, this, &TrayIcon::showSettings);

    QAction* quitAction = new QAction(tr("&Quit Git Monitor"), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    QMenu* trayIconMenu = new QMenu();
    trayIconMenu->addAction(titleAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(settingsAction);
    trayIconMenu->addAction(quitAction);
    m_systemTrayIcon->setContextMenu(trayIconMenu);
}

void TrayIcon::setRepoManager(RepoManager* newRepoManager)
{
    if (m_repoManager)
        disconnect(m_repoManager, &RepoManager::repoChanged, this, &TrayIcon::on_repo_changed);

    m_repoManager = newRepoManager;

    if (m_repoManager)
        connect(m_repoManager, &RepoManager::repoChanged, this, &TrayIcon::on_repo_changed);
}

void TrayIcon::hide()
{
    m_systemTrayIcon->hide();
}

void TrayIcon::show()
{
    m_systemTrayIcon->show();
}

void TrayIcon::on_systemTrayIcon_activated(QSystemTrayIcon::ActivationReason reason)
{
    qDebug() << "Tray icon activated with reason:" << reason;
}

void TrayIcon::on_repo_changed(Repo* repo)
{
    // TODO
}
