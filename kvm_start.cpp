#include "vm.hpp"
#include "gdt.hpp"
#include "layout.hpp"
#include <vector>
#include <stdio.h>
#include <string.h>

#define READ_BUFFER_SIZE 4096

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
    };
    for(size_t i = 0; i < entries.size(); i++) {
        vm.write_gdt_entry(entries[i], GDT_OFFSET + GDT_ENTRY_SIZE * i);
    }

    struct __attribute__((packed)) {
        uint16_t size;
        uint32_t offset;
    } gdtr {
        size: (uint16_t) (entries.size() * GDT_ENTRY_SIZE - 1),
        offset: GDT_OFFSET,
    };

    vm.write_memory((uint8_t *) &gdtr, sizeof(gdtr), GDTR_OFFSET);

    vm.put_vcpu(new RemoteVCpu(vm, 0));

    vm.stop_gracefully();

    return 0;
}
