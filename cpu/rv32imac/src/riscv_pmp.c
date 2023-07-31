
// SPDX-License-Identifier: MIT

#include "cpu/riscv_pmp.h"

#include "cpu/panic.h"
#include "log.h"

// Initialise memory protection driver.
void riscv_pmp_init() {
    // Make sure we're the first to touch PMP so all of it is in our control;
    // Check that none of the protection regions are currently locked.
    riscv_pmpcfg_t cfg[RISCV_PMP_REGION_COUNT];
    riscv_pmpcfg_read_all(cfg);

    bool lock = false;
    for (int i = 0; i < RISCV_PMP_REGION_COUNT; i++) {
        if (cfg[i].lock) {
            logkf(LOG_FATAL, "RISC-V pmp%{d}cfg is locked!");
        }
    }

    if (lock) {
        panic_abort();
    }

    // Zero out PMP config, effectively disabling it.
#if __riscv_xlen == 32
    asm volatile("csrw pmpcfg0, x0");
    asm volatile("csrw pmpcfg1, x0");
    asm volatile("csrw pmpcfg2, x0");
    asm volatile("csrw pmpcfg3, x0");
#endif
#if __riscv_xlen == 32 && RISCV_PMP_REGION_COUNT == 64
    asm volatile("csrw pmpcfg4, x0");
    asm volatile("csrw pmpcfg5, x0");
    asm volatile("csrw pmpcfg6, x0");
    asm volatile("csrw pmpcfg7, x0");
    asm volatile("csrw pmpcfg8, x0");
    asm volatile("csrw pmpcfg9, x0");
    asm volatile("csrw pmpcfg10, x0");
    asm volatile("csrw pmpcfg11, x0");
    asm volatile("csrw pmpcfg12, x0");
    asm volatile("csrw pmpcfg13, x0");
    asm volatile("csrw pmpcfg14, x0");
    asm volatile("csrw pmpcfg15, x0");
#endif

#if __riscv_xlen == 64
    asm volatile("csrw pmpcfg0, x0");
    asm volatile("csrw pmpcfg2, x0");
#endif
#if __riscv_xlen == 64 && RISCV_PMP_REGION_COUNT == 64
    asm volatile("csrw pmpcfg4, x0");
    asm volatile("csrw pmpcfg6, x0");
    asm volatile("csrw pmpcfg8, x0");
    asm volatile("csrw pmpcfg10, x0");
    asm volatile("csrw pmpcfg12, x0");
    asm volatile("csrw pmpcfg14, x0");
#endif
}



// Read all raw PMP configurations.
void riscv_pmpcfg_read_all(riscv_pmpcfg_t cfg_out[RISCV_PMP_REGION_COUNT]) {
    long *word_out = (void *)cfg_out;

#if __riscv_xlen == 32
    asm volatile("csrr %0, pmpcfg0" : "=r"(word_out[0]));
    asm volatile("csrr %0, pmpcfg1" : "=r"(word_out[1]));
    asm volatile("csrr %0, pmpcfg2" : "=r"(word_out[2]));
    asm volatile("csrr %0, pmpcfg3" : "=r"(word_out[3]));
#endif
#if __riscv_xlen == 32 && RISCV_PMP_REGION_COUNT == 64
    asm volatile("csrr %0, pmpcfg4" : "=r"(word_out[4]));
    asm volatile("csrr %0, pmpcfg5" : "=r"(word_out[5]));
    asm volatile("csrr %0, pmpcfg6" : "=r"(word_out[6]));
    asm volatile("csrr %0, pmpcfg7" : "=r"(word_out[7]));
    asm volatile("csrr %0, pmpcfg8" : "=r"(word_out[8]));
    asm volatile("csrr %0, pmpcfg9" : "=r"(word_out[9]));
    asm volatile("csrr %0, pmpcfg10" : "=r"(word_out[10]));
    asm volatile("csrr %0, pmpcfg11" : "=r"(word_out[11]));
    asm volatile("csrr %0, pmpcfg12" : "=r"(word_out[12]));
    asm volatile("csrr %0, pmpcfg13" : "=r"(word_out[13]));
    asm volatile("csrr %0, pmpcfg14" : "=r"(word_out[14]));
    asm volatile("csrr %0, pmpcfg15" : "=r"(word_out[15]));
#endif

#if __riscv_xlen == 64
    asm volatile("csrr %0, pmpcfg0" : "=r"(word_out[0]));
    asm volatile("csrr %0, pmpcfg2" : "=r"(word_out[1]));
#endif
#if __riscv_xlen == 64 && RISCV_PMP_REGION_COUNT == 64
    asm volatile("csrr %0, pmpcfg4" : "=r"(word_out[2]));
    asm volatile("csrr %0, pmpcfg6" : "=r"(word_out[3]));
    asm volatile("csrr %0, pmpcfg8" : "=r"(word_out[4]));
    asm volatile("csrr %0, pmpcfg10" : "=r"(word_out[5]));
    asm volatile("csrr %0, pmpcfg12" : "=r"(word_out[6]));
    asm volatile("csrr %0, pmpcfg14" : "=r"(word_out[7]));
#endif
}

// Read all raw PMP addresses.
void riscv_pmpaddr_read_all(size_t addr_out[RISCV_PMP_REGION_COUNT]) {
    asm volatile("csrr %0, pmpaddr0" : "=r"(addr_out[0]));
    asm volatile("csrr %0, pmpaddr1" : "=r"(addr_out[1]));
    asm volatile("csrr %0, pmpaddr2" : "=r"(addr_out[2]));
    asm volatile("csrr %0, pmpaddr3" : "=r"(addr_out[3]));
    asm volatile("csrr %0, pmpaddr4" : "=r"(addr_out[4]));
    asm volatile("csrr %0, pmpaddr5" : "=r"(addr_out[5]));
    asm volatile("csrr %0, pmpaddr6" : "=r"(addr_out[6]));
    asm volatile("csrr %0, pmpaddr7" : "=r"(addr_out[7]));
    asm volatile("csrr %0, pmpaddr8" : "=r"(addr_out[8]));
    asm volatile("csrr %0, pmpaddr9" : "=r"(addr_out[9]));
    asm volatile("csrr %0, pmpaddr10" : "=r"(addr_out[10]));
    asm volatile("csrr %0, pmpaddr11" : "=r"(addr_out[11]));
    asm volatile("csrr %0, pmpaddr12" : "=r"(addr_out[12]));
    asm volatile("csrr %0, pmpaddr13" : "=r"(addr_out[13]));
    asm volatile("csrr %0, pmpaddr14" : "=r"(addr_out[14]));
    asm volatile("csrr %0, pmpaddr15" : "=r"(addr_out[15]));
#if RISCV_PMP_REGION_COUNT == 64
    asm volatile("csrr %0, pmpaddr16" : "=r"(addr_out[16]));
    asm volatile("csrr %0, pmpaddr17" : "=r"(addr_out[17]));
    asm volatile("csrr %0, pmpaddr18" : "=r"(addr_out[18]));
    asm volatile("csrr %0, pmpaddr19" : "=r"(addr_out[19]));
    asm volatile("csrr %0, pmpaddr20" : "=r"(addr_out[20]));
    asm volatile("csrr %0, pmpaddr21" : "=r"(addr_out[21]));
    asm volatile("csrr %0, pmpaddr22" : "=r"(addr_out[22]));
    asm volatile("csrr %0, pmpaddr23" : "=r"(addr_out[23]));
    asm volatile("csrr %0, pmpaddr24" : "=r"(addr_out[24]));
    asm volatile("csrr %0, pmpaddr25" : "=r"(addr_out[25]));
    asm volatile("csrr %0, pmpaddr26" : "=r"(addr_out[26]));
    asm volatile("csrr %0, pmpaddr27" : "=r"(addr_out[27]));
    asm volatile("csrr %0, pmpaddr28" : "=r"(addr_out[28]));
    asm volatile("csrr %0, pmpaddr29" : "=r"(addr_out[29]));
    asm volatile("csrr %0, pmpaddr30" : "=r"(addr_out[30]));
    asm volatile("csrr %0, pmpaddr31" : "=r"(addr_out[31]));
    asm volatile("csrr %0, pmpaddr32" : "=r"(addr_out[32]));
    asm volatile("csrr %0, pmpaddr33" : "=r"(addr_out[33]));
    asm volatile("csrr %0, pmpaddr34" : "=r"(addr_out[34]));
    asm volatile("csrr %0, pmpaddr35" : "=r"(addr_out[35]));
    asm volatile("csrr %0, pmpaddr36" : "=r"(addr_out[36]));
    asm volatile("csrr %0, pmpaddr37" : "=r"(addr_out[37]));
    asm volatile("csrr %0, pmpaddr38" : "=r"(addr_out[38]));
    asm volatile("csrr %0, pmpaddr39" : "=r"(addr_out[39]));
    asm volatile("csrr %0, pmpaddr40" : "=r"(addr_out[40]));
    asm volatile("csrr %0, pmpaddr41" : "=r"(addr_out[41]));
    asm volatile("csrr %0, pmpaddr42" : "=r"(addr_out[42]));
    asm volatile("csrr %0, pmpaddr43" : "=r"(addr_out[43]));
    asm volatile("csrr %0, pmpaddr44" : "=r"(addr_out[44]));
    asm volatile("csrr %0, pmpaddr45" : "=r"(addr_out[45]));
    asm volatile("csrr %0, pmpaddr46" : "=r"(addr_out[46]));
    asm volatile("csrr %0, pmpaddr47" : "=r"(addr_out[47]));
    asm volatile("csrr %0, pmpaddr48" : "=r"(addr_out[48]));
    asm volatile("csrr %0, pmpaddr49" : "=r"(addr_out[49]));
    asm volatile("csrr %0, pmpaddr50" : "=r"(addr_out[50]));
    asm volatile("csrr %0, pmpaddr51" : "=r"(addr_out[51]));
    asm volatile("csrr %0, pmpaddr52" : "=r"(addr_out[52]));
    asm volatile("csrr %0, pmpaddr53" : "=r"(addr_out[53]));
    asm volatile("csrr %0, pmpaddr54" : "=r"(addr_out[54]));
    asm volatile("csrr %0, pmpaddr55" : "=r"(addr_out[55]));
    asm volatile("csrr %0, pmpaddr56" : "=r"(addr_out[56]));
    asm volatile("csrr %0, pmpaddr57" : "=r"(addr_out[57]));
    asm volatile("csrr %0, pmpaddr58" : "=r"(addr_out[58]));
    asm volatile("csrr %0, pmpaddr59" : "=r"(addr_out[59]));
    asm volatile("csrr %0, pmpaddr60" : "=r"(addr_out[60]));
    asm volatile("csrr %0, pmpaddr61" : "=r"(addr_out[61]));
    asm volatile("csrr %0, pmpaddr62" : "=r"(addr_out[62]));
    asm volatile("csrr %0, pmpaddr63" : "=r"(addr_out[63]));
#endif
}

// Write all raw PMP configurations.
void riscv_pmpcfg_write_all(riscv_pmpcfg_t const cfg_in[RISCV_PMP_REGION_COUNT]) {
    long const *word_in = (void *)cfg_in;

#if __riscv_xlen == 32
    asm volatile("csrw pmpcfg0, %0" ::"r"(word_in[0]));
    asm volatile("csrw pmpcfg1, %0" ::"r"(word_in[1]));
    asm volatile("csrw pmpcfg2, %0" ::"r"(word_in[2]));
    asm volatile("csrw pmpcfg3, %0" ::"r"(word_in[3]));
#endif
#if __riscv_xlen == 32 && RISCV_PMP_REGION_COUNT == 64
    asm volatile("csrw pmpcfg4, %0" ::"r"(word_in[4]));
    asm volatile("csrw pmpcfg5, %0" ::"r"(word_in[5]));
    asm volatile("csrw pmpcfg6, %0" ::"r"(word_in[6]));
    asm volatile("csrw pmpcfg7, %0" ::"r"(word_in[7]));
    asm volatile("csrw pmpcfg8, %0" ::"r"(word_in[8]));
    asm volatile("csrw pmpcfg9, %0" ::"r"(word_in[9]));
    asm volatile("csrw pmpcfg10, %0" ::"r"(word_in[10]));
    asm volatile("csrw pmpcfg11, %0" ::"r"(word_in[11]));
    asm volatile("csrw pmpcfg12, %0" ::"r"(word_in[12]));
    asm volatile("csrw pmpcfg13, %0" ::"r"(word_in[13]));
    asm volatile("csrw pmpcfg14, %0" ::"r"(word_in[14]));
    asm volatile("csrw pmpcfg15, %0" ::"r"(word_in[15]));
#endif

#if __riscv_xlen == 64
    asm volatile("csrw pmpcfg0, %0" ::"r"(word_in[0]));
    asm volatile("csrw pmpcfg2, %0" ::"r"(word_in[1]));
#endif
#if __riscv_xlen == 64 && RISCV_PMP_REGION_COUNT == 64
    asm volatile("csrw pmpcfg4, %0" ::"r"(word_in[2]));
    asm volatile("csrw pmpcfg6, %0" ::"r"(word_in[3]));
    asm volatile("csrw pmpcfg8, %0" ::"r"(word_in[4]));
    asm volatile("csrw pmpcfg10, %0" ::"r"(word_in[5]));
    asm volatile("csrw pmpcfg12, %0" ::"r"(word_in[6]));
    asm volatile("csrw pmpcfg14, %0" ::"r"(word_in[7]));
#endif
}

// Write all raw PMP addresses.
void riscv_pmpaddr_write_all(size_t const addr_in[RISCV_PMP_REGION_COUNT]) {
    asm volatile("csrw pmpaddr0, %0" ::"r"(addr_in[0]));
    asm volatile("csrw pmpaddr1, %0" ::"r"(addr_in[1]));
    asm volatile("csrw pmpaddr2, %0" ::"r"(addr_in[2]));
    asm volatile("csrw pmpaddr3, %0" ::"r"(addr_in[3]));
    asm volatile("csrw pmpaddr4, %0" ::"r"(addr_in[4]));
    asm volatile("csrw pmpaddr5, %0" ::"r"(addr_in[5]));
    asm volatile("csrw pmpaddr6, %0" ::"r"(addr_in[6]));
    asm volatile("csrw pmpaddr7, %0" ::"r"(addr_in[7]));
    asm volatile("csrw pmpaddr8, %0" ::"r"(addr_in[8]));
    asm volatile("csrw pmpaddr9, %0" ::"r"(addr_in[9]));
    asm volatile("csrw pmpaddr10, %0" ::"r"(addr_in[10]));
    asm volatile("csrw pmpaddr11, %0" ::"r"(addr_in[11]));
    asm volatile("csrw pmpaddr12, %0" ::"r"(addr_in[12]));
    asm volatile("csrw pmpaddr13, %0" ::"r"(addr_in[13]));
    asm volatile("csrw pmpaddr14, %0" ::"r"(addr_in[14]));
    asm volatile("csrw pmpaddr15, %0" ::"r"(addr_in[15]));
#if RISCV_PMP_REGION_COUNT == 64
    asm volatile("csrw pmpaddr16, %0" ::"r"(addr_in[16]));
    asm volatile("csrw pmpaddr17, %0" ::"r"(addr_in[17]));
    asm volatile("csrw pmpaddr18, %0" ::"r"(addr_in[18]));
    asm volatile("csrw pmpaddr19, %0" ::"r"(addr_in[19]));
    asm volatile("csrw pmpaddr20, %0" ::"r"(addr_in[20]));
    asm volatile("csrw pmpaddr21, %0" ::"r"(addr_in[21]));
    asm volatile("csrw pmpaddr22, %0" ::"r"(addr_in[22]));
    asm volatile("csrw pmpaddr23, %0" ::"r"(addr_in[23]));
    asm volatile("csrw pmpaddr24, %0" ::"r"(addr_in[24]));
    asm volatile("csrw pmpaddr25, %0" ::"r"(addr_in[25]));
    asm volatile("csrw pmpaddr26, %0" ::"r"(addr_in[26]));
    asm volatile("csrw pmpaddr27, %0" ::"r"(addr_in[27]));
    asm volatile("csrw pmpaddr28, %0" ::"r"(addr_in[28]));
    asm volatile("csrw pmpaddr29, %0" ::"r"(addr_in[29]));
    asm volatile("csrw pmpaddr30, %0" ::"r"(addr_in[30]));
    asm volatile("csrw pmpaddr31, %0" ::"r"(addr_in[31]));
    asm volatile("csrw pmpaddr32, %0" ::"r"(addr_in[32]));
    asm volatile("csrw pmpaddr33, %0" ::"r"(addr_in[33]));
    asm volatile("csrw pmpaddr34, %0" ::"r"(addr_in[34]));
    asm volatile("csrw pmpaddr35, %0" ::"r"(addr_in[35]));
    asm volatile("csrw pmpaddr36, %0" ::"r"(addr_in[36]));
    asm volatile("csrw pmpaddr37, %0" ::"r"(addr_in[37]));
    asm volatile("csrw pmpaddr38, %0" ::"r"(addr_in[38]));
    asm volatile("csrw pmpaddr39, %0" ::"r"(addr_in[39]));
    asm volatile("csrw pmpaddr40, %0" ::"r"(addr_in[40]));
    asm volatile("csrw pmpaddr41, %0" ::"r"(addr_in[41]));
    asm volatile("csrw pmpaddr42, %0" ::"r"(addr_in[42]));
    asm volatile("csrw pmpaddr43, %0" ::"r"(addr_in[43]));
    asm volatile("csrw pmpaddr44, %0" ::"r"(addr_in[44]));
    asm volatile("csrw pmpaddr45, %0" ::"r"(addr_in[45]));
    asm volatile("csrw pmpaddr46, %0" ::"r"(addr_in[46]));
    asm volatile("csrw pmpaddr47, %0" ::"r"(addr_in[47]));
    asm volatile("csrw pmpaddr48, %0" ::"r"(addr_in[48]));
    asm volatile("csrw pmpaddr49, %0" ::"r"(addr_in[49]));
    asm volatile("csrw pmpaddr50, %0" ::"r"(addr_in[50]));
    asm volatile("csrw pmpaddr51, %0" ::"r"(addr_in[51]));
    asm volatile("csrw pmpaddr52, %0" ::"r"(addr_in[52]));
    asm volatile("csrw pmpaddr53, %0" ::"r"(addr_in[53]));
    asm volatile("csrw pmpaddr54, %0" ::"r"(addr_in[54]));
    asm volatile("csrw pmpaddr55, %0" ::"r"(addr_in[55]));
    asm volatile("csrw pmpaddr56, %0" ::"r"(addr_in[56]));
    asm volatile("csrw pmpaddr57, %0" ::"r"(addr_in[57]));
    asm volatile("csrw pmpaddr58, %0" ::"r"(addr_in[58]));
    asm volatile("csrw pmpaddr59, %0" ::"r"(addr_in[59]));
    asm volatile("csrw pmpaddr60, %0" ::"r"(addr_in[60]));
    asm volatile("csrw pmpaddr61, %0" ::"r"(addr_in[61]));
    asm volatile("csrw pmpaddr62, %0" ::"r"(addr_in[62]));
    asm volatile("csrw pmpaddr63, %0" ::"r"(addr_in[63]));
#endif
}
