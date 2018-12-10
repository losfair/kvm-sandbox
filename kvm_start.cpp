#include "vm.hpp"
#include "gdt.hpp"
#include "layout.hpp"
#include <vector>
#include <stdio.h>
#include <string.h>

#define READ_BUFFER_SIZE 4096

struct pd_entry {
    uint32_t address;
    bool size;
    bool accessed;
    bool cache_disabled;
    bool write_through;
    bool user_or_supervisor;
    bool read_or_write;
    bool present;

    uint32_t encode() {
        uint32_t ret = 0;
        ret |= (address >> 12) << 12;
        ret |= ((uint32_t) size) << 7;
        ret |= ((uint32_t) accessed) << 5;
        ret |= ((uint32_t) cache_disabled) << 4;
        ret |= ((uint32_t) write_through) << 3;
        ret |= ((uint32_t) user_or_supervisor) << 2;
        ret |= ((uint32_t) read_or_write) << 1;
        ret |= ((uint32_t) present) << 0;
        return ret;
    }
};

struct pt_entry {
    uint32_t address;
    bool global;
    bool dirty;
    bool accessed;
    bool cache_disabled;
    bool write_through;
    bool user_or_supervisor;
    bool read_or_write;
    bool present;

    uint32_t encode() {
        uint32_t ret = 0;
        ret |= (address >> 12) << 12;
        ret |= ((uint32_t) global) << 8;
        ret |= ((uint32_t) dirty) << 6;
        ret |= ((uint32_t) accessed) << 5;
        ret |= ((uint32_t) cache_disabled) << 4;
        ret |= ((uint32_t) write_through) << 3;
        ret |= ((uint32_t) user_or_supervisor) << 2;
        ret |= ((uint32_t) read_or_write) << 1;
        ret |= ((uint32_t) present) << 0;
        return ret;
    }
};

struct __attribute__((packed)) tss_entry
{
   uint32_t prev_tss;   // The previous TSS - if we used hardware task switching this would form a linked list.
   uint32_t esp0;       // The stack pointer to load when we change to kernel mode.
   uint32_t ss0;        // The stack segment to load when we change to kernel mode.
   uint32_t esp1;       // everything below here is unused now.. 
   uint32_t ss1;
   uint32_t esp2;
   uint32_t ss2;
   uint32_t cr3;
   uint32_t eip;
   uint32_t eflags;
   uint32_t eax;
   uint32_t ecx;
   uint32_t edx;
   uint32_t ebx;
   uint32_t esp;
   uint32_t ebp;
   uint32_t esi;
   uint32_t edi;
   uint32_t es;         
   uint32_t cs;        
   uint32_t ss;        
   uint32_t ds;        
   uint32_t fs;       
   uint32_t gs;         
   uint32_t ldt;      
   uint16_t trap;
   uint16_t iomap_base;
};

void read_to_memory(VirtualMachine& vm, const char *path, size_t offset) {
    FILE *f = fopen(path, "r");
    if(!f) {
        throw std::runtime_error("unable to open file\n");
    }

    uint8_t *read_buffer = (uint8_t *) malloc(READ_BUFFER_SIZE);
    while(!feof(f)) {
        size_t nb_read = fread(read_buffer, 1, READ_BUFFER_SIZE, f);
        vm.write_memory(read_buffer, nb_read, offset);
        offset += nb_read;
    }
    free(read_buffer);
    fclose(f);
}

int main(int argc, const char *argv[]) {
    if(argc != 4) {
        fprintf(stderr, "expecting exactly 3 arguments\n");
        return 1;
    }

    VirtualMachine vm(VMConfig {
        mem_size: 0x10000000 // 256M
    });

    read_to_memory(vm, argv[1], STAGE1_OFFSET);
    read_to_memory(vm, argv[2], STAGE2_OFFSET);
    read_to_memory(vm, argv[3], STAGE3_OFFSET);

    std::vector<GdtEntry> entries = {
        {
            base: 0,
            limit: 0,
            type: 0,
        },
        {
            base: 0,
            limit: 0xffffffff,
            type: 0x9A, // kernel code
        },
        {
            base: 0,
            limit: 0xffffffff,
            type: 0x92, // kernel data
        },
        {
            base: 0,
            limit: 0xffffffff,
            type: 0xFA, // user code
        },
        {
            base: 0,
            limit: 0xffffffff,
            type: 0xF2, // user data
        },
        {
            base: KERNEL_BASE + TSS_BASE,
            limit: sizeof(tss_entry),
            type: 0xE9 // TSS
        },
        {
            base: GS_DATA_BASE,
            limit: 0x0000ffff,
            type: 0xF2, // gs?
        },
    };
    for(size_t i = 0; i < entries.size(); i++) {
        vm.write_gdt_entry(entries[i], GDT_BASE + GDT_ENTRY_SIZE * i);
    }

    struct __attribute__((packed)) {
        uint16_t size;
        uint32_t offset;
    } gdtr {
        size: (uint16_t) (entries.size() * GDT_ENTRY_SIZE - 1),
        offset: KERNEL_BASE + GDT_BASE,
    };

    vm.write_memory((uint8_t *) &gdtr, sizeof(gdtr), GDTR_OFFSET);

    struct __attribute__((packed)) {
        uint16_t size;
        uint32_t offset;
    } idtr {
        size: 256 * IDT_ENTRY_SIZE - 1,
        offset: KERNEL_BASE + IDT_BASE,
    };
    vm.write_memory((uint8_t *) &idtr, sizeof(idtr), IDTR_OFFSET);

    {
        tss_entry tss;
        memset(&tss, 0, sizeof(tss));
        tss.ss0 = 0x10;
        tss.esp0 = KERNEL_BASE + STAGE3_OFFSET; // kernel stack is put before stage 3
        vm.write_memory((uint8_t *) &tss, sizeof(tss), TSS_BASE);
    }

    for(size_t i = 0; i < 1024; i++) {
        pd_entry de;
        memset(&de, 0, sizeof(pd_entry));

        de.address = PAGE_TABLE_BASE + i * 4096;
        de.read_or_write = 1;
        de.user_or_supervisor = 1;
        de.present = 1;

        uint32_t de_encoded = de.encode();
        vm.write_memory((uint8_t *) &de_encoded, 4, PAGE_DIRECTORY_BASE + i * 4);
        //printf("de = 0x%x\n", * (uint32_t *) &de_encoded);

        for(size_t j = 0; j < 1024; j++) {
            pt_entry te;
            memset(&te, 0, sizeof(pt_entry));

            size_t addr = (i * 1024 * 1024 + j * 1024) * 4;
            if(addr >= KERNEL_BASE) {
                te.address = addr - KERNEL_BASE;
                te.read_or_write = 1;
                te.present = 1;
            } else if(addr >= GS_DATA_BASE) {
                te.address = addr - GS_DATA_BASE + GS_DATA_OFFSET;
                te.read_or_write = 1;
                te.user_or_supervisor = 1;
                te.present = 1;
            } else if(addr >= USER_CODE_BASE) {
                te.address = addr - USER_CODE_BASE + STAGE3_OFFSET;
                te.read_or_write = 1;
                te.user_or_supervisor = 1;
                te.present = 1;
            } else if(addr >= USER_STACK_MIN) {
                te.address = addr - USER_STACK_MIN + STAGE3_STACK_OFFSET;
                te.read_or_write = 1;
                te.user_or_supervisor = 1;
                te.present = 1;
            } else if(addr == 0) {
                te.address = 0;
                te.read_or_write = 0; // read only
                te.present = 1;
            }
            uint32_t te_encoded = te.encode();
            vm.write_memory((uint8_t *) &te_encoded, 4, PAGE_TABLE_BASE + i * 4096 + j * 4);
        }
    }

    vm.put_vcpu(new RemoteVCpu(vm, 0));

    vm.stop_gracefully();

    return 0;
}
