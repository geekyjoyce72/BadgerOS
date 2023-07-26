
// SPDX-License-Identifier: MIT

#include "cpu/syscall.h"

typedef enum {
    // Temporary thread yield system call.
    SYSNUM_THREAD_YIELD = 0,
    SYSNUM_MAX
} syscall_num_t;



// Temporary thread yield system call.
static inline void syscall_thread_yield() {
    syscall_0(SYSNUM_THREAD_YIELD);
}
