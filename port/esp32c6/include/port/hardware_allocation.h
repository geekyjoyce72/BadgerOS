
// SPDX-License-Identifier: MIT

#pragma once

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
