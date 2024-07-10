
// SPDX-License-Identifier: MIT

#include "usercopy.h"

#include "assertions.h"
#include "badge_strings.h"
#include "interrupt.h"
#include "isr_ctx.h"
#include "memprotect.h"
#include "process/internal.h"
#if MEMMAP_VMEM
#include "cpu/mmu.h"
#endif

// TODO: Convert to page fault intercepting memcpy.
// TODO: Migrate into generic process API.



// Determine string length in memory a user owns.
// Returns -1 if the user doesn't have access to any byte in the string.
ptrdiff_t strlen_from_user_raw(process_t *process, size_t user_vaddr, ptrdiff_t max_len) {
    if (!max_len) {
        return 0;
    }
    ptrdiff_t len = 0;

    // Check first page permissions.
    if (!(proc_map_contains_raw(process, user_vaddr, 1) & MEMPROTECT_FLAG_R)) {
        return -1;
    }

    // String length loop.
    asm("csrs sstatus, %0" ::"r"((1 << RISCV_STATUS_SUM_BIT) | (1 << RISCV_STATUS_MXR_BIT)));
    while (len < max_len && *(char const *)user_vaddr) {
        len++;
        user_vaddr++;
        if (user_vaddr % MEMMAP_PAGE_SIZE == 0) {
            // Check further page permissions.
            asm("csrc sstatus, %0" ::"r"((1 << RISCV_STATUS_SUM_BIT) | (1 << RISCV_STATUS_MXR_BIT)));
            if (!(proc_map_contains_raw(process, user_vaddr, 1) & MEMPROTECT_FLAG_R)) {
                return -1;
            }
            asm("csrs sstatus, %0" ::"r"((1 << RISCV_STATUS_SUM_BIT) | (1 << RISCV_STATUS_MXR_BIT)));
        }
    }
    asm("csrc sstatus, %0" ::"r"((1 << RISCV_STATUS_SUM_BIT) | (1 << RISCV_STATUS_MXR_BIT)));

    return len;
}

// Copy bytes from user to kernel.
// Returns whether the user has access to all of these bytes.
// If the user doesn't have access, no copy is performed.
bool copy_from_user_raw(process_t *process, void *kernel_vaddr, size_t user_vaddr, size_t len) {
    if (!proc_map_contains_raw(process, user_vaddr, len)) {
        return false;
    }
#if RISCV_M_MODE_KERNEL
    mem_copy(kernel_vaddr, (void const *)user_vaddr, len);
#else
    memprotect_swap(&process->memmap.mpu_ctx);
    asm("csrs sstatus, %0" ::"r"((1 << RISCV_STATUS_SUM_BIT) | (1 << RISCV_STATUS_MXR_BIT)));
    mem_copy(kernel_vaddr, (void *)user_vaddr, len);
    asm("csrc sstatus, %0" ::"r"((1 << RISCV_STATUS_SUM_BIT) | (1 << RISCV_STATUS_MXR_BIT)));
#endif
    return true;
}

// Copy from kernel to user.
// Returns whether the user has access to all of these bytes.
// If the user doesn't have access, no copy is performed.
bool copy_to_user_raw(process_t *process, size_t user_vaddr, void *_kernel_vaddr, size_t len) {
    uint8_t const *kernel_vaddr = _kernel_vaddr;
    if (!proc_map_contains_raw(process, user_vaddr, len)) {
        return false;
    }
#if RISCV_M_MODE_KERNEL
    mem_copy((void *)user_vaddr, kernel_vaddr, len);
#else
    while (len) {
        virt2phys_t info = memprotect_virt2phys(&process->memmap.mpu_ctx, user_vaddr);
        assert_dev_drop((info.flags & (MEMPROTECT_FLAG_KERNEL | MEMPROTECT_FLAG_KERNEL)) == 0);
        size_t max_len = info.page_size - (user_vaddr & (info.page_size - 1));
        max_len        = len < max_len ? len : max_len;
        mem_copy((uint8_t *)mmu_hhdm_vaddr + info.paddr, kernel_vaddr, max_len);
        len          -= max_len;
        user_vaddr   += max_len;
        kernel_vaddr += max_len;
    }
#endif
    return true;
}
