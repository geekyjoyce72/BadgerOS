
// SPDX-License-Identifier: MIT

#pragma once

#include "userland/types.h"

extern process_t dummy_proc;

static inline process_t *get_process(pid_t pid) {
    (void)pid;
    return &dummy_proc;
}
