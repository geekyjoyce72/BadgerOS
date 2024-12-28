
// SPDX-License-Identifier: MIT

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>



// Identifies a PCI capability as a SATA capability.
#define AHCI_PCI_SATA_CAP 0x12

// Serial ATA capability register 0 - not meant to be constructed.
typedef union {
    struct {
        // Capability ID; must be 0x12.
        uint32_t cap_id   : 8;
        // Next capability.
        uint32_t next_cap : 8;
        // Minor revision; must be 0x0.
        uint32_t min_rev  : 4;
        // Major revision; must be 0x1.
        uint32_t maj_rev  : 4;
        // Reserved.
        uint32_t          : 8;
    };
    uint32_t val;
} satacr0_t;

// Serial ATA capability register 1 - not meant to be constructed.
typedef union {
    struct {
        // BAR location; select BAR to use for index-data pair, or set to 0xF for DWORDS directly following SATACR1.
        uint32_t barloc  : 4;
        // BAR offset; offset, in DWORD granularity, of index-data pair into the BAR.
        uint32_t barofst : 20;
        // Reserved.
        uint32_t         : 0;
    };
    uint32_t val;
} satacr1_t;

// AHCI Serial ATA capability - not meant to be constructed.
typedef struct {
    satacr0_t satacr0;
    satacr1_t satacr1;
} ahci_pci_sata_cap_t;
