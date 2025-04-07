#include "oid.h"
#include "util.h"
#include <fmt/format.h>
#include <git2.h>

using namespace git;

oid::oid()
    : m_oid{0}
{ }

oid::oid(git_oid const& the_oid)
    : m_oid{the_oid}
{ }

bool oid::is_zero() const
{
    return git_oid_is_zero(get());
}

bool oid::operator==(oid const& other) const
{
    return git_oid_equal(get(), other.get());
}

int oid::compare(oid const& other) const
{
    return git_oid_cmp(get(), other.get());
}

auto fmt::formatter<oid>::format(oid o, format_context& ctx) const -> format_context::iterator
{
    char oid_hex[GIT_OID_MAX_HEXSIZE + 1] = {0};
    int error = git_oid_fmt(oid_hex, o.get());
    throw_on_git2_error(error);
    return formatter<std::string>::format(oid_hex, ctx);
}
