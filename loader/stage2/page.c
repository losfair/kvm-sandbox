#include <stdint.h>
#include "offsets.h"
#include "page.h"

struct PhysicalPage {
    uint64_t phys_addr;
    struct PhysicalPage *next;
};

static struct PhysicalPage *phys_unused_begin, *phys_used_begin;

void init_phys_page_map(void *vaddr) {
    uint64_t i;
    struct PhysicalPage *phys_pages = (struct PhysicalPage *) vaddr;
    uint64_t num_user_pages = (uint64_t) MAX_USER_MEM_GB * 1024 * 1024 * 1024 / 4096;
    uint64_t phys_list_size = num_user_pages * sizeof(struct PhysicalPage);

    for(i = 0; i < num_user_pages; i++) {
        phys_pages[i].phys_addr = (uint64_t) vaddr + phys_list_size + i * 4096 - LAYOUT_BASE_OFFSET;
        phys_pages[i].next = &phys_pages[i + 1];
    }
    phys_pages[num_user_pages - 1].next = 0;

    phys_unused_begin = phys_pages;
    phys_used_begin = 0;
}

int phys_alloc(uint64_t *out) {
    struct PhysicalPage *page = phys_unused_begin;

    if(page) {
        *out = page -> phys_addr;
        phys_unused_begin = page -> next;
        page -> next = phys_used_begin;
        phys_used_begin = page;
        return 0;
    } else {
        return -1;
    }
}

void phys_free(uint64_t addr) {
    struct PhysicalPage *page = phys_used_begin;
    if(!page) while(1) {} // invalid

    page -> phys_addr = addr;

    phys_used_begin = page -> next;
    page -> next = phys_unused_begin;
    phys_unused_begin = page;
}

int phys_alloc_to_virt(void **out) {
    uint64_t phys;
    if(phys_alloc(&phys)) {
        return -1;
    }
    *out = (void *) (phys + LAYOUT_BASE_OFFSET);
    return 0;
}

void phys_free_from_virt(void *addr) {
    phys_free((uint64_t) addr - LAYOUT_BASE_OFFSET);
}
