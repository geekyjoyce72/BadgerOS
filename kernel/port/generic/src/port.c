
// SPDX-License-Identifier: MIT

#include "port/port.h"

#include "interrupt.h"
#include "port/hardware_allocation.h"
#include "time.h"

#include <stdbool.h>


// Early hardware initialization.
void port_early_init() {
}

// Full hardware initialization.
void port_init() {
}

// Send a single character to the log output.
void port_putc(char msg) {
    // QEMU virtual UART.
    *(char volatile *)0x10000000 = msg;
}
