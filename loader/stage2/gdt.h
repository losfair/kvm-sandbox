#pragma once

#include <stdint.h>
#include "offsets.h"
#include "tss.h"

#define GDT64_ENTRY_CODE_DPL_0 0x20980000000000
#define GDT64_ENTRY_CODE_DPL_3 0x20f80000000000
#define GDT64_ENTRY_DATA 0x900000000000

#define GDT_ENTRY_COUNT 6

static void encode_tss(volatile uint64_t *out, uint64_t addr) {
    out[0] = (sizeof(struct Tss) - 1) & 0xffff; // 0-16
    out[0] |= (addr & 0xffffff) << 16; // 16-40
    out[0] |= 0x9ull << 40; // 40-44
    out[0] |= 1ull << 47; // bit 47 (present)
    out[0] |= ((addr >> 24) & 0xff) << 56; // bit 56-64
    out[1] = addr >> 32;
}

static __attribute__((used)) inline void gdt_setup_64() {
    gdt[0] = 0;
    gdt[1] = GDT64_ENTRY_CODE_DPL_0;
    gdt[2] = GDT64_ENTRY_CODE_DPL_3;
    gdt[3] = GDT64_ENTRY_DATA;
    encode_tss(&gdt[4], (uintptr_t) tss + 0xffff800000000000); // tss takes two entries
}
