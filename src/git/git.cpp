#include "git.h"
#include "util.h"
#include <git2.h>

using namespace git;

void git::libgit2_init()
{
    int error = git_libgit2_init();
    throw_on_git2_error(error);
}

void git::libgit2_shutdown()
{
    int error = git_libgit2_shutdown();
    throw_on_git2_error(error);
}

version_t git::libgit2_version()
{
    version_t ver;
    int error = git_libgit2_version(&ver.major, &ver.minor, &ver.rev);
    throw_on_git2_error(error);
    return ver;
}

std::ostream& version_t::display(std::ostream& out) const
{
    return out << major << "." << minor << "." << rev;
}
