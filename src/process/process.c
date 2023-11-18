
// SPDX-License-Identifier: MIT

#include "process/process.h"

#include "arrays.h"
#include "badge_strings.h"
#include "log.h"
#include "malloc.h"
#include "port/process/process.h"
#include "process/types.h"

process_t dummy_proc;

// Create a new, empty process.
process_t *proc_create() {
    return &dummy_proc;
}

// Delete a process and release any resources it had.
void proc_delete(pid_t pid) {
    (void)pid;
}

// Get a process handle by ID.
process_t *proc_get(pid_t pid) {
    (void)pid;
    return &dummy_proc;
}



// Create a new thread in a process.
// Returns created thread handle.
sched_thread_t *proc_create_thread(badge_err_t *ec, sched_entry_point_t entry_point, void *arg, sched_prio_t priority) {
    (void)ec;
    (void)entry_point;
    (void)arg;
    (void)priority;
    return NULL;
}

// Memory map address comparator.
static int prog_memmap_cmp(void const *a, void const *b) {
    proc_memmap_ent_t const *a_ptr = a;
    proc_memmap_ent_t const *b_ptr = b;
    if (a_ptr->base < b_ptr->base)
        return -1;
    if (a_ptr->base < b_ptr->base)
        return 1;
    return 0;
}

// Sort the memory map by ascending address.
static inline void proc_memmap_sort(proc_memmap_t *memmap) {
    array_sort(&memmap->regions[0], sizeof(memmap->regions[0]), memmap->regions_len, prog_memmap_cmp);
}

// Allocate more memory to a process.
size_t proc_map(process_t *proc, size_t vaddr_req, size_t min_size, size_t min_align) {
    (void)min_align;
    (void)vaddr_req;

    proc_memmap_t *map = &proc->memmap;
    if (map->regions_len >= PROC_MEMMAP_MAX_REGIONS)
        return 0;

    void *base = malloc(min_size);
    if (!base)
        return 0;

    map->regions[map->regions_len] = (proc_memmap_ent_t){
        .base  = (size_t)base,
        .size  = (size_t)min_size,
        .write = true,
        .exec  = true,
    };
    map->regions_len++;
    proc_memmap_sort(map);
    if (!proc_mpu_gen(map)) {
        proc_unmap(proc, (size_t)base);
        return 0;
    }

    logkf(LOG_INFO, "Mapped %{size;d} bytes at %{size;x} to process %{d}", min_size, base, proc->pid);

    return (size_t)base;
}

// Release memory allocated to a process.
void proc_unmap(process_t *proc, size_t base) {
    proc_memmap_t *map = &proc->memmap;
    for (size_t i = 0; i < map->regions_len; i++) {
        if (map->regions[i].base == base) {
            free((void *)base);
            array_remove(&map->regions[0], sizeof(map->regions[0]), map->regions_len, NULL, i);
            map->regions_len--;
            return;
        }
    }
}
