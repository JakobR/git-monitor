#pragma once

#include "oid.h"
#include <fmt/core.h>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

struct git_credential;
struct git_remote;

namespace git {

    struct remote_ref {
        std::string name;
        oid id;
    };

    struct credential {
        std::string username;
        std::string password;
    };

    class remote {

        struct git_remote_deleter {
            void operator()(git_remote* ptr) const;
        };

        std::unique_ptr<git_remote, git_remote_deleter> m_remote;

        struct callbacks_t;
        std::unique_ptr<callbacks_t> m_callbacks;

    public:
        /// takes ownership of the given git_remote
        explicit remote(git_remote* remote);
        ~remote() noexcept;
        remote(remote const&) = delete;
        remote& operator=(remote const&) = delete;
        remote(remote&&);
        remote& operator=(remote&&);

        /// may be NULL for anonymous/in-memory remotes
        char const* name() const;

        bool is_connected() const;

        // only supports fetch direction for now
        void connect_fetch();
        void disconnect();

        // get the remote repository's reference advertisement list
        std::vector<remote_ref> ls();

        // transform a (local) remote-tracking branch name to the corresponding remote branch name
        // e.g., typically "refs/remotes/origin/main" on the local repo is fetched from "refs/heads/main" on the remote repo,
        // in which case this function would transform "refs/remotes/origin/main" into "refs/heads/main".
        std::optional<std::string> get_remote_branch(char const* remote_tracking_name);

        using acquire_credentials_t = std::function<std::optional<credential>(char const* url, char const* username_from_url)>;
        void set_acquire_credentials_callback(acquire_credentials_t callback);
    };

}
