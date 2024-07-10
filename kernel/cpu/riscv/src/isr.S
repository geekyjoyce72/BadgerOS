
# SPDX-License-Identifier: MIT

#include "cpu/isr.h"
#include "cpu/regs.h"
#include "port/hardware.h"
    .global __global_pointer$
    .global memprotect_swap_from_isr


#if __riscv_xlen == 64
#define ST_REG sd
#define LD_REG ld
#else
#define ST_REG sw
#define LD_REG lw
#endif



    # Entry code for an ISR or trap handler; save T0-T3 and swap out SP/GP/TP.
    # Assumes valid `isr_ctx_t *` in CSR scratch.
    .macro isr_entry
#pragma region
    # Save tempregs t0/t1.
    csrrw   t0, CSR_SCRATCH, t0
    .cfi_def_cfa t0, 0
    ST_REG  t1, isr_ctx_t_scratch1(t0)
    .cfi_val_offset t0, isr_ctx_t_scratch1
    csrrw   t1, CSR_SCRATCH, t0
    .cfi_def_cfa CSR_SCRATCH, 0
    ST_REG  t1, isr_ctx_t_scratch0(t0)
    .cfi_val_offset t1, isr_ctx_t_scratch0
    
    ST_REG  t2, isr_ctx_t_regs+cpu_regs_t_t2(t0)
    .cfi_val_offset t2, isr_ctx_t_regs+cpu_regs_t_t2
    ST_REG  t3, isr_ctx_t_regs+cpu_regs_t_t3(t0)
    .cfi_val_offset t3, isr_ctx_t_regs+cpu_regs_t_t3
    
    ST_REG  ra, isr_ctx_t_regs+cpu_regs_t_ra(t0)
    ST_REG  sp, isr_ctx_t_regs+cpu_regs_t_sp(t0)
    .cfi_val_offset sp, isr_ctx_t_regs+cpu_regs_t_sp
    ST_REG  gp, isr_ctx_t_regs+cpu_regs_t_gp(t0)
    .cfi_val_offset gp, isr_ctx_t_regs+cpu_regs_t_gp
    ST_REG  tp, isr_ctx_t_regs+cpu_regs_t_tp(t0)
    .cfi_val_offset tp, isr_ctx_t_regs+cpu_regs_t_tp
    
    # Clear context to switch field.
    ST_REG  x0, isr_ctx_t_ctxswitch(t0)
    
    # Move tempregs.
    LD_REG  t1, isr_ctx_t_scratch0(t0)
    ST_REG  t1, isr_ctx_t_regs+cpu_regs_t_t0(t0)
    .cfi_val_offset t0, isr_ctx_t_regs+cpu_regs_t_t0
    LD_REG  t1, isr_ctx_t_scratch1(t0)
    ST_REG  t1, isr_ctx_t_regs+cpu_regs_t_t1(t0)
    .cfi_val_offset t1, isr_ctx_t_regs+cpu_regs_t_t1
    
    # Save PC.
    csrr    t1, CSR_EPC
    ST_REG  t1, isr_ctx_t_regs+cpu_regs_t_pc(t0)
    .cfi_val_offset ra, isr_ctx_t_regs+cpu_regs_t_pc
    
    # Set up special regs.
    li      tp, 0
    .option push
    .option norelax
    la      gp, __global_pointer$
    .option pop
    LD_REG  t1, isr_ctx_t_flags(t0)
    andi    t1, t1, ISR_CTX_FLAG_USE_SP
    bnez    t1, .isr_entry_\@
    la      sp, __interrupt_stack_hi
.isr_entry_\@:
#pragma endregion
    .endm



    # Exit code for an ISR or trap handler; restores tempregs and SP/GP/TP.
    # Assumes valid `isr_ctx_t *` in t0.
    .macro isr_exit
#pragma region
    # Restore PC.
    LD_REG  t1, isr_ctx_t_regs+cpu_regs_t_pc(t0)
    csrw    CSR_EPC, t1
    
    LD_REG  ra, isr_ctx_t_regs+cpu_regs_t_ra(t0)
    LD_REG  sp, isr_ctx_t_regs+cpu_regs_t_sp(t0)
    LD_REG  gp, isr_ctx_t_regs+cpu_regs_t_gp(t0)
    LD_REG  tp, isr_ctx_t_regs+cpu_regs_t_tp(t0)
    
    # Restore tempregs t0-t3.
    LD_REG  t1, isr_ctx_t_regs+cpu_regs_t_t0(t0)
    csrw    CSR_SCRATCH, t1
    LD_REG  t3, isr_ctx_t_regs+cpu_regs_t_t3(t0)
    LD_REG  t2, isr_ctx_t_regs+cpu_regs_t_t2(t0)
    LD_REG  t1, isr_ctx_t_regs+cpu_regs_t_t1(t0)
    csrrw   t0, CSR_SCRATCH, t0
#pragma endregion
    .endm



    # Save all regs not saved by `isr_entry`.
    # Assumes valid `isr_ctx_t *` in t0.
    .macro save_all_regs
#pragma region
    ST_REG  s0,  isr_ctx_t_regs+cpu_regs_t_s0(t0)
    .cfi_offset s0, isr_ctx_t_regs+cpu_regs_t_s0
    ST_REG  s1,  isr_ctx_t_regs+cpu_regs_t_s1(t0)
    .cfi_offset s1, isr_ctx_t_regs+cpu_regs_t_s1
    ST_REG  a0,  isr_ctx_t_regs+cpu_regs_t_a0(t0)
    .cfi_offset a0, isr_ctx_t_regs+cpu_regs_t_a0
    ST_REG  a1,  isr_ctx_t_regs+cpu_regs_t_a1(t0)
    .cfi_offset a1, isr_ctx_t_regs+cpu_regs_t_a1
    ST_REG  a2,  isr_ctx_t_regs+cpu_regs_t_a2(t0)
    .cfi_offset a2, isr_ctx_t_regs+cpu_regs_t_a2
    ST_REG  a3,  isr_ctx_t_regs+cpu_regs_t_a3(t0)
    .cfi_offset a3, isr_ctx_t_regs+cpu_regs_t_a3
    ST_REG  a4,  isr_ctx_t_regs+cpu_regs_t_a4(t0)
    .cfi_offset a4, isr_ctx_t_regs+cpu_regs_t_a4
    ST_REG  a5,  isr_ctx_t_regs+cpu_regs_t_a5(t0)
    .cfi_offset a5, isr_ctx_t_regs+cpu_regs_t_a5
    ST_REG  a6,  isr_ctx_t_regs+cpu_regs_t_a6(t0)
    .cfi_offset a6, isr_ctx_t_regs+cpu_regs_t_a6
    ST_REG  a7,  isr_ctx_t_regs+cpu_regs_t_a7(t0)
    .cfi_offset a7, isr_ctx_t_regs+cpu_regs_t_a7
    ST_REG  s2,  isr_ctx_t_regs+cpu_regs_t_s2(t0)
    .cfi_offset s2, isr_ctx_t_regs+cpu_regs_t_s2
    ST_REG  s3,  isr_ctx_t_regs+cpu_regs_t_s3(t0)
    .cfi_offset s3, isr_ctx_t_regs+cpu_regs_t_s3
    ST_REG  s4,  isr_ctx_t_regs+cpu_regs_t_s4(t0)
    .cfi_offset s4, isr_ctx_t_regs+cpu_regs_t_s4
    ST_REG  s5,  isr_ctx_t_regs+cpu_regs_t_s5(t0)
    .cfi_offset s5, isr_ctx_t_regs+cpu_regs_t_s5
    ST_REG  s6,  isr_ctx_t_regs+cpu_regs_t_s6(t0)
    .cfi_offset s6, isr_ctx_t_regs+cpu_regs_t_s6
    ST_REG  s7,  isr_ctx_t_regs+cpu_regs_t_s7(t0)
    .cfi_offset s7, isr_ctx_t_regs+cpu_regs_t_s7
    ST_REG  s8,  isr_ctx_t_regs+cpu_regs_t_s8(t0)
    .cfi_offset s8, isr_ctx_t_regs+cpu_regs_t_s8
    ST_REG  s9,  isr_ctx_t_regs+cpu_regs_t_s9(t0)
    .cfi_offset s9, isr_ctx_t_regs+cpu_regs_t_s9
    ST_REG  s10, isr_ctx_t_regs+cpu_regs_t_s10(t0)
    .cfi_offset s10, isr_ctx_t_regs+cpu_regs_t_s10
    ST_REG  s11, isr_ctx_t_regs+cpu_regs_t_s11(t0)
    .cfi_offset s11, isr_ctx_t_regs+cpu_regs_t_s11
    ST_REG  t4,  isr_ctx_t_regs+cpu_regs_t_t4(t0)
    .cfi_offset t4, isr_ctx_t_regs+cpu_regs_t_t4
    ST_REG  t5,  isr_ctx_t_regs+cpu_regs_t_t5(t0)
    .cfi_offset t5, isr_ctx_t_regs+cpu_regs_t_t5
    ST_REG  t6,  isr_ctx_t_regs+cpu_regs_t_t6(t0)
    .cfi_offset t6, isr_ctx_t_regs+cpu_regs_t_t6
#pragma endregion
    .endm



    # Restore all regs not restored by `isr_exit`.
    # Assumes valid `isr_ctx_t *` in t0.
    .macro restore_all_regs
#pragma region
    LD_REG  s0,  isr_ctx_t_regs+cpu_regs_t_s0(t0)
    LD_REG  s1,  isr_ctx_t_regs+cpu_regs_t_s1(t0)
    LD_REG  a0,  isr_ctx_t_regs+cpu_regs_t_a0(t0)
    LD_REG  a1,  isr_ctx_t_regs+cpu_regs_t_a1(t0)
    LD_REG  a2,  isr_ctx_t_regs+cpu_regs_t_a2(t0)
    LD_REG  a3,  isr_ctx_t_regs+cpu_regs_t_a3(t0)
    LD_REG  a4,  isr_ctx_t_regs+cpu_regs_t_a4(t0)
    LD_REG  a5,  isr_ctx_t_regs+cpu_regs_t_a5(t0)
    LD_REG  a6,  isr_ctx_t_regs+cpu_regs_t_a6(t0)
    LD_REG  a7,  isr_ctx_t_regs+cpu_regs_t_a7(t0)
    LD_REG  s2,  isr_ctx_t_regs+cpu_regs_t_s2(t0)
    LD_REG  s3,  isr_ctx_t_regs+cpu_regs_t_s3(t0)
    LD_REG  s4,  isr_ctx_t_regs+cpu_regs_t_s4(t0)
    LD_REG  s5,  isr_ctx_t_regs+cpu_regs_t_s5(t0)
    LD_REG  s6,  isr_ctx_t_regs+cpu_regs_t_s6(t0)
    LD_REG  s7,  isr_ctx_t_regs+cpu_regs_t_s7(t0)
    LD_REG  s8,  isr_ctx_t_regs+cpu_regs_t_s8(t0)
    LD_REG  s9,  isr_ctx_t_regs+cpu_regs_t_s9(t0)
    LD_REG  s10, isr_ctx_t_regs+cpu_regs_t_s10(t0)
    LD_REG  s11, isr_ctx_t_regs+cpu_regs_t_s11(t0)
    LD_REG  t4,  isr_ctx_t_regs+cpu_regs_t_t4(t0)
    LD_REG  t5,  isr_ctx_t_regs+cpu_regs_t_t5(t0)
    LD_REG  t6,  isr_ctx_t_regs+cpu_regs_t_t6(t0)
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
    .cfi_def_cfa CSR_SCRATCH, 0
    csrr    t0, CSR_SCRATCH
    LD_REG  t1, isr_ctx_t_ctxswitch(t0)
    bnez    t1, .isr_context_switch_do_switch
    
    # Re-enable interrupts.
    li      t1, 1 << CSR_STATUS_IE_BIT
    csrs    CSR_STATUS, t1
    ret
    
.isr_context_switch_do_switch:
    # Re-enable interrupts on exit.
    li      t1, 1 << CSR_STATUS_PIE_BIT
    csrs    CSR_STATUS, t1
    
    # Do the context switching things.
    # Save SP/GP/TP and PC.
    ST_REG  ra, isr_ctx_t_regs+cpu_regs_t_pc(t0)
    ST_REG  ra, isr_ctx_t_regs+cpu_regs_t_ra(t0)
    ST_REG  sp, isr_ctx_t_regs+cpu_regs_t_sp(t0)
    ST_REG  gp, isr_ctx_t_regs+cpu_regs_t_gp(t0)
    ST_REG  tp, isr_ctx_t_regs+cpu_regs_t_tp(t0)
    save_all_regs
    
    # Switch context and set new privilege.
    LD_REG  t0, isr_ctx_t_ctxswitch(t0)
    csrw    CSR_SCRATCH, t0
    
    # Swap memory protection.
    jal     memprotect_swap_from_isr
    csrr    t0, CSR_SCRATCH
    LD_REG  t1, isr_ctx_t_flags(t0)
    andi    t1, t1, ISR_CTX_FLAG_KERNEL
    li      t2, CSR_STATUS_PP_MASK << CSR_STATUS_PP_BASE_BIT
    bnez    t1, .isr_context_switch_do_k
    
    # To user mode.
    csrc    CSR_STATUS, t2
    j       .isr_context_switch_ret
    
.isr_context_switch_do_k:
    # To kernel mode.
    csrs    CSR_STATUS, t2
    
.isr_context_switch_ret:
    # Return to new context.
    restore_all_regs
    isr_exit
#pragma endregion
    RISCV_TRAP_RET
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
    csrr    ra, CSR_EPC
    addi    sp, sp, -16
#if __riscv_xlen == 64
    sd      ra, 8(sp)
    sd      s0, 0(sp)
#else
    sw      ra, 12(sp)
    sw      s0, 8(sp)
#endif
    addi    s0, sp, 16
    ST_REG  s0, isr_ctx_t_frameptr(t0)
    .cfi_def_cfa s0, 0
    .cfi_val_offset s0, -8
    .cfi_val_offset ra, -4
    
    # Most of the trap handler is implemented in C.
    jal     riscv_trap_handler
    csrr    t0, CSR_SCRATCH
    
    # Discard fake stack frame.
    addi    sp, sp, 16
    
    .cfi_def_cfa CSR_STATUS, 0
    .cfi_val_offset s0, isr_ctx_t_regs+cpu_regs_t_s0
    .cfi_val_offset ra, isr_ctx_t_regs+cpu_regs_t_ra
    
    # Check for outstanding context switch.
    # If nonnull, context will be switched.
    LD_REG  t1, isr_ctx_t_ctxswitch(t0)
    beq     t1, x0, .__trap_asm_exit
    # Swap out the context pointer.
    csrr    t0, CSR_SCRATCH
    LD_REG  t0, isr_ctx_t_ctxswitch(t0)
    csrw    CSR_SCRATCH, t0
    # Swap memory protection.
    jal     memprotect_swap_from_isr
    csrr    t0, CSR_SCRATCH
    # Set privilege level.
    LD_REG  t2, isr_ctx_t_flags(t0)
    andi    t2, t2, ISR_CTX_FLAG_KERNEL
    li      t3, CSR_STATUS_PP_MASK << CSR_STATUS_PP_BASE_BIT
    bnez    t2, .__trap_asm_priv_k
    # Zero; user thread; clear xPP.
    csrc    CSR_STATUS, t3
    j       .__trap_asm_exit
.__trap_asm_priv_k:
    # Nonzero; kernel thread; set xPP.
    csrs    CSR_STATUS, t3
    
.__trap_asm_exit:
    restore_all_regs
    isr_exit
#pragma endregion
    RISCV_TRAP_RET
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
    csrr    ra, CSR_EPC
    addi    sp, sp, -16
#if __riscv_xlen == 64
    sd      ra, 8(sp)
    sd      s0, 0(sp)
#else
    sw      ra, 12(sp)
    sw      s0, 8(sp)
#endif
    addi    s0, sp, 16
    ST_REG  s0, isr_ctx_t_frameptr(t0)
    .cfi_def_cfa s0, 0
    .cfi_val_offset s0, -8
    .cfi_val_offset ra, -4
    
    # Most of the interrupt handler is implemented in C.
    jal     riscv_interrupt_handler
    csrr    t0, CSR_SCRATCH
    
    # Discard fake stack frame.
    addi    sp, sp, 16
    
    # Check for outstanding context switch.
    # If nonnull, context will be switched.
    LD_REG  t1, isr_ctx_t_ctxswitch(t0)
    beq     t1, x0, .__isr_asm_exit
    # Swap out the context pointer.
    csrr    t0, CSR_SCRATCH
    LD_REG  t0, isr_ctx_t_ctxswitch(t0)
    csrw    CSR_SCRATCH, t0
    # Swap memory protection.
    jal     memprotect_swap_from_isr
    csrr    t0, CSR_SCRATCH
    # Set privilege level.
    LD_REG  t2, isr_ctx_t_flags(t0)
    andi    t2, t2, ISR_CTX_FLAG_KERNEL
    li      t3, CSR_STATUS_PP_MASK << CSR_STATUS_PP_BASE_BIT
    bnez    t2, .__isr_asm_priv_k
    # Zero; user thread; clear xPP.
    csrc    CSR_STATUS, t3
    j       .__isr_asm_exit
.__isr_asm_priv_k:
    # Nonzero; kernel thread; set xPP.
    csrs    CSR_STATUS, t3
    
.__isr_asm_exit:
    # This is an interrupt; restoring all registers is mandatory.
    restore_all_regs
    isr_exit
    RISCV_TRAP_RET
    .cfi_endproc





    # Interrupt and trap handler stack.
    .bss
    .align 4
__interrupt_stack_lo:
    .skip ISR_STACK_DEPTH*4
__interrupt_stack_hi:





    # Interrupt vector table for the CPU.
    # This must be aligned to a 256-byte boundary, so it is in a special section.
    .section ".interrupt_vector_table", "ax", @progbits
    .global riscv_interrupt_vector_table
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
#if RISCV_VT_PADDING
    .skip RISCV_VT_PADDING*4
#endif
    .option pop
#pragma endregion
