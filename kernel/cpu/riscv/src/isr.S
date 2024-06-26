
# SPDX-License-Identifier: MIT

#include "cpu/isr.h"
#include "cpu/riscv.h"
#include "port/hardware.h"
    .global __global_pointer$
    .global memprotect_swap_from_isr





    # Entry code for an ISR or trap handler; save T0-T3 and swap out SP/GP/TP.
    # Assumes valid `isr_ctx_t *` in CSR mscratch.
    .macro isr_entry
#pragma region
    # Save tempregs t0/t1.
    csrrw t0, mscratch, t0
    .cfi_def_cfa t0, 0
    sw t1, isr_ctx_t_scratch1(t0)
    .cfi_val_offset t0, isr_ctx_t_scratch1
    csrrw t1, mscratch, t0
    .cfi_def_cfa mscratch, 0
    sw t1, isr_ctx_t_scratch0(t0)
    .cfi_val_offset t1, isr_ctx_t_scratch0
    
    sw t2, isr_ctx_t_regs+cpu_regs_t_t2(t0)
    .cfi_val_offset t2, isr_ctx_t_regs+cpu_regs_t_t2
    sw t3, isr_ctx_t_regs+cpu_regs_t_t3(t0)
    .cfi_val_offset t3, isr_ctx_t_regs+cpu_regs_t_t3
    
    sw ra, isr_ctx_t_regs+cpu_regs_t_ra(t0)
    sw sp, isr_ctx_t_regs+cpu_regs_t_sp(t0)
    .cfi_val_offset sp, isr_ctx_t_regs+cpu_regs_t_sp
    sw gp, isr_ctx_t_regs+cpu_regs_t_gp(t0)
    .cfi_val_offset gp, isr_ctx_t_regs+cpu_regs_t_gp
    sw tp, isr_ctx_t_regs+cpu_regs_t_tp(t0)
    .cfi_val_offset tp, isr_ctx_t_regs+cpu_regs_t_tp
    
    # Clear context to switch field.
    sw x0, isr_ctx_t_ctxswitch(t0)
    
    # Move tempregs.
    lw t1, isr_ctx_t_scratch0(t0)
    sw t1, isr_ctx_t_regs+cpu_regs_t_t0(t0)
    .cfi_val_offset t0, isr_ctx_t_regs+cpu_regs_t_t0
    lw t1, isr_ctx_t_scratch1(t0)
    sw t1, isr_ctx_t_regs+cpu_regs_t_t1(t0)
    .cfi_val_offset t1, isr_ctx_t_regs+cpu_regs_t_t1
    
    # Save PC.
    csrr t1, mepc
    sw t1, isr_ctx_t_regs+cpu_regs_t_pc(t0)
    .cfi_val_offset ra, isr_ctx_t_regs+cpu_regs_t_pc
    
    # Set up special regs.
    li tp, 0
    .option push
    .option norelax
    la gp, __global_pointer$
    .option pop
    lw t1, isr_ctx_t_use_sp(t0)
    bnez t1, .isr_entry_\@
    la sp, __interrupt_stack_hi
.isr_entry_\@:
#pragma endregion
    .endm



    # Exit code for an ISR or trap handler; restores tempregs and SP/GP/TP.
    # Assumes valid `isr_ctx_t *` in t0.
    .macro isr_exit
#pragma region
    # Restore PC.
    lw t1, isr_ctx_t_regs+cpu_regs_t_pc(t0)
    csrw mepc, t1
    
    lw ra, isr_ctx_t_regs+cpu_regs_t_ra(t0)
    lw sp, isr_ctx_t_regs+cpu_regs_t_sp(t0)
    lw gp, isr_ctx_t_regs+cpu_regs_t_gp(t0)
    lw tp, isr_ctx_t_regs+cpu_regs_t_tp(t0)
    
    # Restore tempregs t0-t3.
    lw t1, isr_ctx_t_regs+cpu_regs_t_t0(t0)
    csrw mscratch, t1
    lw t3, isr_ctx_t_regs+cpu_regs_t_t3(t0)
    lw t2, isr_ctx_t_regs+cpu_regs_t_t2(t0)
    lw t1, isr_ctx_t_regs+cpu_regs_t_t1(t0)
    csrrw t0, mscratch, t0
#pragma endregion
    .endm



    # Save all regs not saved by `isr_entry`.
    # Assumes valid `isr_ctx_t *` in t0.
    .macro save_all_regs
#pragma region
    sw s0,  isr_ctx_t_regs+cpu_regs_t_s0(t0)
    .cfi_offset s0, isr_ctx_t_regs+cpu_regs_t_s0
    sw s1,  isr_ctx_t_regs+cpu_regs_t_s1(t0)
    .cfi_offset s1, isr_ctx_t_regs+cpu_regs_t_s1
    sw a0,  isr_ctx_t_regs+cpu_regs_t_a0(t0)
    .cfi_offset a0, isr_ctx_t_regs+cpu_regs_t_a0
    sw a1,  isr_ctx_t_regs+cpu_regs_t_a1(t0)
    .cfi_offset a1, isr_ctx_t_regs+cpu_regs_t_a1
    sw a2,  isr_ctx_t_regs+cpu_regs_t_a2(t0)
    .cfi_offset a2, isr_ctx_t_regs+cpu_regs_t_a2
    sw a3,  isr_ctx_t_regs+cpu_regs_t_a3(t0)
    .cfi_offset a3, isr_ctx_t_regs+cpu_regs_t_a3
    sw a4,  isr_ctx_t_regs+cpu_regs_t_a4(t0)
    .cfi_offset a4, isr_ctx_t_regs+cpu_regs_t_a4
    sw a5,  isr_ctx_t_regs+cpu_regs_t_a5(t0)
    .cfi_offset a5, isr_ctx_t_regs+cpu_regs_t_a5
    sw a6,  isr_ctx_t_regs+cpu_regs_t_a6(t0)
    .cfi_offset a6, isr_ctx_t_regs+cpu_regs_t_a6
    sw a7,  isr_ctx_t_regs+cpu_regs_t_a7(t0)
    .cfi_offset a7, isr_ctx_t_regs+cpu_regs_t_a7
    sw s2,  isr_ctx_t_regs+cpu_regs_t_s2(t0)
    .cfi_offset s2, isr_ctx_t_regs+cpu_regs_t_s2
    sw s3,  isr_ctx_t_regs+cpu_regs_t_s3(t0)
    .cfi_offset s3, isr_ctx_t_regs+cpu_regs_t_s3
    sw s4,  isr_ctx_t_regs+cpu_regs_t_s4(t0)
    .cfi_offset s4, isr_ctx_t_regs+cpu_regs_t_s4
    sw s5,  isr_ctx_t_regs+cpu_regs_t_s5(t0)
    .cfi_offset s5, isr_ctx_t_regs+cpu_regs_t_s5
    sw s6,  isr_ctx_t_regs+cpu_regs_t_s6(t0)
    .cfi_offset s6, isr_ctx_t_regs+cpu_regs_t_s6
    sw s7,  isr_ctx_t_regs+cpu_regs_t_s7(t0)
    .cfi_offset s7, isr_ctx_t_regs+cpu_regs_t_s7
    sw s8,  isr_ctx_t_regs+cpu_regs_t_s8(t0)
    .cfi_offset s8, isr_ctx_t_regs+cpu_regs_t_s8
    sw s9,  isr_ctx_t_regs+cpu_regs_t_s9(t0)
    .cfi_offset s9, isr_ctx_t_regs+cpu_regs_t_s9
    sw s10, isr_ctx_t_regs+cpu_regs_t_s10(t0)
    .cfi_offset s10, isr_ctx_t_regs+cpu_regs_t_s10
    sw s11, isr_ctx_t_regs+cpu_regs_t_s11(t0)
    .cfi_offset s11, isr_ctx_t_regs+cpu_regs_t_s11
    sw t4,  isr_ctx_t_regs+cpu_regs_t_t4(t0)
    .cfi_offset t4, isr_ctx_t_regs+cpu_regs_t_t4
    sw t5,  isr_ctx_t_regs+cpu_regs_t_t5(t0)
    .cfi_offset t5, isr_ctx_t_regs+cpu_regs_t_t5
    sw t6,  isr_ctx_t_regs+cpu_regs_t_t6(t0)
    .cfi_offset t6, isr_ctx_t_regs+cpu_regs_t_t6
#pragma endregion
    .endm



    # Restore all regs not restored by `isr_exit`.
    # Assumes valid `isr_ctx_t *` in t0.
    .macro restore_all_regs
#pragma region
    lw s0,  isr_ctx_t_regs+cpu_regs_t_s0(t0)
    lw s1,  isr_ctx_t_regs+cpu_regs_t_s1(t0)
    lw a0,  isr_ctx_t_regs+cpu_regs_t_a0(t0)
    lw a1,  isr_ctx_t_regs+cpu_regs_t_a1(t0)
    lw a2,  isr_ctx_t_regs+cpu_regs_t_a2(t0)
    lw a3,  isr_ctx_t_regs+cpu_regs_t_a3(t0)
    lw a4,  isr_ctx_t_regs+cpu_regs_t_a4(t0)
    lw a5,  isr_ctx_t_regs+cpu_regs_t_a5(t0)
    lw a6,  isr_ctx_t_regs+cpu_regs_t_a6(t0)
    lw a7,  isr_ctx_t_regs+cpu_regs_t_a7(t0)
    lw s2,  isr_ctx_t_regs+cpu_regs_t_s2(t0)
    lw s3,  isr_ctx_t_regs+cpu_regs_t_s3(t0)
    lw s4,  isr_ctx_t_regs+cpu_regs_t_s4(t0)
    lw s5,  isr_ctx_t_regs+cpu_regs_t_s5(t0)
    lw s6,  isr_ctx_t_regs+cpu_regs_t_s6(t0)
    lw s7,  isr_ctx_t_regs+cpu_regs_t_s7(t0)
    lw s8,  isr_ctx_t_regs+cpu_regs_t_s8(t0)
    lw s9,  isr_ctx_t_regs+cpu_regs_t_s9(t0)
    lw s10, isr_ctx_t_regs+cpu_regs_t_s10(t0)
    lw s11, isr_ctx_t_regs+cpu_regs_t_s11(t0)
    lw t4,  isr_ctx_t_regs+cpu_regs_t_t4(t0)
    lw t5,  isr_ctx_t_regs+cpu_regs_t_t5(t0)
    lw t6,  isr_ctx_t_regs+cpu_regs_t_t6(t0)
#pragma endregion
    .endm





    # Explicit context switch from M-mode.
    # Interrupts must be disabled on entry and will be re-enabled on exit.
    # If the context switch target is not set, this is a NOP.
    .text
    .global isr_context_switch
    .type isr_context_switch, %function
    .align 2
    .cfi_startproc
isr_context_switch:
    # Check for context switch required.
    .cfi_def_cfa mscratch, 0
    csrr t0, mscratch
    lw t1, isr_ctx_t_ctxswitch(t0)
    bnez t1, .isr_context_switch_do_switch
    
    # Re-enable interrupts.
    li t1, 1 << RISCV_STATUS_MIE_BIT
    csrs mstatus, t1
    ret
    
.isr_context_switch_do_switch:
    # Re-enable interrupts on exit.
    li t1, (1 << RISCV_STATUS_MPIE_BIT)
    csrs mstatus, t1
    
    # Do the context switching things.
    # Save SP/GP/TP and PC.
    sw ra, isr_ctx_t_regs+cpu_regs_t_pc(t0)
    sw ra, isr_ctx_t_regs+cpu_regs_t_ra(t0)
    sw sp, isr_ctx_t_regs+cpu_regs_t_sp(t0)
    sw gp, isr_ctx_t_regs+cpu_regs_t_gp(t0)
    sw tp, isr_ctx_t_regs+cpu_regs_t_tp(t0)
    save_all_regs
    
    # Switch context and set new privilege.
    lw t0, isr_ctx_t_ctxswitch(t0)
    csrw mscratch, t0
    
    # Swap memory protection.
    jal  memprotect_swap_from_isr
    csrr t0, mscratch
    lw t1, isr_ctx_t_is_kernel_thread(t0)
    li t2, (3 << RISCV_STATUS_MPP_BASE_BIT)
    bnez t1, .isr_context_switch_do_m
    
    # To U-mode.
    csrc mstatus, t2
    j .isr_context_switch_ret
    
.isr_context_switch_do_m:
    # To M-mode.
    csrs mstatus, t2
    
.isr_context_switch_ret:
    # Return to new context.
    restore_all_regs
    isr_exit
#pragma endregion
    mret
    .cfi_endproc





    # Trap and system call handler.
    .text
    .type __trap_asm, %function
    .align 2
    .cfi_startproc simple
__trap_asm:
    isr_entry
    save_all_regs
    
    # Construct fake stack frame.
    csrr ra, mepc
    addi sp, sp, -16
    sw   ra, 12(sp)
    sw   s0, 8(sp)
    addi s0, sp, 16
    sw   s0, isr_ctx_t_frameptr(t0)
    .cfi_def_cfa s0, 0
    .cfi_val_offset s0, -8
    .cfi_val_offset ra, -4
    
    # Most of the trap handler is implemented in C.
    jal  riscv_trap_handler
    csrr t0, mscratch
    
    # Discard fake stack frame.
    addi sp, sp, 16
    
    .cfi_def_cfa mstatus, 0
    .cfi_val_offset s0, isr_ctx_t_regs+cpu_regs_t_s0
    .cfi_val_offset ra, isr_ctx_t_regs+cpu_regs_t_ra
    
    # Check for outstanding context switch.
    # If nonnull, context will be switched.
    lw   t1, isr_ctx_t_ctxswitch(t0)
    beq  t1, x0, .__trap_asm_exit
    # Swap out the context pointer.
    csrr t0, mscratch
    lw   t0, isr_ctx_t_ctxswitch(t0)
    csrw mscratch, t0
    # Swap memory protection.
    jal  memprotect_swap_from_isr
    csrr t0, mscratch
    # Set privilege level.
    lw   t2, isr_ctx_t_is_kernel_thread(t0)
    li   t3, 3 << RISCV_STATUS_MPP_BASE_BIT
    bnez t2, .__trap_asm_priv_m
    # Zero; user thread; clear MPP.
    csrc mstatus, t3
    j .__trap_asm_exit
.__trap_asm_priv_m:
    # Nonzero; kernel thread; set MPP.
    csrs mstatus, t3
    
.__trap_asm_exit:
    restore_all_regs
    isr_exit
#pragma endregion
    mret
    .cfi_endproc





    # Interrupt handler.
    .text
    .type __isr_asm, %function
    .align 2
    .cfi_startproc simple
__isr_asm:
    # This is an interrupt; saving all registers is mandatory.
    isr_entry
    save_all_regs
    
    # Construct fake stack frame.
    csrr ra, mepc
    addi sp, sp, -16
    sw   ra, 12(sp)
    sw   s0, 8(sp)
    addi s0, sp, 16
    sw   s0, isr_ctx_t_frameptr(t0)
    .cfi_def_cfa s0, 0
    .cfi_val_offset s0, -8
    .cfi_val_offset ra, -4
    
    # Most of the interrupt handler is implemented in C.
    jal riscv_interrupt_handler
    csrr t0, mscratch
    
    # Discard fake stack frame.
    addi sp, sp, 16
    
    # Check for outstanding context switch.
    # If nonnull, context will be switched.
    lw   t1, isr_ctx_t_ctxswitch(t0)
    beq  t1, x0, .__isr_asm_exit
    # Swap out the context pointer.
    csrr t0, mscratch
    lw   t0, isr_ctx_t_ctxswitch(t0)
    csrw mscratch, t0
    # Swap memory protection.
    jal  memprotect_swap_from_isr
    csrr t0, mscratch
    # Set privilege level.
    lw   t2, isr_ctx_t_is_kernel_thread(t0)
    li   t3, 3 << RISCV_STATUS_MPP_BASE_BIT
    bnez t2, .__isr_asm_priv_m
    # Zero; user thread; clear MPP.
    csrc mstatus, t3
    j .__isr_asm_exit
.__isr_asm_priv_m:
    # Nonzero; kernel thread; set MPP.
    csrs mstatus, t3
    
.__isr_asm_exit:
    # This is an interrupt; restoring all registers is mandatory.
    restore_all_regs
    isr_exit
    mret
    .cfi_endproc





    # Interrupt and trap handler stack.
    .section ".bss"
    .align 4
__interrupt_stack_lo:
    .skip ISR_STACK_DEPTH*4
__interrupt_stack_hi:





    # Interrupt vector table for the CPU.
    # This must be aligned to a 256-byte boundary, so it is in a special section.
    .section ".interrupt_vector_table"
riscv_interrupt_vector_table:
    .option push
    .option norvc
    # Trap handler.
    j __trap_asm
    # Interrupt handlers.
    .rept RISCV_VT_INT_COUNT
    j __isr_asm
    .endr
    # Padding.
    .skip RISCV_VT_PADDING*4
    .option pop
#pragma endregion
