#include "repository.h"
#include "util.h"
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/std.h>
#include <git2.h>
#include <map>

using namespace git;

repository::repository(git_repository* repo)
    : m_repo(repo)
{
    if (!m_repo)
        throw std::invalid_argument("git_repository* is null");
}

void repository::git_repository_deleter::operator()(git_repository* ptr) const
{
    git_repository_free(ptr);
}

repository::~repository() noexcept
{ }

repository repository::open(char const* path)
{
    git_repository* repo = nullptr;
    int error = git_repository_open(&repo, path);
    throw_on_git2_error(error);
    return repository(repo);
}

bool repository::is_bare() const
{
    return git_repository_is_bare(repo());
}

bool repository::is_worktree() const
{
    return git_repository_is_worktree(repo());
}

char const* repository::path() const
{
    return git_repository_path(repo());
}

char const* repository::workdir() const
{
    return git_repository_workdir(repo());
}

char const* repository::commondir() const
{
    return git_repository_commondir(repo());
}

bool repository::is_head_detached()
{
    int result = git_repository_head_detached(repo());
    throw_on_git2_error(result);
    return (result >= 1);
}

reference repository::head()
{
    git_reference* head = nullptr;
    int error = git_repository_head(&head, repo());
    throw_on_git2_error(error);
    return reference{head};
}

ahead_behind_t repository::graph_ahead_behind(oid const& local, oid const& upstream)
{
    ahead_behind_t result;
    int error = git_graph_ahead_behind(&result.ahead, &result.behind, repo(), local.get(), upstream.get());
    throw_on_git2_error(error);
    return result;
}

std::optional<reference> repository::lookup_local_branch(char const* name)
{
    git_reference* branch_raw;
    int error = git_branch_lookup(&branch_raw, repo(), name, GIT_BRANCH_LOCAL);
    if (error == GIT_ENOTFOUND)
        return std::nullopt;
    throw_on_git2_error(error);
    return {reference(branch_raw)};
}

std::vector<reference> repository::local_branches()
{
    git_branch_iterator* iter_raw;
    int error = git_branch_iterator_new(&iter_raw, repo(), GIT_BRANCH_LOCAL);
    throw_on_git2_error(error);

    branch_iterator iter{iter_raw};
    std::vector<reference> branches;
    while (auto branch = iter.next())
        branches.push_back(std::move(branch->first));
    return branches;
}

std::optional<ahead_behind_t> repository::branch_ahead_behind(reference const& local)
{
    if (!local.is_branch())
        throw std::invalid_argument("reference must be a local branch");
    std::optional<reference> upstream = local.branch_upstream();
    if (!upstream)
        return std::nullopt;
    std::optional<oid> local_oid = local.resolve().target();
    std::optional<oid> upstream_oid = upstream->resolve().target();
    if (!local_oid)
        return std::nullopt;
    if (!upstream_oid)
        return std::nullopt;
    return graph_ahead_behind(*local_oid, *upstream_oid);
}

std::optional<ahead_behind_t> repository::head_ahead_behind()
{
    if (is_head_detached())
        return std::nullopt;
    reference head = this->head();
    if (!head.is_branch())
        return std::nullopt;
    return branch_ahead_behind(head);
}

ahead_behind_t repository::total_ahead_behind()
{
    ahead_behind_t total;

    for (auto const& branch : local_branches()) {
        fmt::println("local branch: {}", branch.name());
        auto ab = branch_ahead_behind(branch);
        if (!ab)
            continue;
        fmt::println("{} ahead, {} behind", ab->ahead, ab->behind);
        total.ahead += ab->ahead;
        total.behind += ab->behind;
    }

    return total;
}

namespace {
    struct status_data {
        size_t uncommitted = 0;
    };

    int status_cb(char const* path, unsigned int status_flags, void* payload)
    {
        status_data* data = static_cast<status_data*>(payload);
        unsigned int const uncommitted_change_mask = 0
            | GIT_STATUS_INDEX_NEW
            | GIT_STATUS_INDEX_MODIFIED
            | GIT_STATUS_INDEX_DELETED
            | GIT_STATUS_INDEX_RENAMED
            | GIT_STATUS_INDEX_TYPECHANGE
            | GIT_STATUS_WT_NEW
            | GIT_STATUS_WT_MODIFIED
            | GIT_STATUS_WT_DELETED
            | GIT_STATUS_WT_TYPECHANGE
            | GIT_STATUS_WT_RENAMED
            | GIT_STATUS_WT_UNREADABLE
            | GIT_STATUS_CONFLICTED;  // we treat unreadable and conflicted files as uncommitted change
        if ((status_flags & uncommitted_change_mask) != 0) {
            data->uncommitted += 1;
        }
        return 0;
    }
}

size_t repository::uncommitted_changes()
{
    git_status_options opts = GIT_STATUS_OPTIONS_INIT;
    opts.flags =
        GIT_STATUS_OPT_INCLUDE_UNTRACKED |
        GIT_STATUS_OPT_INCLUDE_UNREADABLE |
        GIT_STATUS_OPT_NO_REFRESH;
    status_data data;
    int error = git_status_foreach_ext(repo(), &opts, status_cb, &data);
    throw_on_git2_error(error);
    return data.uncommitted;
}


std::vector<std::string> repository::remotes()
{
    git_strarray remotes_raw = {0};
    int error = git_remote_list(&remotes_raw, repo());
    throw_on_git2_error(error);
    std::vector<std::string> remotes;
    for (size_t i = 0; i < remotes_raw.count; ++i) {
        char const* name = remotes_raw.strings[i];
        remotes.emplace_back(name);
    }
    git_strarray_dispose(&remotes_raw);
    return remotes;
}

std::optional<remote> repository::lookup_remote(char const* name)
{
    git_remote* remote_raw;
    int error = git_remote_lookup(&remote_raw, repo(), name);
    if (error == GIT_ENOTFOUND)
        return std::nullopt;
    throw_on_git2_error(error);
    return {remote{remote_raw}};
}

remote_state_t repository::check_remote_state()
{
    remote_state_t result;
    std::vector<std::string>& errors = result.errors;

    // Consider the local branch "refs/heads/main".
    // The configured upstream is the remote-tracking branch "refs/remotes/origin/main".
    // The corresponding remote is "origin".
    // The fetch refspec on "origin" is "+refs/heads/*:refs/remotes/origin/*":
    // this means the (local) remote-tracking branch "refs/remotes/origin/main" is fetched from the remote branch "refs/heads/main".
    //
    // The goal of this method is to check for all (relevant) remote-tracking branches whether
    // they are up-to-date with the remote branch.
    // - relevant means: checked out as a local branch
    // - up-to-date means: the advertised commit id is the same as the remote-tracking branch's id

    reference head = this->head();

    struct branch_info {
        reference local;
        reference upstream;
        oid upstream_oid;
        branch_state state = branch_state::unknown;
    };

    size_t branches_without_upstream = 0;  // these count as up-to-date
    std::vector<branch_info> bis;

    for (reference& local : local_branches()) {
        fmt::println("local branch: {}", local.name());
        if (local == head)
            fmt::println("    is HEAD");
        std::optional<reference> upstream = local.branch_upstream();
        if (!upstream) {
            branches_without_upstream += 1;
            continue;
        }
        fmt::println("    upstream: {}", upstream->name());
        std::optional<oid> upstream_oid = upstream->resolve().target();
        if (!upstream_oid) {
            // TODO: these should probably count as outdated, if a corresponding remote is configured.
            continue;
        }
        fmt::println("    upstream oid: {}", *upstream_oid);
        branch_info bi {
            .local = std::move(local),
            .upstream = std::move(*upstream),
            .upstream_oid = *upstream_oid,
        };
        bis.push_back(std::move(bi));
    }

    for (std::string const& remote_name : remotes()) {
        std::optional<remote> remote = lookup_remote(remote_name.c_str());
        fmt::println("Querying remote {}...", remote->name());
        if (!remote)
            continue;

        using remote_branch_name_t = std::string;
        std::map<remote_branch_name_t, std::vector<size_t>> remote_branch_to_info;

        for (size_t i = 0; i < bis.size(); ++i) {
            branch_info const& bi = bis[i];
            std::optional<std::string> remote_branch = remote->get_remote_branch(bi.upstream.name());
            if (!remote_branch)
                continue;
            if (bi.state != branch_state::unknown) {
                fmt::println("    WARN: local branch {} matches multiple remotes", bi.local.name());
                errors.push_back(fmt::format("warning: local branch '{}' matches multiple remotes", bi.local.name()));
                continue;
            }
            fmt::println("    remote-tracking branch {} is fetched from remote branch {}", bi.upstream.name(), *remote_branch);
            remote_branch_to_info[*remote_branch].push_back(i);
        }

        // no local branches match this remote, so we do not need to connect
        if (remote_branch_to_info.empty())
            continue;

        char const* error_msg = "";
        std::vector<remote_ref> remote_refs;
        try {
            error_msg = "connect to";
            remote->connect_fetch();
            error_msg = "list";
            remote_refs = remote->ls();
        }
        catch (std::exception const& e) {
            errors.push_back(fmt::format("unable to {} remote '{}': {}", error_msg, remote_name, e.what()));
            for (auto const& item : remote_branch_to_info)
                for (size_t i : item.second)
                    bis[i].state = branch_state::connection_error;
            continue;
        }

        for (remote_ref const& rr : remote_refs) {
            fmt::println("    remote_ref {} is at {}", rr.name, rr.id);

            auto it = remote_branch_to_info.find(rr.name);
            if (it == remote_branch_to_info.end())
                continue;
            for (size_t i : it->second) {
                branch_info& bi = bis[i];
                if (bi.upstream_oid == rr.id) {
                    fmt::println("    up-to-date");
                    bi.state = branch_state::up_to_date;
                }
                else {
                    fmt::println("    mismatch!");
                    bi.state = branch_state::outdated;
                }
            }
        }

        try {
            remote->disconnect();
        }
        catch (std::exception const& e) {
            errors.push_back(fmt::format("unable to disconnect from remote '{}': {}", remote_name, e.what()));
        }
    }

    for (branch_info const& bi : bis) {
        if (bi.local == head)
            result.head_state = bi.state;
        if (bi.state == branch_state::up_to_date)
            result.branches_up_to_date += 1;
        if (bi.state == branch_state::outdated)
            result.branches_outdated += 1;
    }
    result.branches_up_to_date += branches_without_upstream;

    return result;
}

auto fmt::formatter<branch_state>::format(branch_state tp, format_context& ctx) const -> format_context::iterator
{
    string_view name = "<invalid>";
    switch (tp) {
        case branch_state::unknown:          name = "unknown";          break;
        case branch_state::up_to_date:       name = "up to date";       break;
        case branch_state::outdated:         name = "outdated";         break;
        case branch_state::connection_error: name = "connection error"; break;
    }
    return formatter<string_view>::format(name, ctx);
}
