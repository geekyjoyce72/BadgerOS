
// SPDX-License-Identifier: MIT

#include "driver.h"
#include "driver/ata/ahci.h"
#include "log.h"



// SATA AHCI vtable.
static ata_vtable_t const sata_vtable = {};

// Initialize SATA AHCI driver with PCI.
void driver_sata_ahci_pci_init(pcie_addr_t addr) {
    // Map the BAR for this AHCI controller.
    void           *cfgm = pcie_ecam_vaddr(addr);
    pcie_hdr_dev_t *hdr  = cfgm;
    logkf(LOG_DEBUG, "Detected SATA AHCI at %{u8;x}:%{u8;x}.%{u8;d}", addr.bus, addr.dev, addr.func);
    pci_bar_handle_t bar = pci_bar_map(&hdr->bar[5]);
    if (!bar.pointer) {
        logkf(LOG_WARN, "Unable to map the BAR; ignoring");
        return;
    }

    // Create SATA controller.
    ahci_bar_t *regs   = bar.pointer;
    int         n_port = regs->generic_host_ctrl.cap.n_ports + 1;
    for (int i = 0; i < n_port; i++) {
        if (regs->ports[i].sstatus.detect) {
            logkf(LOG_DEBUG, "Found drive in slot %{d}", i);
        }
    }
}

DRIVER_DECL(driver_sata_ahci_pcie) = {
    .type                = DRIVER_TYPE_PCI,
    .pci_class.baseclass = PCI_BCLASS_STORAGE,
    .pci_class.subclass  = PCI_SUBCLASS_STORAGE_SATA,
    .pci_class.progif    = PCI_PROGIF_STORAGE_SATA_AHCI,
    .pci_init            = driver_sata_ahci_pci_init,
};
