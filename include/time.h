
// SPDX-License-Identifier: MIT

#pragma once

#include <hardware.h>
#include <stdint.h>

// Initialise timer and watchdog subsystem.
void time_init();
// Get current time in microseconds.
int64_t time_us();
