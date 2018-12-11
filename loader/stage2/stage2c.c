#include <stdint.h>
#include <string.h>
#include "offsets.h"
#include "idt.h"
#include "gdt.h"
#include "kheap.h"
#include "page.h"

void init_regs();
void init_syscall(void (*handler)());
void handle_div_by_zero();
void handle_page_fault();
void handle_double_fault();
void handle_gpf();
void flush_tlb();
void enter_userspace(void *user_code);

void do_halt() {
    asm volatile("hlt" : : : "memory");
}

struct _unused_syscall_frame {};

void __attribute__((interrupt)) on_syscall(struct _unused_syscall_frame *_frame) {
    asm volatile(
        "push %rdx\n"
        "mov $0x3f03, %dx\n"
        "out %ax, (%dx)\n"
        "pop %rdx\n"
    );
    //do_halt();
    while(1) {}
}

struct interrupt_frame {
    void *rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
};

void __attribute__((section(".loader_start"))) _loader_start() {
    uint64_t i;

    // reload gdtr with paged address
    struct __attribute__((packed)) {
        uint16_t size;
        uint64_t offset;
    } gdtr;

    gdtr.size = GDT_ENTRY_COUNT * sizeof(uint64_t) - 1;
    gdtr.offset = (uint64_t) (uintptr_t) gdt;

    asm volatile(
        "lgdt (%0)" : : "r" (&gdtr) : "memory"
    );

    // init ds, es, ss
    init_regs();

    init_syscall(on_syscall);

    // remove identity mapping
    pml4t[0] = 0;
    flush_tlb();

    // setup IDT
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

    // setup tss

    memset((char *) tss, 0, sizeof(struct Tss));
    tss->rsp0 = FIXED_DATA_OFFSET; // stack
    asm volatile("push %%rax; mov $0x23, %%ax; ltr %%ax; pop %%rax" : : : "memory");

    uint64_t *user_pdpt = (uint64_t *) kmalloc(0x1000);
    uint64_t *user_pdt = (uint64_t *) kmalloc(0x1000);
    for(i = 0; i < 512; i++) {
        user_pdpt[i] = 0;
        user_pdt[i] = (STAGE3_OFFSET - LAYOUT_BASE_OFFSET + i * 2 * 1024 * 1024) | (PAGE_PRESENT | PAGE_RW | PAGE_USER | PAGE_LARGE);
    }
    user_pdpt[1] = ((uint64_t) user_pdt - LAYOUT_BASE_OFFSET) | (PAGE_PRESENT | PAGE_RW | PAGE_USER); // 1GB offset
    pml4t[0] = ((uint64_t) user_pdpt - LAYOUT_BASE_OFFSET) | (PAGE_PRESENT | PAGE_RW | PAGE_USER);
    flush_tlb();

    asm volatile("sti" : : : "memory");
    enter_userspace((void *) 0x40000000);

    //*(uint32_t *) (0xdeadbeef) = 42;
    while(1) {}
    do_halt();
}
