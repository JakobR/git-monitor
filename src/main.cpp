#include <QApplication>
#include <git2.h>
#include <iostream>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/std.h>

#include "mainwindow.h"
#include "git/repository.h"


int main(int argc, char* argv[])
{
    int error = 0;
    qDebug() << "Starting";

    git_libgit2_init();

    int major, minor, rev;
    error = git_libgit2_version(&major, &minor, &rev);
    if (error < 0) {
        git_error const* e = git_error_last();
        printf("Error %d/%d: %s\n", error, e->klass, e->message);
        return error;
    }
    fmt::println("libgit2 version %d.%d.%d", major, minor, rev);

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
        // fmt::println("unfetched commits: head={} total={}", unfetched.head, unfetched.total);
        auto main = repo.lookup_local_branch("main");
        if (main) {
            auto main_upstream = main->branch_upstream();
            fmt::println("main: {}", main->resolve().target());
            if (main_upstream)
                fmt::println("main: -> {}", main_upstream->name());
        }
    }

    // return 0;

    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    int result = app.exec();
    qDebug() << "Exiting:" << result;

    // this is optional if the application is exiting anyway
    // git_libgit2_shutdown();

    return result;
}
