#pragma once

#include <stdint.h>

struct __attribute__((packed)) Tss {
    uint32_t _reserved_0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t _reserved_1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t _reserved_2;
    uint32_t iopb_offset;
};
