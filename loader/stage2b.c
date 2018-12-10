#include <stdint.h>
#include "../layout.hpp"

volatile uint64_t *pml4t = (uint64_t *) (FIXED_DATA_OFFSET + 0); // 512 entries
volatile uint64_t *kernel_pdpt = (uint64_t *) (FIXED_DATA_OFFSET + 0x1000);
volatile uint64_t *kernel_pdt = (uint64_t *) (FIXED_DATA_OFFSET + 0x2000);
volatile uint64_t *kernel_pt = (uint64_t *) (FIXED_DATA_OFFSET + 0x3000);

void do_halt() {
    asm volatile("hlt" : : : "memory");
}

void __attribute__((section(".loader_start"))) _loader_start() {
    uintptr_t i, j;

    for(i = 0; i < 512; i++) {
        pml4t[i] = 0;
        kernel_pdpt[i] = 0;
    }

    pml4t[0] = (uint64_t) (uintptr_t) kernel_pdpt | 0x03;
    pml4t[256] = (uint64_t) (uintptr_t) kernel_pdpt | 0x03;
    kernel_pdpt[0] = (uint64_t) (uintptr_t) kernel_pdt | 0x03;
    for(i = 0; i < 512; i++) {
        volatile uint64_t *current_pt = &kernel_pt[i * 512];
        kernel_pdt[i] = (uint64_t) (uintptr_t) current_pt | 0x03;
        for(j = 0; j < 512; j++) {
            current_pt[j] = (i * 512 * 4096 + j * 4096) | 0x03;
        }
    }

    asm volatile(
        "mov %0, %%eax\n"
        "mov %%eax, %%cr3\n"
        : : "r"(pml4t) : "memory"
    );

    asm volatile(
        "mov %%cr4, %%eax\n"
        "or $0x20, %%eax\n" // bit 5 (PAE)
        "mov %%eax, %%cr4\n"
        : : : "memory"
    );

    asm volatile(
        "mov $0xC0000080, %%ecx\n"
        "rdmsr\n"
        "or $0x100, %%eax\n"
        "wrmsr\n"
        "mov %%cr0, %%eax\n"
        "or $0x80000000, %%eax\n"
        "mov %%eax, %%cr0\n"
        : : : "memory"
    );
    do_halt();
}