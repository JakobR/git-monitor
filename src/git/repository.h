#pragma once

#include "branch_iterator.h"
#include "reference.h"
#include "remote.h"
#include <memory>
#include <vector>
#include <string>
#include <utility>

struct git_repository;

namespace git {

    struct ahead_behind_t {
        size_t ahead = 0;
        size_t behind = 0;
    };

    enum class branch_state {
        unknown,
        // either no upstream configured, or upstream (i.e., remote-tracking branch) commit matches the remote commit id
        up_to_date,
        // upstream commit is different from remote commit
        outdated,
        // connection error
        connection_error,
    };

    struct remote_state_t {
        /// Whether HEAD has unfetched commits in the remote corresponding to its remote-tracking branch.
        /// This is also true if the remote commit exists in the repository but is different from the (local) remote-tracking branch (may happen if someone force-pushes an old commit).
        branch_state head_state = branch_state::unknown;
        size_t branches_up_to_date = 0;
        size_t branches_outdated = 0;
        std::vector<std::string> errors;
    };

    class repository {

        struct git_repository_deleter {
            void operator()(git_repository* ptr) const;
        };

        std::unique_ptr<git_repository, git_repository_deleter> m_repo;

        git_repository* repo() { return m_repo.get(); }
        git_repository const* repo() const { return m_repo.get(); }

    public:
        /// takes ownership of the given git_repository
        explicit repository(git_repository* repo);
        ~repository() noexcept;
        repository(repository const&) = delete;
        repository& operator=(repository const&) = delete;
        repository(repository&&) = default;
        repository& operator=(repository&&) = default;

        static repository open(char const* path);

        bool is_bare() const;
        bool is_worktree() const;

        /// path of the repository (the .git folder for normal repositories)
        char const* path() const;

        /// path of the working directory (always NULL for bare repositories)
        char const* workdir() const;

        char const* commondir() const;

        bool is_head_detached();
        reference head();

        ahead_behind_t graph_ahead_behind(oid const& local, oid const& upstream);

        /// Look up branch by name (e.g., "main")
        std::optional<reference> lookup_local_branch(char const* name);
        std::vector<reference> local_branches();
        std::optional<ahead_behind_t> branch_ahead_behind(reference const& local);

        std::optional<ahead_behind_t> head_ahead_behind();
        ahead_behind_t total_ahead_behind();

        // number of files with uncommitted changes (including untracked files).
        size_t uncommitted_changes();

        std::vector<std::string> remotes();
        std::optional<remote> lookup_remote(char const* name);

        remote_state_t check_remote_state();
    };

}

template <>
struct fmt::formatter<git::branch_state> : formatter<string_view>
{
    auto format(git::branch_state tp, format_context& ctx) const -> format_context::iterator;
};
