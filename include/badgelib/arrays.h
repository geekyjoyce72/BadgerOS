
// SPDX-License-Identifier: MIT

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>



// Comparator function for sorting functions.
typedef int (*array_sort_comp_t)(void const *a, void const *b);
// Sort a contiguous array given a comparator function.
// The array is sorted into ascending order.
void array_sort(void *array, size_t ent_size, size_t ent_count, array_sort_comp_t comparator);
