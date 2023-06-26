
// SPDX-License-Identifier: MIT

#pragma once

#ifndef __ASSEMBLER__
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#endif

// Word size of the stack used for interrupt and trap handlers.
#define ISR_STACK_DEPTH 4096

#ifdef __ASSEMBLER__

#define STRUCT_BEGIN(structname)
#define STRUCT_FIELD_WORD(structname, name, offset) \
	.equ structname ## _ ## name, offset
#define STRUCT_FIELD_PTR(structname, type, name, offset) \
	.equ structname ## _ ## name, offset
#define STRUCT_END(structname)

#else

#define STRUCT_BEGIN(structname) \
	typedef struct {
#define STRUCT_FIELD_WORD(structname, name, offset) \
	uint32_t name;
#define STRUCT_FIELD_PTR(structname, type, name, offset) \
	type *name;
#define STRUCT_END(structname) \
	} structname;

#endif

#ifdef __ASSEMBLER__
	# Interrupt vector table implemented in ASM.
	.global __interrupt_vector_table
#else
// Interrupt vector table implemented in ASM.
extern void *__interrupt_vector_table[32];
// Called from ASM on interrupt.
extern void __interrupt_handler();
// Called from ASM on system call.
extern void __syscall_handler(long a0, long a1, long a2, long a3, long a4, long a5, long a6, long sysno);
// Called from ASM on non-system call trap.
extern void __trap_handler();
#endif


// RISC-V register file copy.
STRUCT_BEGIN(riscv_regs_t)
STRUCT_FIELD_WORD(riscv_regs_t, pc, 0)
STRUCT_FIELD_WORD(riscv_regs_t, ra, 4)
STRUCT_FIELD_WORD(riscv_regs_t, sp, 8)
STRUCT_FIELD_WORD(riscv_regs_t, gp, 12)
STRUCT_FIELD_WORD(riscv_regs_t, tp, 16)
STRUCT_FIELD_WORD(riscv_regs_t, t0, 20)
STRUCT_FIELD_WORD(riscv_regs_t, t1, 24)
STRUCT_FIELD_WORD(riscv_regs_t, t2, 28)
STRUCT_FIELD_WORD(riscv_regs_t, s0, 32)
STRUCT_FIELD_WORD(riscv_regs_t, s1, 36)
STRUCT_FIELD_WORD(riscv_regs_t, a0, 40)
STRUCT_FIELD_WORD(riscv_regs_t, a1, 44)
STRUCT_FIELD_WORD(riscv_regs_t, a2, 48)
STRUCT_FIELD_WORD(riscv_regs_t, a3, 52)
STRUCT_FIELD_WORD(riscv_regs_t, a4, 56)
STRUCT_FIELD_WORD(riscv_regs_t, a5, 60)
STRUCT_FIELD_WORD(riscv_regs_t, a6, 64)
STRUCT_FIELD_WORD(riscv_regs_t, a7, 68)
STRUCT_FIELD_WORD(riscv_regs_t, s2, 72)
STRUCT_FIELD_WORD(riscv_regs_t, s3, 76)
STRUCT_FIELD_WORD(riscv_regs_t, s4, 80)
STRUCT_FIELD_WORD(riscv_regs_t, s5, 84)
STRUCT_FIELD_WORD(riscv_regs_t, s6, 88)
STRUCT_FIELD_WORD(riscv_regs_t, s7, 92)
STRUCT_FIELD_WORD(riscv_regs_t, s8, 96)
STRUCT_FIELD_WORD(riscv_regs_t, s9, 100)
STRUCT_FIELD_WORD(riscv_regs_t, s10, 104)
STRUCT_FIELD_WORD(riscv_regs_t, s11, 108)
STRUCT_FIELD_WORD(riscv_regs_t, t3, 112)
STRUCT_FIELD_WORD(riscv_regs_t, t4, 116)
STRUCT_FIELD_WORD(riscv_regs_t, t5, 120)
STRUCT_FIELD_WORD(riscv_regs_t, t6, 124)
STRUCT_END(riscv_regs_t)


// Kernel thread context.
STRUCT_BEGIN(isr_ctx_t)
// Scratch words for use by the ASM code.
STRUCT_FIELD_WORD(isr_ctx_t, scratch0, 0)
STRUCT_FIELD_WORD(isr_ctx_t, scratch1, 4)
STRUCT_FIELD_WORD(isr_ctx_t, scratch2, 8)
STRUCT_FIELD_WORD(isr_ctx_t, scratch3, 12)
STRUCT_FIELD_WORD(isr_ctx_t, scratch4, 16)
STRUCT_FIELD_WORD(isr_ctx_t, scratch5, 20)
STRUCT_FIELD_WORD(isr_ctx_t, scratch6, 24)
STRUCT_FIELD_WORD(isr_ctx_t, scratch7, 28)
// Pointer to registers storage.
STRUCT_FIELD_PTR(isr_ctx_t, riscv_regs_t, regs, 32)
STRUCT_END(isr_ctx_t)



#undef STRUCT_BEGIN
#undef STRUCT_FIELD_WORD
#undef STRUCT_FIELD_PTR
#undef STRUCT_END
