
// SPDX-License-Identifier: MIT

#include "syscall.h"

#include "cpu/isr.h"
#include "cpu/isr_ctx.h"
#include "cpu/panic.h"
#include "log.h"
#include "process/internal.h"
#include "process/process.h"
#include "rawprint.h"
#include "scheduler/cpu.h"
#include "scheduler/scheduler.h"

// Syscall implementations.
#include "process/syscall_impl.h"



// Called on invalid system call.
static void invalid_syscall(long sysno) {
    isr_ctx_t *ctx = isr_ctx_get();

    // Report error.
    rawprint("Invalid syscall ");
    rawprintdec(sysno, 0);
    rawprint(" at PC 0x");
    rawprinthex(ctx->regs.pc, sizeof(ctx->regs.pc) * 2);
    rawprint("\n");
    isr_ctx_dump(ctx);

    // Terminate thread.
    proc_exit_self(-1);
}

// Shutdown system call implementation.
extern void syscall_sys_shutdown(bool is_reboot);

// System call handler jump table thing.
__SYSCALL_HANDLER_SIGNATURE {
    __SYSCALL_HANDLER_IGNORE_UNUSED;

    long long retval = 0;
    switch (sysnum) {
        case SYSCALL_TEMP_WRITE:
            mutex_acquire(NULL, &log_mtx, TIMESTAMP_US_MAX);
            rawprint_substr((char const *)a0, (size_t)a1);
            mutex_release(NULL, &log_mtx);
            break;
        case SYSCALL_THREAD_YIELD: sched_yield(); break;
        case SYSCALL_SELF_EXIT: syscall_self_exit(a0); break;
        case SYSCALL_SYS_SHUTDOWN: syscall_sys_shutdown(a0); break;
        default: invalid_syscall(sysnum); break;
    }
    __syscall_return(retval);
}
