#pragma once

#include <git2/oid.h>
#include <fmt/core.h>
#include <memory>
#include <string>

struct git_oid;

namespace git {

    class oid {

        git_oid m_oid;

    public:
        // makes a copy of the given oid
        oid();
        oid(git_oid const& the_oid);

        git_oid const* get() const { return &m_oid; }

        bool is_zero() const;

        bool operator==(oid const& other) const;
        bool operator!=(oid const& other) const { return !(*this == other); }

        int compare(oid const& other) const;
        bool operator<(oid const& other) const { return compare(other) < 0; }
        bool operator<=(oid const& other) const { return compare(other) <= 0; }
        bool operator>(oid const& other) const { return compare(other) > 0; }
        bool operator>=(oid const& other) const { return compare(other) >= 0; }
    };

}

template <>
struct fmt::formatter<git::oid> : formatter<std::string>
{
    auto format(git::oid o, format_context& ctx) const -> format_context::iterator;
};
