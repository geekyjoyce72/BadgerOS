
// SPDX-License-Identifier: MIT

#pragma once

#include <regs.h>

// Call this function when and only when the kernel has encountered a fatal error.
void panic_abort() __attribute__((noreturn));
