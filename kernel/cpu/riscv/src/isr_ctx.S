
# SPDX-License-Identifier: MIT

#include "cpu/regs.h"
    .global kernel_reg_dump_arr



#if __riscv_xlen == 64
#define ST_REG sd
#define LD_REG ld
#else
#define ST_REG sw
#define LD_REG lw
#endif



    # Print a register dump of the current registers.
    .global kernel_cur_regs_dump
    .type kernel_cur_regs_dump, %function
kernel_cur_regs_dump:
    # Save all registers to stack.
    addi sp, sp, -4*__riscv_xlen
    ST_REG ra, cpu_regs_t_pc(sp) # PC inferred.
    ST_REG x0, cpu_regs_t_ra(sp) # RA omitted.
    # ST_REG sp, cpu_regs_t_sp(sp) # SP inferred.
    ST_REG gp, cpu_regs_t_gp(sp)
    ST_REG tp, cpu_regs_t_tp(sp)
    ST_REG t0, cpu_regs_t_t0(sp)
    ST_REG t1, cpu_regs_t_t1(sp)
    ST_REG t2, cpu_regs_t_t2(sp)
    ST_REG s0, cpu_regs_t_s0(sp)
    ST_REG s1, cpu_regs_t_s1(sp)
    ST_REG a0, cpu_regs_t_a0(sp)
    ST_REG a1, cpu_regs_t_a1(sp)
    ST_REG a2, cpu_regs_t_a2(sp)
    ST_REG a3, cpu_regs_t_a3(sp)
    ST_REG a4, cpu_regs_t_a4(sp)
    ST_REG a5, cpu_regs_t_a5(sp)
    ST_REG a6, cpu_regs_t_a6(sp)
    ST_REG a7, cpu_regs_t_a7(sp)
    ST_REG s2, cpu_regs_t_s2(sp)
    ST_REG s3, cpu_regs_t_s3(sp)
    ST_REG s4, cpu_regs_t_s4(sp)
    ST_REG s5, cpu_regs_t_s5(sp)
    ST_REG s6, cpu_regs_t_s6(sp)
    ST_REG s7, cpu_regs_t_s7(sp)
    ST_REG s8, cpu_regs_t_s8(sp)
    ST_REG s9, cpu_regs_t_s9(sp)
    ST_REG s10, cpu_regs_t_s10(sp)
    ST_REG s11, cpu_regs_t_s11(sp)
    ST_REG t3, cpu_regs_t_t3(sp)
    ST_REG t4, cpu_regs_t_t4(sp)
    ST_REG t5, cpu_regs_t_t5(sp)
    ST_REG t6, cpu_regs_t_t6(sp)
    # Store the real value of SP and call the printer.
    addi a0, sp, 4*__riscv_xlen
    ST_REG a0, cpu_regs_t_sp(sp)
    mv a0, sp
    jal kernel_reg_dump_arr
    # Restore all registers.
    LD_REG ra, cpu_regs_t_pc(sp) # PC inferred.
    # LD_REG ra, cpu_regs_t_ra(sp) # RA omitted.
    # LD_REG sp, cpu_regs_t_sp(sp) # SP inferred.
    LD_REG gp, cpu_regs_t_gp(sp)
    LD_REG tp, cpu_regs_t_tp(sp)
    LD_REG t0, cpu_regs_t_t0(sp)
    LD_REG t1, cpu_regs_t_t1(sp)
    LD_REG t2, cpu_regs_t_t2(sp)
    LD_REG s0, cpu_regs_t_s0(sp)
    LD_REG s1, cpu_regs_t_s1(sp)
    LD_REG a0, cpu_regs_t_a0(sp)
    LD_REG a1, cpu_regs_t_a1(sp)
    LD_REG a2, cpu_regs_t_a2(sp)
    LD_REG a3, cpu_regs_t_a3(sp)
    LD_REG a4, cpu_regs_t_a4(sp)
    LD_REG a5, cpu_regs_t_a5(sp)
    LD_REG a6, cpu_regs_t_a6(sp)
    LD_REG a7, cpu_regs_t_a7(sp)
    LD_REG s2, cpu_regs_t_s2(sp)
    LD_REG s3, cpu_regs_t_s3(sp)
    LD_REG s4, cpu_regs_t_s4(sp)
    LD_REG s5, cpu_regs_t_s5(sp)
    LD_REG s6, cpu_regs_t_s6(sp)
    LD_REG s7, cpu_regs_t_s7(sp)
    LD_REG s8, cpu_regs_t_s8(sp)
    LD_REG s9, cpu_regs_t_s9(sp)
    LD_REG s10, cpu_regs_t_s10(sp)
    LD_REG s11, cpu_regs_t_s11(sp)
    LD_REG t3, cpu_regs_t_t3(sp)
    LD_REG t4, cpu_regs_t_t4(sp)
    LD_REG t5, cpu_regs_t_t5(sp)
    LD_REG t6, cpu_regs_t_t6(sp)
    # Return.
    addi sp, sp, 4*__riscv_xlen
#pragma endregion
    ret



    # Calls the actual trapping code in `isr_noexc_run`.
    .global _isr_noexc_run_int
    .type _isr_noexc_run_int, %function
_isr_noexc_run_int:
    ST_REG ra, cpu_regs_t_pc(a1) // PC inferred
    ST_REG ra, cpu_regs_t_ra(a1)
    ST_REG sp, cpu_regs_t_sp(a1)
    ST_REG gp, cpu_regs_t_gp(a1)
    ST_REG tp, cpu_regs_t_tp(a1)
    // ST_REG t0, cpu_regs_t_t0(a1) // Tempregs omitted
    // ST_REG t1, cpu_regs_t_t1(a1)
    // ST_REG t2, cpu_regs_t_t2(a1)
    ST_REG s0, cpu_regs_t_s0(a1)
    ST_REG s1, cpu_regs_t_s1(a1)
    // ST_REG a0, cpu_regs_t_a0(a1) // Argregs omitted
    // ST_REG a1, cpu_regs_t_a1(a1)
    // ST_REG a2, cpu_regs_t_a2(a1)
    // ST_REG a3, cpu_regs_t_a3(a1)
    // ST_REG a4, cpu_regs_t_a4(a1)
    // ST_REG a5, cpu_regs_t_a5(a1)
    // ST_REG a6, cpu_regs_t_a6(a1)
    // ST_REG a7, cpu_regs_t_a7(a1)
    ST_REG s2, cpu_regs_t_s2(a1)
    ST_REG s3, cpu_regs_t_s3(a1)
    ST_REG s4, cpu_regs_t_s4(a1)
    ST_REG s5, cpu_regs_t_s5(a1)
    ST_REG s6, cpu_regs_t_s6(a1)
    ST_REG s7, cpu_regs_t_s7(a1)
    ST_REG s8, cpu_regs_t_s8(a1)
    ST_REG s9, cpu_regs_t_s9(a1)
    ST_REG s10, cpu_regs_t_s10(a1)
    ST_REG s11, cpu_regs_t_s11(a1)
    // ST_REG t3, cpu_regs_t_t3(a1) // Tempregs omitted
    // ST_REG t4, cpu_regs_t_t4(a1)
    // ST_REG t5, cpu_regs_t_t5(a1)
    // ST_REG t6, cpu_regs_t_t6(a1)
    jr a2
#pragma endregion
