
// SPDX-License-Identifier: MIT

#include "process/syscall_impl.h"

#include "process/internal.h"

// Sycall: Exit the process; exit code can be read by parent process.
// When this system call returns, the thread will be suspended awaiting process termination.
void syscall_self_exit(int code) {
    proc_exit_self(code);
}
