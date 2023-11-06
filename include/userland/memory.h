
// SPDX-License-Identifier: MIT

#pragma once

#include "userland/types.h"

// Allocate more memory to a process.
// Returns actual virtual address on success, 0 on failure.
size_t user_map(process_t *map, size_t vaddr_req, size_t min_size, size_t min_align);
// Release memory allocated to a process.
void   user_unmap(process_t *map, size_t base);
