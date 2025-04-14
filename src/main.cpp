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
    int error = 0;
    qDebug() << "Starting";

    git::libgit2_init();

    fmt::println("libgit2 version {}", fmt::streamed(git::libgit2_version()));
    fmt::println("Qt version {}", qVersion());

    char const* repo_path = "/home/jakob/testrepo";

    {
        auto repo = git::repository::open(repo_path);
        auto uncommitted = repo.uncommitted_changes();
        auto unpushed_head = repo.head_ahead_behind();
        auto unpushed_all = repo.total_ahead_behind();
        auto remote_state = repo.check_remote_state();
        fmt::println("");
        fmt::println("uncommitted changes: {}" , uncommitted);
        fmt::println("HEAD: {} ahead, {} behind, remote state: {}", unpushed_head->ahead, unpushed_head->behind, remote_state.head_state);
        fmt::println("Total over all branches: {} ahead, {} behind, remote state: {} up to date, {} outdated", unpushed_all.ahead, unpushed_all.behind, remote_state.branches_up_to_date, remote_state.branches_outdated);
        if (!remote_state.errors.empty())
            fmt::println("Remote Errors:\n    {}", fmt::join(remote_state.errors, "\n    "));
        auto main = repo.lookup_local_branch("main");
        if (main) {
            auto main_upstream = main->branch_upstream();
            fmt::println("main: {}", main->resolve().target());
            if (main_upstream)
                fmt::println("main: -> {}", main_upstream->name());
        }
    }

    // return 0;

    QCoreApplication::setOrganizationName("Jakob Rath");
    QCoreApplication::setOrganizationDomain("jakobrath.eu");
    QCoreApplication::setApplicationName("Git Monitor");

    QApplication app(argc, argv);

    RepoManager repoManager;

    MainWindow w;
    w.show();
    int result = app.exec();
    qDebug() << "Exiting:" << result;

    // this is optional if the application is exiting anyway
    git::libgit2_shutdown();

    return result;
}
