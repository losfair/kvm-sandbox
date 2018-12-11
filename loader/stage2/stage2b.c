#include <stdint.h>
#include "offsets.h"
#include "gdt.h"
#include "page.h"

void __attribute__((section(".loader_start"))) _loader_start() {
    uintptr_t i, j;

    for(i = 0; i < 512; i++) {
        pml4t[i] = 0;
        kernel_pdpt[i] = 0;
    }

    pml4t[0] = (uint64_t) (uintptr_t) kernel_pdpt | (PAGE_PRESENT | PAGE_RW);
    pml4t[256] = (uint64_t) (uintptr_t) kernel_pdpt | (PAGE_PRESENT | PAGE_RW);
    kernel_pdpt[0] = (uint64_t) (uintptr_t) kernel_pdt | (PAGE_PRESENT | PAGE_RW);
    for(i = 0; i < 512; i++) {
        volatile uint64_t *current_pt = &kernel_pt[i * 512];
        kernel_pdt[i] = (uint64_t) (uintptr_t) current_pt | (PAGE_PRESENT | PAGE_RW);
        for(j = 0; j < 512; j++) {
            current_pt[j] = (i * 512 * 4096 + j * 4096) | (PAGE_PRESENT | PAGE_RW);
        }
    }

    struct __attribute__((packed)) {
        uint16_t size;
        uint32_t offset;
    } gdtr;

    gdtr.size = GDT_ENTRY_COUNT * sizeof(uint64_t) - 1;
    gdtr.offset = (uint32_t) (uintptr_t) gdt;

    gdt_setup_64();

    asm volatile(
        "push %1\n" // gdtr

        // set pml4t
        "mov %0, %%eax\n"
        "mov %%eax, %%cr3\n"

        // enable pae
        "mov %%cr4, %%eax\n"
        "or $0x20, %%eax\n" // bit 5 (PAE)
        "mov %%eax, %%cr4\n"

        // enable paging
        "mov %%cr0, %%eax\n"
        "or $0x80000000, %%eax\n"
        "mov %%eax, %%cr0\n"

        // get gdtr
        "pop %%eax\n"

        // load gdt
        "lgdt (%%eax)\n"

        // enter 64-bit mode
        "jmp $0x08, $0x00100100\n"
        : : "r"(pml4t), "r" (&gdtr) : "memory"
    );
}
