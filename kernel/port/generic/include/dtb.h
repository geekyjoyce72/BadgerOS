
// SPDX-License-Identifier: MIT

#pragma once

#include <stdint.h>



// Minimum supported FDT version.
#define FDT_VERSION_MIN  16
// Maximum supported FDT version.
#define FDT_VERSION_MAX  16
// Magic value for FDT headers.
#define FDT_HEADER_MAGIC 0xd00dfeed

// FDT node types.
typedef enum {
    FDT_BEGIN_NODE = 1,
    FDT_END_NODE,
    FDT_PROP,
    FDT_NOP,
    FDT_END = 9,
} fdt_node_t;

// Struct attributes for FDT.
#define FDT_ATTR __attribute__((scalar_storage_order("big-endian")))

// FDT header struct.
typedef struct FDT_ATTR {
    // This field shall contain 0xd00dfeed.
    uint32_t magic;
    // The size of the entire FDT including this header.
    uint32_t totalsize;
    // Offset in bytes of the structure block.
    uint32_t off_dt_struct;
    // Offset in bytes of the string block.
    uint32_t off_dt_strings;
    // Offset in bytes of the memory reservation block.
    uint32_t off_mem_rsvmap;
    // FDT version.
    uint32_t version;
    // The oldest version with which this FDT is backwards-compatible.
    uint32_t last_comp_version;
    // Booting CPU ID.
    uint32_t boot_cpuid_phys;
    // Size of the string block.
    uint32_t size_dt_strings;
    // Size of the structure block.
    uint32_t size_dt_struct;
} dtb_header_t;

// FDT reserved memory.
typedef struct FDT_ATTR {
    // Base physical address.
    uint64_t paddr;
    // Size in bytes.
    uint64_t size;
} dtb_rsvmem_t;
