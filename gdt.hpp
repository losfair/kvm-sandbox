#pragma once

#include <stdint.h>
#include <unistd.h>

const size_t GDT_ENTRY_SIZE = 8;
const size_t IDT_ENTRY_SIZE = 8;

class GdtEntry {
    public:
    uint32_t base;
    uint32_t limit;
    uint8_t type;

    void encode(uint8_t *target) const;
};

class IdtEntry {
    public:
    uint32_t offset;
    uint16_t selector;
    uint8_t type_attr;

    void encode(uint8_t *target) const;
};
