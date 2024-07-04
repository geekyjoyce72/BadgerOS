
// SPDX-License-Identifier: MIT

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MEMPROTECT_FLAG_R   0x00000001
#define MEMPROTECT_FLAG_W   0x00000002
#define MEMPROTECT_FLAG_RW  0x00000003
#define MEMPROTECT_FLAG_X   0x00000004
#define MEMPROTECT_FLAG_RX  0x00000005
#define MEMPROTECT_FLAG_WX  0x00000006
#define MEMPROTECT_FLAG_RWX 0x00000007

#include "port/memprotect.h"
#include "process/types.h"



// Initialise memory protection driver.
void memprotect_init();
// Create a memory protection context.
void memprotect_create(mpu_ctx_t *ctx);
// Clean up a memory protection context.
void memprotect_destroy(mpu_ctx_t *ctx);
// Add a memory protection region for user memory.
bool memprotect_u(proc_memmap_t *new_mm, mpu_ctx_t *ctx, size_t vaddr, size_t paddr, size_t length, uint32_t flags);
// Add a memory protection region for kernel memory.
bool memprotect_k(size_t vaddr, size_t paddr, size_t length, uint32_t flags);
// Commit pending memory protections, if any.
void memprotect_commit(mpu_ctx_t *ctx);
// Swap in memory protections for the current thread.
void memprotect_swap_from_isr();
