#include "proc.h"
#include "page.h"
#include "offsets.h"

static inline uint64_t pentry_to_phys(uint64_t entry) {
    return entry & (~0xfffull);
}

static inline void * pentry_to_virt(uint64_t entry) {
    return (void *) (LAYOUT_BASE_OFFSET + pentry_to_phys(entry));
}

static inline void destroy_pt(uint64_t *table) {
    int i;
    for(i = 0; i < 512; i++) {
        if(table[i] & PAGE_PRESENT) {
            phys_free_from_virt(pentry_to_virt(table[i]));
        }
    }
    phys_free_from_virt(table);
}

static inline void destroy_pdt(uint64_t *table) {
    int i;
    for(i = 0; i < 512; i++) {
        if(table[i] & PAGE_PRESENT) {
            destroy_pt(pentry_to_virt(table[i]));
        }
    }
    phys_free_from_virt(table);
}

static inline void destroy_pdpt(uint64_t *table) {
    int i;
    for(i = 0; i < 512; i++) {
        if(table[i] & PAGE_PRESENT) {
            destroy_pdt(pentry_to_virt(table[i]));
        }
    }
    phys_free_from_virt(table);
}

static void destroy_pml4t(uint64_t *table) {
    int i;
    for(i = 0; i < 256; i++) {
        if(table[i] & PAGE_PRESENT) {
            destroy_pdpt(pentry_to_virt(table[i]));
        }
    }
    phys_free_from_virt(table);
}

static int alloc_ptable(uint64_t **out) {
    int i;
    if(phys_alloc_to_virt((void **) out)) {
        return -1;
    }
    for(i = 0; i < 512; i++) {
        (*out)[i] = 0;
    }
    return 0;
}

static inline int get_child_table(uint64_t **out, uint64_t *parent, uint64_t index) {
    if(parent[index] & PAGE_PRESENT) {
        *out = (uint64_t *) pentry_to_virt(parent[index]);
        return 0;
    } else {
        return -1;
    }
}

static inline int get_or_create_child_table(uint64_t **out, uint64_t *parent, uint64_t index, uint64_t flags) {
    if(parent[index] & PAGE_PRESENT) {
        *out = (uint64_t *) pentry_to_virt(parent[index]);
        return 0;
    } else {
        uint64_t *new_table;
        if(alloc_ptable(&new_table)) {
            return -1;
        }
        parent[index] = ((uint64_t) new_table - LAYOUT_BASE_OFFSET) | (flags | PAGE_PRESENT);
        *out = new_table;
        return 0;
    }
}

int proc_unmap_vm(struct Proc *proc, void *_virt) {
    uint64_t virt = (uint64_t) _virt;

    uint64_t pdpt_id = (virt >> PDPT_SHIFT) & 0x1ff,
            pdt_id = (virt >> PDT_SHIFT) & 0x1ff,
            pt_id = (virt >> PT_SHIFT) & 0x1ff,
            pte_id = (virt >> PTE_SHIFT) & 0x1ff;

    if(pdpt_id >= 256) {
        return -1; // cannot map kernel memory
    }

    uint64_t *t_pdpt, *t_pdt, *t_pt;

    if(
        get_child_table(&t_pdpt, proc->pml4t, pdpt_id)
        || get_child_table(&t_pdt, t_pdpt, pdt_id)
        || get_child_table(&t_pt, t_pdt, pt_id)
    ) {
        return -1;
    }

    t_pt[pte_id] = 0;
    return 0;
}

int proc_get_vm_map(struct Proc *proc, void **out, void *_virt) {
    uint64_t virt = (uint64_t) _virt;

    uint64_t pdpt_id = (virt >> PDPT_SHIFT) & 0x1ff,
            pdt_id = (virt >> PDT_SHIFT) & 0x1ff,
            pt_id = (virt >> PT_SHIFT) & 0x1ff,
            pte_id = (virt >> PTE_SHIFT) & 0x1ff;

    if(pdpt_id >= 256) {
        return -1; // cannot map kernel memory
    }

    uint64_t *t_pdpt, *t_pdt, *t_pt;

    if(
        get_child_table(&t_pdpt, proc->pml4t, pdpt_id)
        || get_child_table(&t_pdt, t_pdpt, pdt_id)
        || get_child_table(&t_pt, t_pdt, pt_id)
    ) {
        return -1;
    }

    *out = pentry_to_virt(t_pt[pte_id]);
    return 0;
}

int proc_map_vm(struct Proc *proc, void *_virt, uint64_t phys) {
    uint64_t i;
    uint64_t virt = (uint64_t) _virt;

    uint64_t pdpt_id = (virt >> PDPT_SHIFT) & 0x1ff,
            pdt_id = (virt >> PDT_SHIFT) & 0x1ff,
            pt_id = (virt >> PT_SHIFT) & 0x1ff,
            pte_id = (virt >> PTE_SHIFT) & 0x1ff;

    if(pdpt_id >= 256) {
        return -1; // cannot map kernel memory
    }

    uint64_t *t_pdpt, *t_pdt, *t_pt;

    if(
        get_or_create_child_table(&t_pdpt, proc->pml4t, pdpt_id, PAGE_PRESENT | PAGE_RW | PAGE_USER)
        || get_or_create_child_table(&t_pdt, t_pdpt, pdt_id, PAGE_PRESENT | PAGE_RW | PAGE_USER)
        || get_or_create_child_table(&t_pt, t_pdt, pt_id, PAGE_PRESENT | PAGE_RW | PAGE_USER)
    ) {
        return -1;
    }

    if(t_pt[pte_id] & PAGE_PRESENT) {
        phys_free(pentry_to_phys(t_pt[pte_id]));
    }

    t_pt[pte_id] = phys | (PAGE_PRESENT | PAGE_RW | PAGE_USER);

    // Clean the newly mapped page. 64-bit stores should be faster.
    uint64_t *new_virt = pentry_to_virt(t_pt[pte_id]);
    for(i = 0; i < 512; i++) new_virt[i] = 0;

    return 0;
}

void proc_load_table(struct Proc *proc) {
    asm volatile(
        "mov %0, %%rax\n"
        "mov %%rax, %%cr3\n"
        : : "r" ((uint64_t) proc -> pml4t - LAYOUT_BASE_OFFSET): "memory"
    );
}

int proc_new(struct Proc **proc) {
    if(phys_alloc_to_virt((void **) proc)) {
        return -1;
    }

    if(alloc_ptable(&(*proc)->pml4t)) {
        phys_free_from_virt(*proc);
        return -1;
    }

    (*proc)->pml4t[256] = ((uint64_t) kernel_pdpt - LAYOUT_BASE_OFFSET) | (PAGE_PRESENT | PAGE_RW);

    return 0;
}

void proc_destroy(struct Proc *proc) {
    destroy_pml4t(proc->pml4t);
    phys_free_from_virt(proc);
}
