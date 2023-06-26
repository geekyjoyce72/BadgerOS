
// SPDX-License-Identifier: MIT

#pragma once

#ifndef __ASSEMBLER__
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#endif
#include <regs.h>

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


// Kernel thread context.
STRUCT_BEGIN(kernel_ctx_t)
// Scratch words for use by the ASM code.
STRUCT_FIELD_WORD(kernel_ctx_t, scratch0, 0)
STRUCT_FIELD_WORD(kernel_ctx_t, scratch1, 4)
STRUCT_FIELD_WORD(kernel_ctx_t, scratch2, 8)
STRUCT_FIELD_WORD(kernel_ctx_t, scratch3, 12)
STRUCT_FIELD_WORD(kernel_ctx_t, scratch4, 16)
STRUCT_FIELD_WORD(kernel_ctx_t, scratch5, 20)
STRUCT_FIELD_WORD(kernel_ctx_t, scratch6, 24)
STRUCT_FIELD_WORD(kernel_ctx_t, scratch7, 28)
// Pointer to registers storage.
STRUCT_FIELD_PTR(kernel_ctx_t, cpu_regs_t, regs, 32)
STRUCT_END(kernel_ctx_t)



#undef STRUCT_BEGIN
#undef STRUCT_FIELD_WORD
#undef STRUCT_FIELD_PTR
#undef STRUCT_END
