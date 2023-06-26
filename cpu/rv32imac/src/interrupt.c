
// SPDX-License-Identifier: MIT

#include <isr.h>
#include <interrupt.h>

// Install interrupt and trap handlers.
void interrupt_init(isr_ctx_t *ctx, isr_regs_t *regs) {
	// Disable interrupts.
	asm volatile ("csrc mstatus, %0" :: "r" (0x0000000f));
	// Disable interrupt delegation.
	asm volatile ("csrw mideleg, %0" :: "r" (0x00000000));
	
	// Set up trap handler (vectored mode; 256 byte-aligned).
	asm volatile ("csrw mtvec, %0" :: "r" ((size_t) &__interrupt_vector_table | 1));
	
	// Set up trap context.
	ctx->regs = regs;
	asm volatile ("csrw mscratch, %0" :: "r" (ctx));
	
	// Re-enable interrupts.
	asm volatile ("csrs mstatus, %0" :: "r" (0x00000008));
}
