
// SPDX-License-Identifier: MIT

#pragma once

#include "driver/pcie.h"



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


// HBA bar register: generic host control - not meant to be constructed.
typedef struct {
    // TODO: Host Capabilities.
    uint32_t cap;
    // TODO: Global Host Control.
    uint32_t ghc;
    // TODO: Interrupt Status.
    uint32_t is;
    // TODO: Ports Implemented.
    uint32_t pi;
    // TODO: Version.
    uint32_t vs;
    // TODO: Command Completion Coalescing Control.
    uint32_t ccc_ctl;
    // TODO: Command Completion Coalsecing Ports.
    uint32_t ccc_ports;
    // TODO: Enclosure Management Location.
    uint32_t em_loc;
    // TODO: Enclosure Management Control.
    uint32_t em_ctl;
    // TODO: Host Capabilities Extended.
    uint32_t cap2;
    // TODO: BIOS/OS Handoff Control and Status.
    uint32_t bohc;
} ahci_bar_ghc_t;

// HBA bar register: port control - not meant to be constructed.
typedef struct {
} ahci_bar_port_t;

// All HBA BAR registers - not meant to be constructed.
typedef struct {
    // Generic host control.
    ahci_bar_ghc_t  generic_host_ctrl;
    // Reserved.
    uint8_t         _reserved0[52];
    // Reserved for NVMHCI.
    uint8_t         _reserved1[64];
    // Vendor-specific registers.
    uint8_t         _reserved2[96];
    // Port control registers.
    ahci_bar_port_t ports[32];
} achi_bar_t;


// SATA controller handle.
typedef struct {
    // PCI address.
    pcie_addr_t addr;
} sata_handle_t;
