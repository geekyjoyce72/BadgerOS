
// SPDX-License-Identifier: MIT

#pragma once

#include <stddef.h>
#include <stdint.h>

// Early hardware initialization.
void port_early_init();
// Tell port to add other available memory to the pools.
void port_post_memprotect_init();
// Full hardware initialization.
void port_init();

// Send a single character to the log output.
void port_putc(char msg);

// Fence data and instruction memory for executable mapping.
static inline void port_fencei() {
    asm("fence rw,rw");
    asm("fence.i");
}
