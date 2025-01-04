
// SPDX-License-Identifier: MIT

#pragma once

#include "driver/pcie.h"



// Send a single ATA command synchronously.
typedef void (*ata_cmd_sync_t)();

// ATA virtual function table.
typedef struct {
    ata_cmd_sync_t cmd_sync;
} ata_vtable_t;

// ATA device handle.
typedef struct {
    // ATA virtual function table.
    ata_vtable_t const *vtable;
} ata_handle_t;
