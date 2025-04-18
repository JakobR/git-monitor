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

version_t git::libgit2_compile_version()
{
    version_t ver;
    ver.major = LIBGIT2_VER_MAJOR;
    ver.minor = LIBGIT2_VER_MINOR;
    ver.rev = LIBGIT2_VER_REVISION;
    return ver;
}

version_t git::libgit2_runtime_version()
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
