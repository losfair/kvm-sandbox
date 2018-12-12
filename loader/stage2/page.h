#pragma once

#include <stdint.h>
#include "offsets.h"

// PML4T: 512GB * 512 = 256TB
// PDPT: 1GB * 512 = 512GB
// PDT: 2MB * 512 = 1GB
// PT: 4KB * 512 = 2MB
// PTE: 4KB

#define PDPT_SHIFT 39
#define PDT_SHIFT 30
#define PT_SHIFT 21
#define PTE_SHIFT 12

#define SHIFT_TO_MASK(shift) ((~0ull) << (shift))

#define PAGE_PRESENT 1
#define PAGE_RW 2
#define PAGE_USER 4
#define PAGE_LARGE 128

#define MAX_USER_MEM_GB 4

void init_phys_page_map(void *vaddr);
int phys_alloc(uint64_t *out);
void phys_free(uint64_t addr);
int phys_alloc_to_virt(void **out);
void phys_free_from_virt(void *addr);
