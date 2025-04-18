#include "mainwindow.h"
#include "repomanager.h"
#include "trayicon.h"
#include "git/git.h"

#include <QApplication>
#include <QCommandLineParser>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <fmt/std.h>


int main(int argc, char* argv[])
{
    fmt::println("Git Monitor version {}", GIT_MONITOR_VERSION);
    fmt::println("libgit2: loaded version {}, compiled against version {}", fmt::streamed(git::libgit2_runtime_version()), fmt::streamed(git::libgit2_compile_version()));
    fmt::println("Qt: loaded version {}, compiled against version {}", qVersion(), QT_VERSION_STR);

    git::libgit2_init();

    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("Jakob Rath");
    QCoreApplication::setOrganizationDomain("jakobrath.eu");
#ifdef QT_DEBUG
    QCoreApplication::setApplicationName("Git Monitor (Debug Mode)");
#else
    QCoreApplication::setApplicationName("Git Monitor");
#endif
    QCoreApplication::setApplicationVersion(GIT_MONITOR_VERSION);

    QApplication::setQuitOnLastWindowClosed(false);


    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::applicationName());
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption hide("hide", QCoreApplication::translate("main", "Hide the settings window on startup"));
    parser.addOption(hide);

    parser.process(app);

    RepoManager repoManager;
    repoManager.readSettings();

    TrayIcon trayIcon;
    trayIcon.setRepoManager(&repoManager);
    trayIcon.show();

    MainWindow w;
    w.setRepoManager(&repoManager);
    QObject::connect(&trayIcon, &TrayIcon::showSettings, &w, &MainWindow::show);

    if (!parser.isSet(hide))
        w.show();

    int result = app.exec();
    qDebug() << "Exiting:" << result;

    // this is optional if the application is exiting anyway
    // git::libgit2_shutdown();

    return result;
}
