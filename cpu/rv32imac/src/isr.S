
# SPDX-License-Identifier: MIT

#include <isr.h>
	.global __global_pointer$



	# Entry code for an ISR; saves all registers.
	.macro isr_entry
	# Save tempregs t0-t3.
	csrrw t0, mscratch, t0
	sw t1, kernel_ctx_t_scratch1(t0)
	csrrw t1, mscratch, t0
	sw t1, kernel_ctx_t_scratch0(t0)
	lw t1, kernel_ctx_t_regs(t0)
	sw t2, riscv_regs_t_t2(t1)
	sw t3, riscv_regs_t_t3(t1)
	
	# Save all regs.
	sw ra, riscv_regs_t_ra(t1)
	sw sp, riscv_regs_t_sp(t1)
	sw gp, riscv_regs_t_gp(t1)
	sw tp, riscv_regs_t_tp(t1)
	sw s0, riscv_regs_t_s0(t1)
	sw s1, riscv_regs_t_s1(t1)
	sw a0, riscv_regs_t_a0(t1)
	sw a1, riscv_regs_t_a1(t1)
	sw a2, riscv_regs_t_a2(t1)
	sw a3, riscv_regs_t_a3(t1)
	sw a4, riscv_regs_t_a4(t1)
	sw a5, riscv_regs_t_a5(t1)
	sw a6, riscv_regs_t_a6(t1)
	sw a7, riscv_regs_t_a7(t1)
	sw s2, riscv_regs_t_s2(t1)
	sw s3, riscv_regs_t_s3(t1)
	sw s4, riscv_regs_t_s4(t1)
	sw s5, riscv_regs_t_s5(t1)
	sw s6, riscv_regs_t_s6(t1)
	sw s7, riscv_regs_t_s7(t1)
	sw s8, riscv_regs_t_s8(t1)
	sw s9, riscv_regs_t_s9(t1)
	sw s10, riscv_regs_t_s10(t1)
	sw s11, riscv_regs_t_s11(t1)
	sw t4, riscv_regs_t_t4(t1)
	sw t5, riscv_regs_t_t5(t1)
	sw t6, riscv_regs_t_t6(t1)
	
	# Save PC.
	csrr t2, mepc
	sw t2, riscv_regs_t_pc(t1)
	
	# Move tempregs.
	lw t2, kernel_ctx_t_scratch0(t0)
	sw t2, riscv_regs_t_t0(t1)
	lw t2, kernel_ctx_t_scratch1(t0)
	sw t2, riscv_regs_t_t1(t1)
	
	# Set up special regs.
	li tp, 0
	.option push
	.option norvc
	la gp, __global_pointer$
	.option pop
	la sp, __interrupt_stack_hi
	.endm



	# Exit code for an ISR; restores all registers.
	.macro isr_exit
	# Restore PC.
	csrr t0, mscratch
	lw t1, kernel_ctx_t_regs(t0)
	lw t2, riscv_regs_t_pc(t1)
	csrw mepc, t2
	
	# Restore all regs.
	lw ra, riscv_regs_t_ra(t1)
	lw sp, riscv_regs_t_sp(t1)
	lw gp, riscv_regs_t_gp(t1)
	lw tp, riscv_regs_t_tp(t1)
	lw s0, riscv_regs_t_s0(t1)
	lw s1, riscv_regs_t_s1(t1)
	lw a0, riscv_regs_t_a0(t1)
	lw a1, riscv_regs_t_a1(t1)
	lw a2, riscv_regs_t_a2(t1)
	lw a3, riscv_regs_t_a3(t1)
	lw a4, riscv_regs_t_a4(t1)
	lw a5, riscv_regs_t_a5(t1)
	lw a6, riscv_regs_t_a6(t1)
	lw a7, riscv_regs_t_a7(t1)
	lw s2, riscv_regs_t_s2(t1)
	lw s3, riscv_regs_t_s3(t1)
	lw s4, riscv_regs_t_s4(t1)
	lw s5, riscv_regs_t_s5(t1)
	lw s6, riscv_regs_t_s6(t1)
	lw s7, riscv_regs_t_s7(t1)
	lw s8, riscv_regs_t_s8(t1)
	lw s9, riscv_regs_t_s9(t1)
	lw s10, riscv_regs_t_s10(t1)
	lw s11, riscv_regs_t_s11(t1)
	lw t4, riscv_regs_t_t4(t1)
	lw t5, riscv_regs_t_t5(t1)
	lw t6, riscv_regs_t_t6(t1)
	
	# Restore tempregs t0-t3.
	lw t2, riscv_regs_t_t0(t1)
	csrw mscratch, t2
	lw t3, riscv_regs_t_t3(t1)
	lw t2, riscv_regs_t_t2(t1)
	lw t1, riscv_regs_t_t1(t1)
	csrrw t0, mscratch, t0
	.endm



	# Trap and system call handler.
	.text
	.type __trap_asm, %function
	.align 2
__trap_asm:
	isr_entry
	# Separate system calls from other traps.
	csrr t2, mcause
	li t3, 8
	beq t2, t3, .syscall
	li t3, 9
	beq t2, t3, .syscall
	li t3, 11
	beq t2, t3, .syscall
	
	# Handle normal traps.
	la ra, .trap_exit
	j __trap_handler
	
.syscall:
	# Handle system calls.
	lw t2, riscv_regs_t_pc(t1)
	addi t2, t2, 4
	sw t2, riscv_regs_t_pc(t1)
	jal __syscall_handler
	
.trap_exit:
	isr_exit
	mret



	# Interrupt handler.
	.text
	.type __isr_asm, %function
	.align 2
__isr_asm:
	isr_entry
	jal __interrupt_handler
	isr_exit
	mret



	# Interrupt and trap handler stack.
	.section ".bss"
__interrupt_stack_lo:
	.skip ISR_STACK_DEPTH*4
__interrupt_stack_hi:



	# Interrupt vector table.
	# This must be aligned to a 256-byte boundary, so it is in a special section.
	.section ".interrupt_vector_table"
__interrupt_vector_table:
	j __trap_asm
	j __isr_asm
	j __isr_asm
	j __isr_asm
	j __isr_asm
	j __isr_asm
	j __isr_asm
	j __isr_asm
	j __isr_asm
	j __isr_asm
	j __isr_asm
	j __isr_asm
	j __isr_asm
	j __isr_asm
	j __isr_asm
	j __isr_asm
	j __isr_asm
	j __isr_asm
	j __isr_asm
	j __isr_asm
	j __isr_asm
	j __isr_asm
	j __isr_asm
	j __isr_asm
	j __isr_asm
	j __isr_asm
	j __isr_asm
	j __isr_asm
	j __isr_asm
	j __isr_asm
	j __isr_asm
	j __isr_asm
	