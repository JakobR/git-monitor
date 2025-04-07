#include "util.h"
#include <stdexcept>
#include <fmt/format.h>
#include <git2.h>

void git::throw_on_git2_error(int error)
{
    if (error >= 0)
        return;
    git_error const* e = git_error_last();
    std::string message = fmt::format("{}/{}: {}", error, e->klass, e->message);
    throw_with_message(message);
}

void git::throw_with_message(std::string const& inner_message)
{
    std::string message = fmt::format("libgit2 error: {}", inner_message);
    throw std::runtime_error(message);
}
