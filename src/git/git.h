#pragma once

#include "repository.h"
#include "reference.h"
#include "remote.h"
#include "oid.h"

namespace git {

    void libgit2_init();
    void libgit2_shutdown();

    struct version_t {
        int major;
        int minor;
        int rev;
    };

    version_t libgit2_version();

}
