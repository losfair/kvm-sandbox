#include "gdt.hpp"
#include <stdexcept>
#include <stdio.h>

void GdtEntry::encode(uint8_t *target) const {
    size_t limit = this -> limit;

    // Check the limit to make sure that it can be encoded
    if ((limit > 65536) && (limit & 0xFFF) != 0xFFF) {
        throw std::runtime_error("invalid limit");
    }
    if (limit > 65536) {
        // Adjust granularity if required
        limit = limit >> 12;
        target[6] = 0xC0;
    } else {
        target[6] = 0x40;
    }
 
    // Encode the limit
    target[0] = limit & 0xFF;
    target[1] = (limit >> 8) & 0xFF;
    target[6] |= (limit >> 16) & 0xF;
 
    // Encode the base 
    target[2] = base & 0xFF;
    target[3] = (base >> 8) & 0xFF;
    target[4] = (base >> 16) & 0xFF;
    target[7] = (base >> 24) & 0xFF;
 
    // And... Type
    target[5] = type;
}

void IdtEntry::encode(uint8_t *target) const {
    target[0] = offset & 0xFF;
    target[1] = (offset >> 8) & 0xFF;
    target[2] = selector & 0xFF;
    target[3] = (selector >> 8) & 0xFF;
    target[4] = 0;
    target[5] = type_attr;
    target[6] = (offset >> 16) & 0xFF;
    target[7] = (offset >> 24) & 0xFF;
}