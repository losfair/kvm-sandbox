#include <stdint.h>
#include <string.h>
#include "offsets.h"
#include "idt.h"
#include "gdt.h"
#include "page.h"
#include "proc.h"

void init_regs();
void init_syscall(void (*handler)());
void handle_div_by_zero();
void handle_page_fault();
void handle_double_fault();
void handle_gpf();
void flush_tlb();
void enter_userspace(void *user_code, void *user_stack);

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
    uint64_t i, j;

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
    init_phys_page_map((void *) USER_OFFSET);

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

    struct Proc *proc;
    if(proc_new(&proc)) {
        do_halt();
    }
    for(i = 0; i < 2048; i++) {// 8MB
        uint64_t addr;
        if(phys_alloc(&addr)) do_halt();
        if(proc_map_vm(proc, (void *) (0x40000000ull + i * 4096), addr)) do_halt();

        uint8_t *pg_mem;
        if(proc_get_vm_map(proc, (void **) &pg_mem, (void *) (0x40000000ull + i * 4096))) {
            do_halt();
        }

        for(j = 0; j < 4096; j++) {
            pg_mem[j] = * (uint8_t *) (STAGE3_OFFSET + i * 4096 + j);
        }
    }
    proc_load_table(proc);

    asm volatile("sti" : : : "memory");
    enter_userspace((void *) 0x40000000, (void *) 0x40800000);

    //*(uint32_t *) (0xdeadbeef) = 42;
    while(1) {}
    do_halt();
}
