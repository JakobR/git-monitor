#include "mainwindow.h"
#include "repomanager.h"
#include "git/git.h"

#include <QApplication>
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
    QCoreApplication::setApplicationName("Git Monitor");
    QCoreApplication::setApplicationVersion(GIT_MONITOR_VERSION);


    RepoManager repoManager;

    MainWindow w;
    w.show();
    int result = app.exec();
    qDebug() << "Exiting:" << result;

    // this is optional if the application is exiting anyway
    git::libgit2_shutdown();

    return result;
}
