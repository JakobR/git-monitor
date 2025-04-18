#pragma once

#include "repository.h"
#include "reference.h"
#include "remote.h"
#include "oid.h"
#include <ostream>

namespace git {

    void libgit2_init();
    void libgit2_shutdown();

    struct version_t {
        int major;
        int minor;
        int rev;

        std::ostream& display(std::ostream& out) const;
    };

    inline std::ostream& operator<<(std::ostream& out, version_t const& v) { return v.display(out); }

    version_t libgit2_compile_version();
    version_t libgit2_runtime_version();

}
