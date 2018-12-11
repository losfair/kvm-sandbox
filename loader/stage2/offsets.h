#pragma once

#include <stdint.h>
#include "tss.h"
#include "../../layout.hpp"

static volatile __attribute__((used)) uint64_t *gdt = (uint64_t *) (FIXED_DATA_OFFSET + 0); // 8192 entries
static volatile __attribute__((used)) struct IdtEntry *idt = (struct IdtEntry *) (FIXED_DATA_OFFSET + 0x10000); // 256 * 16
static volatile __attribute__((used)) struct Tss *tss = (struct Tss *) (FIXED_DATA_OFFSET + 0x11000);
static volatile __attribute__((used)) uint64_t *pml4t = (uint64_t *) (FIXED_DATA_OFFSET + 0x12000); // 512 entries
static volatile __attribute__((used)) uint64_t *kernel_pdpt = (uint64_t *) (FIXED_DATA_OFFSET + 0x13000);
static volatile __attribute__((used)) uint64_t *kernel_pdt = (uint64_t *) (FIXED_DATA_OFFSET + 0x14000);
static volatile __attribute__((used)) uint64_t *kernel_pt = (uint64_t *) (FIXED_DATA_OFFSET + 0x15000);

#define KERNEL_HEAP ((uint8_t *) (FIXED_DATA_OFFSET + 0x100000))
