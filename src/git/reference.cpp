#include "reference.h"
#include "util.h"
#include <fmt/format.h>
#include <git2.h>

using namespace git;

reference::reference(git_reference* ref)
    : m_ref{ref}
{
    if (!m_ref)
        throw std::invalid_argument("git_reference* is null");
}

void reference::git_reference_deleter::operator()(git_reference* ptr) const
{
    git_reference_free(ptr);
}

reference::~reference() noexcept
{ }

reference_type reference::type() const
{
    git_reference_t tp = git_reference_type(ref());
    switch (tp) {
    case GIT_REFERENCE_INVALID:
        return reference_type::invalid;
    case GIT_REFERENCE_DIRECT:
        return reference_type::direct;
    case GIT_REFERENCE_SYMBOLIC:
        return reference_type::symbolic;
    default:
        std::string message = fmt::format("unknown reference type: {}", fmt::underlying(tp));
        throw_with_message(message);
    }
}

char const* reference::name() const
{
    return git_reference_name(ref());
}

char const* reference::shorthand() const
{
    return git_reference_shorthand(ref());
}

reference reference::resolve() const
{
    git_reference* resolved;
    int error = git_reference_resolve(&resolved, ref());
    throw_on_git2_error(error);
    return reference{resolved};
}

std::optional<oid> reference::target() const
{
    git_oid const* oid_ptr = git_reference_target(ref());
    return oid_ptr ? std::optional{oid{*oid_ptr}} : std::nullopt;
}

char const* reference::symbolic_target() const
{
    return git_reference_symbolic_target(ref());
}

bool reference::is_branch() const
{
    return git_reference_is_branch(ref());
}

bool reference::is_remote() const
{
    return git_reference_is_remote(ref());
}

bool reference::is_tag() const
{
    return git_reference_is_tag(ref());
}

bool reference::is_note() const
{
    return git_reference_is_note(ref());
}

std::optional<reference> reference::branch_upstream() const
{
    git_reference* upstream_raw = nullptr;
    int error = git_branch_upstream(&upstream_raw, ref());
    if (error == GIT_ENOTFOUND)
        return std::nullopt;
    throw_on_git2_error(error);
    return {reference{upstream_raw}};
}

std::optional<std::string> reference::remote_name() const
{
    git_repository* repo = git_reference_owner(ref());
    char const* name = this->name();

    git_buf buf = GIT_BUF_INIT;
    int error = git_branch_remote_name(&buf, repo, name);
    // TODO: decide what to do with GIT_ENOTFOUND and GIT_EAMBIGUOUS
    if (error == GIT_ENOTFOUND) {
        fmt::println("no remote found for remote-tracking branch {}", name);
        return std::nullopt;
    }
    if (error == GIT_EAMBIGUOUS) {
        fmt::println("multiple remotes found for remote-tracking branch {}", name);
        return std::nullopt;
    }
    throw_on_git2_error(error);

    std::string rname{buf.ptr, buf.size};
    git_buf_dispose(&buf);

    return {rname};
}

bool reference::operator==(reference const& other) const
{
    return git_reference_cmp(ref(), other.ref()) == 0;
}

auto fmt::formatter<reference_type>::format(reference_type tp, format_context& ctx) const -> format_context::iterator
{
    string_view name = "unknown";
    switch (tp) {
        case reference_type::invalid:  name = "invalid";  break;
        case reference_type::direct:   name = "direct";   break;
        case reference_type::symbolic: name = "symbolic"; break;
    }
    return formatter<string_view>::format(name, ctx);
}
