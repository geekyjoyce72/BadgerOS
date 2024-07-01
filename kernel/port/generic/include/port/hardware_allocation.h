
// SPDX-License-Identifier: MIT

#pragma once



/* ==== Interrupts ==== */

// Interrupt channel used for timer alarms.
#define INT_CHANNEL_TIMER_ALARM    16
// Interrupt channel used for watchdog alarms.
#define INT_CHANNEL_WATCHDOG_ALARM 17
// Interrupt channel used for UART.
#define INT_CHANNEL_UART           18
// Interrupt channel used for I²C.
#define INT_CHANNEL_I2C            19
// Interrupt channel used for SPI.
#define INT_CHANNEL_SPI            20

// Default priority for timer alarm interrupts.
#define INT_PRIO_TIMER_ALARM    11
// Default priority for timer watchdog interrupts.
#define INT_PRIO_WATCHDOG_ALARM 15
// Interrupt channel used for UART interrupts.
#define INT_PRIO_UART           14
// Interrupt channel used for I²C interrupts.
#define INT_PRIO_I2C            13
// Interrupt channel used for SPI interrupts.
#define INT_PRIO_SPI            12



/* ==== Timers ==== */

// Number of usable hardware timers.
#define TIMER_COUNT        4
// Timer used for system timekeeping.
#define TIMER_SYSTICK_NUM  0
// System tick rate in Hz.
#define TIMER_SYSTICK_RATE 1000000



/* ==== Memory protection regions ==== */

// Kernel supports virtual memory.
#define MEMMAP_VMEM             0
// Page size for memory protections.
#define MEMMAP_PAGE_SIZE        4096
// Maximum number of mapped regions per process.
#define PROC_MEMMAP_MAX_REGIONS 8
// Lowest numbered PMP to use for process memory maps, must be a multiple of 4.
#define PROC_RISCV_PMP_START    0
// Number of PMPs to use for process memory maps, must be a multiple of 4.
#define PROC_RISCV_PMP_COUNT    8
