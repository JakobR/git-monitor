#include <QApplication>
#include <git2.h>

#include "mainwindow.h"


int status_cb(char const* path, unsigned int status_flags, void* payload)
{
    printf("%d\t%s", status_flags, path);
    if ((status_flags & GIT_STATUS_INDEX_NEW) != 0) printf(" INDEX_NEW");
    if ((status_flags & GIT_STATUS_INDEX_MODIFIED) != 0) printf(" INDEX_MODIFIED");
    if ((status_flags & GIT_STATUS_INDEX_DELETED) != 0) printf(" INDEX_DELETED");
    if ((status_flags & GIT_STATUS_INDEX_RENAMED) != 0) printf(" INDEX_RENAMED");
    if ((status_flags & GIT_STATUS_INDEX_TYPECHANGE) != 0) printf(" INDEX_TYPECHANGE");
    if ((status_flags & GIT_STATUS_WT_NEW) != 0) printf(" WT_NEW");
    if ((status_flags & GIT_STATUS_WT_MODIFIED) != 0) printf(" WT_MODIFIED");
    if ((status_flags & GIT_STATUS_WT_DELETED) != 0) printf(" WT_DELETED");
    if ((status_flags & GIT_STATUS_WT_TYPECHANGE) != 0) printf(" WT_TYPECHANGE");
    if ((status_flags & GIT_STATUS_WT_RENAMED) != 0) printf(" WT_RENAMED");
    if ((status_flags & GIT_STATUS_WT_UNREADABLE) != 0) printf(" WT_UNREADABLE");
    if ((status_flags & GIT_STATUS_IGNORED) != 0) printf(" IGNORED");
    if ((status_flags & GIT_STATUS_CONFLICTED) != 0) printf(" CONFLICTED");
    printf("\n");
    return 0;
}


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
    printf("libgit2 version %d.%d.%d\n", major, minor, rev);

    git_repository* repo = nullptr;

    error = git_repository_open(&repo, "/home/jakob/projects/git-monitor");
    if (error < 0) {
        git_error const* e = git_error_last();
        printf("Error %d/%d: %s\n", error, e->klass, e->message);
        return error;
    }

    printf("\nStatus (simple):\n");
    int my_data = 0;
    error = git_status_foreach(repo, status_cb, &my_data);

    printf("\nStatus (opts):\n");
    git_status_options opts = GIT_STATUS_OPTIONS_INIT;
    opts.flags =
        GIT_STATUS_OPT_INCLUDE_UNTRACKED |
        GIT_STATUS_OPT_INCLUDE_UNREADABLE |
        GIT_STATUS_OPT_NO_REFRESH;  // unclear what this does
    error = git_status_foreach_ext(repo, &opts, status_cb, &my_data);


    git_strarray remotes = {0};
    error = git_remote_list(&remotes, repo);
    if (error < 0) {
        git_error const* e = git_error_last();
        printf("Error %d/%d: %s\n", error, e->klass, e->message);
        return error;
    }

    printf("\nRemotes:\n");
    for (size_t i = 0; i < remotes.count; ++i) {
        char const* name = remotes.strings[i];
        printf("    %s\n", name);

        git_remote* remote = nullptr;
        error = git_remote_lookup(&remote, repo, name);
        if (error < 0) {
            git_error const* e = git_error_last();
            printf("Error %d/%d: %s\n", error, e->klass, e->message);
            return error;
        }

        size_t num_refspecs = git_remote_refspec_count(remote);
        for (size_t i = 0; i < num_refspecs; ++i) {
            git_refspec const* refspec = git_remote_get_refspec(remote, i);
            char const* str = git_refspec_string(refspec);
            char const* src = git_refspec_src(refspec);
            char const* dst = git_refspec_dst(refspec);
            printf("        refspec: %s\n", str);
            printf("            src: %s\n", src);
            printf("            dst: %s\n", dst);
            printf("            force: %d\n", git_refspec_force(refspec));
            printf("            direction: %d\n", git_refspec_direction(refspec));
        }

        git_strarray fetch_refspecs = {0};
        error = git_remote_get_fetch_refspecs(&fetch_refspecs, remote);
        if (error < 0) {
            git_error const* e = git_error_last();
            printf("Error %d/%d: %s\n", error, e->klass, e->message);
            return error;
        }
        for (size_t i = 0; i < fetch_refspecs.count; ++i) {
            char const* refspec = fetch_refspecs.strings[i];
            printf("        fetch: %s\n", refspec);
        }
        git_strarray_dispose(&fetch_refspecs);

        git_remote_free(remote);
    }

    git_strarray_dispose(&remotes);

    git_repository_free(repo);
    repo = nullptr;

    // no need to all this if application is exiting
    git_libgit2_shutdown();

    return 0;

    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    int result = app.exec();
    qDebug() << "Exiting:" << result;
    return result;
}
