
// SPDX-License-Identifier: MIT

#include "syscall.h"

#include "cpu/isr.h"
#include "cpu/panic.h"
#include "rawprint.h"
#include "scheduler.h"



// Called on invalid system call.
static long long invalid_syscall(long sysno) {
    kernel_ctx_t *ctx = kernel_ctx_get();

    // Report error.
    rawprint("Invalid syscall ");
    rawprintdec(sysno, 0);
    rawprint(" at PC 0x");
    rawprinthex(ctx->regs.pc, sizeof(ctx->regs.pc) * 2);
    rawprint("\n");
    kernel_ctx_dump(ctx);

    // TODO: Terminate thread.
    panic_poweroff();

    return 0;
}



// Temporary thread yield system call.
static long long syscall_impl_thread_yield() {
    sched_request_switch_from_isr();
    return 0;
}

// System call handler jump table thing.
__SYSCALL_HANDLER_SIGNATURE {
    __SYSCALL_HANDLER_IGNORE_UNUSED;

    switch (sysnum) {
        case SYSNUM_THREAD_YIELD: return syscall_impl_thread_yield();
        default: return invalid_syscall(sysnum);
    }
}
