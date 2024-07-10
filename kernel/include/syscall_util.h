
// SPDX-License-Identifier: MIT

#pragma once

#include "memprotect.h"
#include "syscall.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SYSUTIL_EC_WRAPPER_V(func, ...)                                                                                \
    {                                                                                                                  \
        badge_err_t ec_buf;                                                                                            \
        func(&ec_buf, __VA_ARGS__);                                                                                    \
        if (ec) {                                                                                                      \
            sigsegv_assert(copy_to_user(proc_current_pid(), (size_t)ec, &ec_buf, sizeof(badge_err_t)));                \
        }                                                                                                              \
    }

#define SYSUTIL_EC_WRAPPER(rettype, func, ...)                                                                         \
    ({                                                                                                                 \
        badge_err_t ec_buf;                                                                                            \
        rettype     ec_rettmp = func(&ec_buf, __VA_ARGS__);                                                            \
        if (ec) {                                                                                                      \
            sigsegv_assert(copy_to_user(proc_current_pid(), (size_t)ec, &ec_buf, sizeof(badge_err_t)));                \
        }                                                                                                              \
        ec_rettmp;                                                                                                     \
    })

#define badge_err_userset(ec, loc, cause)                                                                              \
    {                                                                                                                  \
        badge_err_t ec_buf = {(loc), (cause)};                                                                         \
        sigsegv_assert(copy_to_user(proc_current_pid(), (size_t)ec, &ec_buf, sizeof(badge_err_t)));                    \
    }



// Assert that a condition is true, or raise SIGSYS and don't return.
void sigsys_assert(bool condition);
// Assert that a condition is true, or raise SIGSEGV and don't return.
void sigsegv_assert(bool condition);
// Checks whether the process has permission for a range of memory.
bool sysutil_memperm(void const *ptr, size_t len, uint32_t flags);
// If the process does not have access, raise SIGSEGV and don't return.
void sysutil_memassert(void const *ptr, size_t len, uint32_t flags);
// Assert the process can read memory, or raise SIGSEGV and don't return.
#define sysutil_memassert_r(ptr, len)  sysutil_memassert(ptr, len, MEMPROTECT_FLAG_R)
// Assert the process can read/write memory, or raise SIGSEGV and don't return.
#define sysutil_memassert_rw(ptr, len) sysutil_memassert(ptr, len, MEMPROTECT_FLAG_RW)
// Assert the process can execute memory, or raise SIGSEGV and don't return.
#define sysutil_memassert_x(ptr, len)  sysutil_memassert(ptr, len, MEMPROTECT_FLAG_X)
