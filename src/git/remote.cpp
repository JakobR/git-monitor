#include "remote.h"
#include "oid.h"
#include "util.h"
#include <git2.h>

using namespace git;

struct remote::callbacks_t {
    acquire_credentials_t acquire_credentials;

    callbacks_t() = default;
    ~callbacks_t() = default;

    // disallow copy/move to guarantee stable pointer
    callbacks_t(callbacks_t const&) = delete;
    callbacks_t& operator=(callbacks_t const&) = delete;
    callbacks_t(callbacks_t&&) = delete;
    callbacks_t& operator=(callbacks_t&&) = delete;

    static int credential_acquire_cb(git_credential** out, char const* url, char const* username_from_url, unsigned int allowed_types, void* payload);
};

remote::remote(git_remote* remote)
    : m_remote{remote}, m_callbacks{std::make_unique<callbacks_t>()}
{
    if (!m_remote)
        throw std::invalid_argument("git_remote* is null");
}

void remote::git_remote_deleter::operator()(git_remote* ptr) const
{
    git_remote_free(ptr);
}

remote::~remote() noexcept = default;
remote::remote(remote&&) = default;
remote& remote::operator=(remote&&) = default;

char const* remote::name() const
{
    return git_remote_name(m_remote.get());
}

bool remote::is_connected() const
{
    return git_remote_connected(m_remote.get());
}

void remote::set_acquire_credentials_callback(acquire_credentials_t callback)
{
    m_callbacks->acquire_credentials = std::move(callback);
}

int remote::callbacks_t::credential_acquire_cb(git_credential** out, char const* url, char const* username_from_url, unsigned int allowed_types, void* payload)
{
    callbacks_t* cb = static_cast<callbacks_t*>(payload);

    // we only support username/password authentication for now
    if (allowed_types & GIT_CREDENTIAL_USERPASS_PLAINTEXT) {
        if (cb->acquire_credentials) {
            auto cred = cb->acquire_credentials(url, username_from_url);
            if (cred) {
                int error = git_credential_userpass_plaintext_new(out, cred->username.c_str(), cred->password.c_str());
                throw_on_git2_error(error);
                return 0;  // success
            }
        }
    }
    else {
        fmt::println("no supported credential types: {}", allowed_types);
    }

    // return value:
    //  0 for success
    //  <0 to indicate an error
    //  >0 to indicate no credential was acquired
    return 1;
}

void remote::connect_fetch()
{
    git_remote_callbacks callbacks = GIT_REMOTE_CALLBACKS_INIT;
    callbacks.payload = m_callbacks.get();  // we need a stable pointer as callback payload
    callbacks.credentials = callbacks_t::credential_acquire_cb;

    int error = git_remote_connect(m_remote.get(), GIT_DIRECTION_FETCH, &callbacks, nullptr, nullptr);
    throw_on_git2_error(error);
}

void remote::disconnect()
{
    int error = git_remote_disconnect(m_remote.get());
    throw_on_git2_error(error);
}

std::vector<remote_ref> remote::ls()
{
    git_remote_head const** remote_heads;
    size_t num_remote_heads;
    int error = git_remote_ls(&remote_heads, &num_remote_heads, m_remote.get());
    throw_on_git2_error(error);

    std::vector<remote_ref> rrs;
    for (size_t i = 0; i < num_remote_heads; ++i) {
        git_remote_head const* remote_head = remote_heads[i];
        // fmt::println("remote_head #{}:", i);
        // fmt::println("    name: {}", remote_head->name);
        // fmt::println("    symref_target: {}", remote_head->symref_target ? remote_head->symref_target : "<null>");
        // fmt::println("    locally available: {}", remote_head->local);
        // fmt::println("    oid: {}", oid{remote_head->oid});
        // fmt::println("    local oid: {}", oid{remote_head->loid});
        remote_ref rr;
        rr.name = remote_head->name;
        rr.id = remote_head->oid;
        rrs.push_back(std::move(rr));
    }
    return rrs;
}

std::optional<std::string> remote::get_remote_branch(char const* remote_tracking_name)
{
    size_t refspec_count = git_remote_refspec_count(m_remote.get());
    for (size_t i = 0; i < refspec_count; ++i) {
        git_refspec const* refspec = git_remote_get_refspec(m_remote.get(), i);
        if (git_refspec_direction(refspec) != GIT_DIRECTION_FETCH)
            continue;
        if (!git_refspec_dst_matches(refspec, remote_tracking_name))
            continue;

        git_buf buf = GIT_BUF_INIT;
        int error = git_refspec_rtransform(&buf, refspec, remote_tracking_name);
        throw_on_git2_error(error);

        std::string remote_branch{buf.ptr, buf.size};
        git_buf_dispose(&buf);
        return {std::move(remote_branch)};
    }
    // TODO: should we check if multiple refspecs match?
    fmt::println("no matching refspec");
    return std::nullopt;
}
