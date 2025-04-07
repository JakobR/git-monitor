#pragma once

#include "reference.h"
#include <memory>
#include <optional>
#include <utility>

struct git_branch_iterator;

namespace git {

    enum class branch_type {
        /// local branch
        local,
        /// remote-tracking branch
        remote,
    };

    class branch_iterator {

        struct git_branch_iterator_deleter {
            void operator()(git_branch_iterator* ptr) const;
        };

        std::unique_ptr<git_branch_iterator, git_branch_iterator_deleter> m_iter;

    public:
        /// takes ownership of the given git_branch_iterator.
        explicit branch_iterator(git_branch_iterator* iter);
        ~branch_iterator() noexcept;
        branch_iterator(branch_iterator const&) = delete;
        branch_iterator& operator=(branch_iterator const&) = delete;
        branch_iterator(branch_iterator&&) = default;
        branch_iterator& operator=(branch_iterator&&) = default;

        std::optional<std::pair<reference, branch_type>> next();
    };

}
