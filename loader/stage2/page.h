#pragma once

#include <stdint.h>
#include "offsets.h"

// PML4T: 512GB * 512 = 256TB
// PDPT: 1GB * 512 = 512GB
// PDT: 2MB * 512 = 1GB
// PT: 4KB * 512 = 2MB
// PTE: 4KB

#define PAGE_PRESENT 1
#define PAGE_RW 2
#define PAGE_USER 4
#define PAGE_LARGE 128
