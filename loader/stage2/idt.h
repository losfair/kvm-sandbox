#pragma once

struct __attribute__((packed)) IdtEntry {
    uint16_t offset_1; // offset bits 0..15
    uint16_t selector; // a code segment selector in GDT or LDT
    uint8_t ist;       // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.
    uint8_t type_attr; // type and attributes
    uint16_t offset_2; // offset bits 16..31
    uint32_t offset_3; // offset bits 32..63
    uint32_t zero;     // reserved
};

static void idt_write(volatile struct IdtEntry *entry, uint64_t offset, uint8_t type_attr) {
    entry->offset_1 = (uint16_t) ((offset >> 0) & 0xffff);
    entry->offset_2 = (uint16_t) ((offset >> 16) & 0xffff);
    entry->offset_3 = (uint32_t) (offset >> 32);
    entry->selector = 0x08;
    entry->ist = 0;
    entry->type_attr = type_attr;
    entry->zero = 0;
}
