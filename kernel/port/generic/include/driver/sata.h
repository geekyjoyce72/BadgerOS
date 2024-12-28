
// SPDX-License-Identifier: MIT

#pragma once

#include "driver/pcie.h"
#include "driver/sata/ahci_port.h"
#include "driver/sata/generic_host_ctrl.h"
#include "driver/sata/pci_cap.h"



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
_Static_assert(offsetof(achi_bar_t, ports) == 0x100, "Offset of ports in ahci_bar_t must be 0x100");



// SATA controller handle.
typedef struct {
    // PCI address.
    pcie_addr_t addr;
} sata_handle_t;
