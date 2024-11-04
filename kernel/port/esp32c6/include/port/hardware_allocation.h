
// SPDX-License-Identifier: MIT

#pragma once



/* ==== Interrupts ==== */

// Kernel runs in M-mode instead of S-mode.
#define RISCV_M_MODE_KERNEL        1
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

// Region size used for nullpointer protection regions.
#define PMP_SIZE_NULLPTR  0x10000000
// Region size used for ROM write protection.
#define PMP_SIZE_ROM_WP   0x00080000
// Region size used for FLASH write protection.
#define PMP_SIZE_FLASH_WP 0x01000000

// Region base used for ROM write protection.
#define PMP_BASE_ROM_WP   0x40000000
// Region base used for ROM write protection.
#define PMP_BASE_FLASH_WP 0x42000000

// NAPOT PMP entry for lower nullpointer protection.
#define PMP_ENTRY_NULLPTR_LOW_NAPOT  12
// NAPOT PMP entry for upper nullpointer protection.
#define PMP_ENTRY_NULLPTR_HIGH_NAPOT 13
// NAPOT PMP entry for ROM write protection
#define PMP_ENTRY_ROM_WP_NAPOT       14
// NAPOT PMP entry for FLASH write protection
#define PMP_ENTRY_FLASH_WP_NAPOT     15
// NAPOT PMP entry for U-mode global permissions.
#define PMP_ENTRY_USER_GLOBAL_NAPOT  11

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
