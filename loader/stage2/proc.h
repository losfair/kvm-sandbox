#pragma once

#include <stdint.h>

struct Proc {
    uint64_t *pml4t;
};

int proc_unmap_vm(struct Proc *proc, void *_virt);
int proc_get_vm_map(struct Proc *proc, void **out, void *_virt);
int proc_map_vm(struct Proc *proc, void *_virt, uint64_t phys);
void proc_load_table(struct Proc *proc);
int proc_new(struct Proc **proc);
void proc_destroy(struct Proc *proc);
