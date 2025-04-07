#pragma once

#include "oid.h"
#include <memory>
#include <optional>
#include <string>
#include <fmt/core.h>

struct git_reference;

namespace git {

    enum class reference_type {
        invalid,
        direct,
        symbolic,
    };

    class reference {

        struct git_reference_deleter {
            void operator()(git_reference* ptr) const;
        };

        std::unique_ptr<git_reference, git_reference_deleter> m_ref;

        git_reference* ref() { return m_ref.get(); }
        git_reference const* ref() const { return m_ref.get(); }

    public:
        /// takes ownership of the given git_reference
        explicit reference(git_reference* ref);
        ~reference() noexcept;
        reference(reference const&) = delete;
        reference& operator=(reference const&) = delete;
        reference(reference&&) = default;
        reference& operator=(reference&&) = default;

        reference_type type() const;
        char const* name() const;
        char const* shorthand() const;

        /// Resolves the reference to a direct reference.
        reference resolve() const;

        /// Only available if reference type is direct.
        std::optional<oid> target() const;

        /// Only available if reference type is symbolic.
        char const* symbolic_target() const;

        /// Check if the reference is a local branch.
        bool is_branch() const;

        /// Check if the reference is a remote-tracking branch.
        bool is_remote() const;

        /// Check if the reference is a tag.
        bool is_tag() const;

        /// Check if the reference is a note.
        bool is_note() const;

        /// The upstream branch of a local branch, if configured.
        std::optional<reference> branch_upstream() const;

        /// The remote name of a remote-tracking branch.
        std::optional<std::string> remote_name() const;

        bool operator==(reference const& other) const;
        bool operator!=(reference const& other) const { return !(*this == other); }
    };

}

template <>
struct fmt::formatter<git::reference_type> : formatter<string_view>
{
    auto format(git::reference_type tp, format_context& ctx) const -> format_context::iterator;
};
