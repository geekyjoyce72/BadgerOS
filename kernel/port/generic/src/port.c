
// SPDX-License-Identifier: MIT

#include "port/port.h"

#include "assertions.h"
#include "cpu/panic.h"
#include "interrupt.h"
#include "limine.h"
#include "malloc.h"
#include "port/hardware_allocation.h"
#include "time.h"

#include <stdbool.h>

void init_pool(void *mem_start, void *mem_end, uint32_t flags);



#define REQ __attribute__((section(".requests")))

LIMINE_BASE_REVISION(2);
__attribute__((section(".requests_start"))) LIMINE_REQUESTS_START_MARKER;

static REQ struct limine_memmap_request mm_req = {
    .id       = LIMINE_MEMMAP_REQUEST,
    .revision = 2,
};

static REQ struct limine_dtb_request dtb_req = {
    .id       = LIMINE_DTB_REQUEST,
    .revision = 2,
};

__attribute__((section(".requests_end"))) LIMINE_REQUESTS_END_MARKER;



// Early hardware initialization.
void port_early_init() {
    // Verify needed requests have been answered.
    if (!mm_req.response) {
        logk_from_isr(LOG_FATAL, "Limine memmap response missing");
        panic_poweroff();
    }
    // if (!dtb_req.response) {
    //     logk_from_isr(LOG_FATAL, "Limine DTB response missing");
    //     panic_poweroff();
    // }

    // Print memory map.
    struct limine_memmap_response *mem     = mm_req.response;
    char const *const              types[] = {
        [LIMINE_MEMMAP_USABLE]                 = "Usable",
        [LIMINE_MEMMAP_RESERVED]               = "Reserved",
        [LIMINE_MEMMAP_ACPI_RECLAIMABLE]       = "ACPI Reclaimable",
        [LIMINE_MEMMAP_ACPI_NVS]               = "ACPI NVS",
        [LIMINE_MEMMAP_BAD_MEMORY]             = "Bad",
        [LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE] = "Bootloader reclaimable",
        [LIMINE_MEMMAP_KERNEL_AND_MODULES]     = "Kernel",
        [LIMINE_MEMMAP_FRAMEBUFFER]            = "Framebuffer",
    };
    logkf_from_isr(LOG_INFO, "Memory map:");
    for (uint64_t i = 0; i < mem->entry_count; i++) {
        logkf_from_isr(
            LOG_INFO,
            "%{u64;x}-%{u64;x} %{cs}",
            mem->entries[i]->base,
            mem->entries[i]->base + mem->entries[i]->length - 1,
            types[mem->entries[i]->type]
        );
    }
}

// Full hardware initialization.
void port_init() {
}

// Send a single character to the log output.
void port_putc(char msg) __attribute__((naked));
void port_putc(char msg) {
    (void)msg;
    // SBI console putchar.
    asm("li a7, 1; ecall; ret");
}
