
# SPDX-License-Identifier: MIT

#include "cpu/isr.h"
    .global __global_pointer$





    # Entry code for an ISR or trap handler; save T0-T3 and swap out SP/GP/TP.
    # Assumes valid `kernel_ctx_t *` in CSR mscratch.
    .macro isr_entry
#pragma region
    # Save tempregs t0/t1.
    csrrw t0, mscratch, t0
    sw t1, kernel_ctx_t_scratch1(t0)
    csrrw t1, mscratch, t0
    sw t1, kernel_ctx_t_scratch0(t0)
    
    sw t2, kernel_ctx_t_regs+cpu_regs_t_t2(t0)
    sw t3, kernel_ctx_t_regs+cpu_regs_t_t3(t0)
    
    sw ra, kernel_ctx_t_regs+cpu_regs_t_ra(t0)
    sw sp, kernel_ctx_t_regs+cpu_regs_t_sp(t0)
    sw gp, kernel_ctx_t_regs+cpu_regs_t_gp(t0)
    sw tp, kernel_ctx_t_regs+cpu_regs_t_tp(t0)
    
    # Clear context to switch field.
    sw x0, kernel_ctx_t_ctxswitch(t0)
    
    # Move tempregs.
    lw t1, kernel_ctx_t_scratch0(t0)
    sw t1, kernel_ctx_t_regs+cpu_regs_t_t0(t0)
    lw t1, kernel_ctx_t_scratch1(t0)
    sw t1, kernel_ctx_t_regs+cpu_regs_t_t1(t0)
    
    # Save PC.
    csrr t1, mepc
    sw t1, kernel_ctx_t_regs+cpu_regs_t_pc(t0)
    
    # Set up special regs.
    li tp, 0
    .option push
    .option norvc
    la gp, __global_pointer$
    .option pop
    la sp, __interrupt_stack_hi
#pragma endregion
    .endm



    # Exit code for an ISR or trap handler; restores tempregs and SP/GP/TP.
    # Assumes valid `kernel_ctx_t *` in t0.
    .macro isr_exit
#pragma region
    # Restore PC.
    lw t1, kernel_ctx_t_regs+cpu_regs_t_pc(t0)
    csrw mepc, t1
    
    lw ra, kernel_ctx_t_regs+cpu_regs_t_ra(t0)
    lw sp, kernel_ctx_t_regs+cpu_regs_t_sp(t0)
    lw gp, kernel_ctx_t_regs+cpu_regs_t_gp(t0)
    lw tp, kernel_ctx_t_regs+cpu_regs_t_tp(t0)
    
    # Restore tempregs t0-t3.
    lw t1, kernel_ctx_t_regs+cpu_regs_t_t0(t0)
    csrw mscratch, t1
    lw t3, kernel_ctx_t_regs+cpu_regs_t_t3(t0)
    lw t2, kernel_ctx_t_regs+cpu_regs_t_t2(t0)
    lw t1, kernel_ctx_t_regs+cpu_regs_t_t1(t0)
    csrrw t0, mscratch, t0
#pragma endregion
    .endm



    # Save all regs not saved by `isr_entry`.
    # Assumes valid `kernel_ctx_t *` in t0.
    .macro save_regs
#pragma region
    # Save all regs.
    sw s0,  kernel_ctx_t_regs+cpu_regs_t_s0(t0)
    sw s1,  kernel_ctx_t_regs+cpu_regs_t_s1(t0)
    sw a0,  kernel_ctx_t_regs+cpu_regs_t_a0(t0)
    sw a1,  kernel_ctx_t_regs+cpu_regs_t_a1(t0)
    sw a2,  kernel_ctx_t_regs+cpu_regs_t_a2(t0)
    sw a3,  kernel_ctx_t_regs+cpu_regs_t_a3(t0)
    sw a4,  kernel_ctx_t_regs+cpu_regs_t_a4(t0)
    sw a5,  kernel_ctx_t_regs+cpu_regs_t_a5(t0)
    sw a6,  kernel_ctx_t_regs+cpu_regs_t_a6(t0)
    sw a7,  kernel_ctx_t_regs+cpu_regs_t_a7(t0)
    sw s2,  kernel_ctx_t_regs+cpu_regs_t_s2(t0)
    sw s3,  kernel_ctx_t_regs+cpu_regs_t_s3(t0)
    sw s4,  kernel_ctx_t_regs+cpu_regs_t_s4(t0)
    sw s5,  kernel_ctx_t_regs+cpu_regs_t_s5(t0)
    sw s6,  kernel_ctx_t_regs+cpu_regs_t_s6(t0)
    sw s7,  kernel_ctx_t_regs+cpu_regs_t_s7(t0)
    sw s8,  kernel_ctx_t_regs+cpu_regs_t_s8(t0)
    sw s9,  kernel_ctx_t_regs+cpu_regs_t_s9(t0)
    sw s10, kernel_ctx_t_regs+cpu_regs_t_s10(t0)
    sw s11, kernel_ctx_t_regs+cpu_regs_t_s11(t0)
    sw t4,  kernel_ctx_t_regs+cpu_regs_t_t4(t0)
    sw t5,  kernel_ctx_t_regs+cpu_regs_t_t5(t0)
    sw t6,  kernel_ctx_t_regs+cpu_regs_t_t6(t0)
#pragma endregion
    .endm



    # Restore all regs not restored by `isr_exit`.
    # Assumes valid `kernel_ctx_t *` in t0.
    .macro restore_regs
#pragma region
    lw s0,  kernel_ctx_t_regs+cpu_regs_t_s0(t0)
    lw s1,  kernel_ctx_t_regs+cpu_regs_t_s1(t0)
    lw a0,  kernel_ctx_t_regs+cpu_regs_t_a0(t0)
    lw a1,  kernel_ctx_t_regs+cpu_regs_t_a1(t0)
    lw a2,  kernel_ctx_t_regs+cpu_regs_t_a2(t0)
    lw a3,  kernel_ctx_t_regs+cpu_regs_t_a3(t0)
    lw a4,  kernel_ctx_t_regs+cpu_regs_t_a4(t0)
    lw a5,  kernel_ctx_t_regs+cpu_regs_t_a5(t0)
    lw a6,  kernel_ctx_t_regs+cpu_regs_t_a6(t0)
    lw a7,  kernel_ctx_t_regs+cpu_regs_t_a7(t0)
    lw s2,  kernel_ctx_t_regs+cpu_regs_t_s2(t0)
    lw s3,  kernel_ctx_t_regs+cpu_regs_t_s3(t0)
    lw s4,  kernel_ctx_t_regs+cpu_regs_t_s4(t0)
    lw s5,  kernel_ctx_t_regs+cpu_regs_t_s5(t0)
    lw s6,  kernel_ctx_t_regs+cpu_regs_t_s6(t0)
    lw s7,  kernel_ctx_t_regs+cpu_regs_t_s7(t0)
    lw s8,  kernel_ctx_t_regs+cpu_regs_t_s8(t0)
    lw s9,  kernel_ctx_t_regs+cpu_regs_t_s9(t0)
    lw s10, kernel_ctx_t_regs+cpu_regs_t_s10(t0)
    lw s11, kernel_ctx_t_regs+cpu_regs_t_s11(t0)
    lw t4,  kernel_ctx_t_regs+cpu_regs_t_t4(t0)
    lw t5,  kernel_ctx_t_regs+cpu_regs_t_t5(t0)
    lw t6,  kernel_ctx_t_regs+cpu_regs_t_t6(t0)
#pragma endregion
    .endm





    # Trap and system call handler.
    .text
    .type __trap_asm, %function
    .align 2
__trap_asm:
    isr_entry
    # Separate system calls from other traps.
    csrr t1, mcause
    li   t3, 8
    bltu t1, t3, .__trap_asm_save_regs
    li   t3, 11
    bleu t1, t3, .__trap_asm_syscall
    
.__trap_asm_save_regs:
    # This is a non-syscall trap; saving all registers is mandatory.
    save_regs
    # Most of the trap handler is implemented in C.
    jal __trap_handler
    csrr t0, mscratch
    
    # Check for outstanding context switch.
    # If nonnull, context will be switched.
    lw   t1, kernel_ctx_t_ctxswitch(t0)
    beq  t1, x0, .__trap_asm_restore_regs
    # All that is necessary is swapping out context handle.
    mv   t0, t1
    
.__trap_asm_restore_regs:
    # This is a non-syscall trap; restoring all registers is mandatory.
    restore_regs
    j .__trap_asm_exit
    
.__trap_asm_syscall:
    # This is a system call; only callee-saved registers are saved.
    # Increment the program counter past the initiating `ecall` instruction.
    lw   t1, kernel_ctx_t_regs+cpu_regs_t_pc(t0)
    addi t1, t1, 4
    sw   t1, kernel_ctx_t_regs+cpu_regs_t_pc(t0)
    # Most of the syscall handler is implemented in C.
    jal __syscall_handler
    csrr t0, mscratch
    
    # Check for outstanding context switch.
    # If nonnull, context will be switched.
    lw   t1, kernel_ctx_t_ctxswitch(t0)
    beq  t1, x0, .__trap_asm_exit
    # General registers were not saved and must be done explicitly.
    save_regs
    mv   t0, t1
    restore_regs
    
.__trap_asm_exit:
    isr_exit
    mret





    # Interrupt handler.
    .text
    .type __isr_asm, %function
    .align 2
__isr_asm:
    # This is an interrupt; saving all registers is mandatory.
    isr_entry
    save_regs
    # Most of the interrupt handler is implemented in C.
    jal __interrupt_handler
    csrr t0, mscratch
    
    # Check for outstanding context switch.
    # If nonnull, context will be switched.
    lw   t1, kernel_ctx_t_ctxswitch(t0)
    beq  t1, x0, .__isr_asm_exit
    # All that is necessary is swapping out t0.
    mv   t0, t1
    
.__isr_asm_exit:
    # This is an interrupt; restoring all registers is mandatory.
    restore_regs
    isr_exit
    mret





    # Interrupt and trap handler stack.
    .section ".bss"
    .align 4
__interrupt_stack_lo:
    .skip ISR_STACK_DEPTH*4
__interrupt_stack_hi:





    # Interrupt vector table for the CPU.
    # This must be aligned to a 256-byte boundary, so it is in a special section.
    .section ".interrupt_vector_table"
__interrupt_vector_table:
    .option push
    .option norvc
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
    .option pop
