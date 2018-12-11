#define LONG_MODE_64

#include <stdint.h>
#include <string.h>
#include "offsets.h"
#include "idt.h"
#include "gdt.h"

void init_regs();
void handle_div_by_zero();
void handle_page_fault();
void handle_double_fault();
void handle_gpf();

void do_halt() {
    asm volatile("hlt" : : : "memory");
}

struct interrupt_frame {
    void *rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
};

void __attribute__((section(".loader_start"))) _loader_start() {
    struct __attribute__((packed)) {
        uint16_t size;
        uint64_t offset;
    } gdtr;

    gdtr.size = GDT_ENTRY_COUNT * sizeof(uint64_t) - 1;
    gdtr.offset = (uint64_t) (uintptr_t) gdt;

    asm volatile(
        "lgdt (%0)" : : "r" (&gdtr) : "memory"
    );

    init_regs();

    pml4t[0] = 0;
    asm volatile(
        "push %%rax\n"
        "mov %%cr3, %%rax\n"
        "mov %%rax, %%cr3\n"
        "pop %%rax"
        : : : "memory"
    );

    memset((char *) idt, 4096, 0);

    struct __attribute__((packed)) {
        uint16_t limit;
        uint64_t offset;
    } idtr;
    idtr.limit = 4095;
    idtr.offset = (uint64_t) (uintptr_t) idt;
    asm volatile (
        "lidt (%0)"
        : : "r" (&idtr) : "memory"
    );

    idt_write(&idt[0x00], (uintptr_t) handle_div_by_zero, 0x8F);
    idt_write(&idt[0x08], (uintptr_t) handle_double_fault, 0x8F);
    idt_write(&idt[0x0d], (uintptr_t) handle_gpf, 0x8F);
    idt_write(&idt[0x0e], (uintptr_t) handle_page_fault, 0x8F);

    memset((char *) tss, 0, sizeof(struct Tss));
    tss->rsp0 = FIXED_DATA_OFFSET; // stack
    asm volatile("push %%rax; mov $0x23, %%ax; ltr %%ax; pop %%rax" : : : "memory");

    asm volatile("sti" : : : "memory");

    //*(uint32_t *) (0xdeadbeef) = 42;
    //while(1) {}
    do_halt();
}
