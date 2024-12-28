
// SPDX-License-Identifier: MIT

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>



// AHCI port interrupt numbers.
typedef union {
    struct {
        // Device to host register FIS interrupt.
        uint32_t d2h_reg_fis   : 1;
        // PIO setup FIS interrupt.
        uint32_t pio_setup_fis : 1;
        // DMA setup FIS interrupt.
        uint32_t dma_setup_fis : 1;
        // Set device bits interrupt.
        uint32_t set_dev_bits  : 1;
        // Unknown FIS interrupt.
        uint32_t unknown_fis   : 1;
        // Descriptor processed.
        uint32_t desc_proc     : 1;
        // Port connect status change.
        uint32_t port_status   : 1;
        // Device mechanical presence status.
        uint32_t presence      : 1;
        // Reserved.
        uint32_t               : 14;
        // PhyRdy signal changed.
        uint32_t phy_rdy       : 1;
        // Incorrect port multiplier status.
        uint32_t port_mul_err  : 1;
        // Overflow status.
        uint32_t overflow      : 1;
    };
    uint32_t val;
} achi_bar_port_irq_t;

// Command and status register.
typedef union {
    struct {
        // Start command list.
        uint32_t cmd_start               : 1;
        // Spin up device.
        uint32_t spinup                  : 1;
        // Power on device.
        uint32_t poweron                 : 1;
        // Command list override.
        uint32_t clo                     : 1;
        // FIS receive enable.
        uint32_t fis_en                  : 1;
        // Reserved
        uint32_t                         : 3;
        // Current command slot being issued.
        uint32_t cur_slot                : 5;
        // Presence switch state.
        uint32_t present_switch          : 1;
        // FIS receive DMA engine is running.
        uint32_t fis_running             : 1;
        // Command list running.
        uint32_t cmd_running             : 1;
        // Cold presence state.
        uint32_t present_cold            : 1;
        // Port multiplier attached.
        uint32_t port_mul_det            : 1;
        // Hot plug capable port.
        uint32_t supports_hotplug        : 1;
        // Port has mechanical presence switch.
        uint32_t supports_present_switch : 1;
        // Supports cold presence detection.
        uint32_t supports_present_cold   : 1;
        // Is an external port.
        uint32_t is_external             : 1;
        // FIS-based switching capable.
        uint32_t supports_fis_switching  : 1;
        // Enable automatic partial to slumber.
        uint32_t auto_slumber_en         : 1;
        // Is an ATAPI device.
        uint32_t dev_is_atapi            : 1;
        // Enable drive LED even for ATAPI devices.
        uint32_t drive_led_atapi         : 1;
    };
    uint32_t val;
} achi_bar_port_cmd_t;

// HBA BAR registers: AHCI port.
typedef struct {
    // Command list base address.
    uint64_t            cmdlist_addr;
    // FIS base address.
    uint64_t            fis_addr;
    // Interrupt status.
    achi_bar_port_irq_t irq_status;
    // Interrupt enable.
    achi_bar_port_irq_t irq_enable;
    // Command and status register.
    achi_bar_port_cmd_t cmd;
} ahci_bar_port_t;
