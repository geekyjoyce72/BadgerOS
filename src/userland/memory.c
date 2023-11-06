
// SPDX-License-Identifier: MIT

#include "userland/memory.h"

#include "badge_strings.h"
#include "log.h"
#include "malloc.h"

process_t dummy_proc;



// Allocate more memory to a process.
size_t user_map(process_t *proc, size_t vaddr_req, size_t min_size, size_t min_align) {
    (void)min_align;
    (void)vaddr_req;

    proc_memmap_t *map = &proc->memmap;
    if (map->regions_len >= PROC_MEMMAP_MAX_REGIONS)
        return false;

    void *base = malloc(min_size);
    if (!base)
        return false;

    logkf(LOG_INFO, "Mapped %{size;d} bytes to process %{d}", min_size, proc->pid);

    map->region_bases[map->regions_len] = (size_t)base;
    map->region_sizes[map->regions_len] = min_size;
    map->regions_len++;

    return (size_t)base;
}

// Release memory allocated to a process.
void user_unmap(process_t *proc, size_t base) {
    proc_memmap_t *map = &proc->memmap;
    for (size_t i = 0; i < map->regions_len; i++) {
        if (map->region_bases[i] == base) {
            free((void *)base);
            mem_copy(map->region_bases + i, map->region_bases + i + 1, sizeof(size_t) * (map->regions_len - i - 1));
            mem_copy(map->region_sizes + i, map->region_sizes + i + 1, sizeof(size_t) * (map->regions_len - i - 1));
            map->regions_len--;
            return;
        }
    }
}
