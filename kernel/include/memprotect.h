
// SPDX-License-Identifier: MIT

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MEMPROTECT_FLAG_R      0x00000001
#define MEMPROTECT_FLAG_W      0x00000002
#define MEMPROTECT_FLAG_X      0x00000004
#define MEMPROTECT_FLAG_KERNEL 0x80000000

// #include <port/memprotect.h>
// #include <process/process.h>
typedef struct {
} mpu_ctx_t;



// MPU context for kernel threads not associated to processes.
extern mpu_ctx_t kernel_mpu_ctx;

// Initialise memory protection driver.
void memprotect_init();
// Add a memory protection region.
bool memprotect(mpu_ctx_t *ctx, size_t vaddr, size_t paddr, size_t length, uint32_t flags);
// Commit pending memory protections, if any.
void memprotect_commit(mpu_ctx_t *ctx);
