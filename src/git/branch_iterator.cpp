#include "branch_iterator.h"
#include "util.h"
#include <fmt/format.h>
#include <git2.h>
#include <stdexcept>

using namespace git;

branch_iterator::branch_iterator(git_branch_iterator* iter)
    : m_iter(iter)
{
    if (!m_iter)
        throw std::invalid_argument("git_branch_iterator* is null");
}

void branch_iterator::git_branch_iterator_deleter::operator()(git_branch_iterator* ptr) const
{
    git_branch_iterator_free(ptr);
}

branch_iterator::~branch_iterator() noexcept
{ }

std::optional<std::pair<reference, branch_type>> branch_iterator::next()
{
    git_reference* ref_raw;
    git_branch_t type_raw;
    int error = git_branch_next(&ref_raw, &type_raw, m_iter.get());
    if (error == GIT_ITEROVER)
        return std::nullopt;
    throw_on_git2_error(error);

    reference ref{ref_raw};
    branch_type type;
    switch (type_raw) {
        case GIT_BRANCH_LOCAL:
            type = branch_type::local;
            break;
        case GIT_BRANCH_REMOTE:
            type = branch_type::remote;
            break;
        default:
            std::string message = fmt::format("unknown branch type: {}", fmt::underlying(type_raw));
            throw_with_message(message);
    }

    return std::optional{std::pair{std::move(ref), type}};
}
