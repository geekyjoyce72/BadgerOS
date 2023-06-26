
// SPDX-License-Identifier: MIT

#include <isr.h>
#include <log.h>

// Called from ASM on interrupt.
void __interrupt_handler() {
	
}

// Called from ASM on system call.
void __syscall_handler(long a0, long a1, long a2, long a3, long a4, long a5, long a6, long sysno) {
	logk(LOG_DEBUG, "The system call!");
}

// Called from ASM on non-system call trap.
void __trap_handler() {
	
}