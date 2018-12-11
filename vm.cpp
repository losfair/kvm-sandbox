#include "vm.hpp"
#include "gdt.hpp"
#include "error.hpp"
#include "layout.hpp"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/kvm.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdexcept>
#include <sys/mman.h>
#include <string.h>

#define DEBUG_PRINT

static uint64_t translate_address(int vcpu_fd, uint64_t vaddr) {
    kvm_translation trans;
    trans.linear_address = vaddr;
    chk_result(ioctl(vcpu_fd, KVM_TRANSLATE, &trans));
    return trans.physical_address;
}

VirtualMachine::VirtualMachine(VMConfig new_config) {
    config = new_config;

    kvm_device_fd = chk_result(open("/dev/kvm", O_RDWR));

    vcpu_mmap_size = chk_result(ioctl(kvm_device_fd, KVM_GET_VCPU_MMAP_SIZE, 0));

    vm_fd = ioctl(kvm_device_fd, KVM_CREATE_VM, 0);
    if(vm_fd < 0) {
        close(kvm_device_fd);
        throw std::runtime_error("unable to create vm");
    }

    max_vcpus = chk_result(ioctl(vm_fd, KVM_CHECK_EXTENSION, KVM_CAP_NR_VCPUS));
    if(!max_vcpus) {
        fprintf(stderr, "unable to get max vcpus, defaulting to 4\n");
        max_vcpus = 4;
    }

    guest_mem = (uint8_t *) mmap(NULL, config.mem_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if(!guest_mem) {
        close(vm_fd);
        close(kvm_device_fd);
        throw std::runtime_error("unable to initialize guest mem");
    }

    memset(guest_mem, 0, config.mem_size);

    kvm_userspace_memory_region region = {
        .slot = 0,
        .flags = 0,
        .guest_phys_addr = PHYS_OFFSET,
        .memory_size = config.mem_size,
        .userspace_addr = reinterpret_cast<uint64_t>(guest_mem),
    };
    if(ioctl(vm_fd, KVM_SET_USER_MEMORY_REGION, &region) < 0) {
        close(vm_fd);
        close(kvm_device_fd);
        throw std::runtime_error("unable to set user memory region");
    }
}

void VirtualMachine::write_memory(const uint8_t *data, size_t len, size_t offset) {
    offset -= PHYS_OFFSET;
    check_guest_mem_bounds(offset, len);
    memcpy(guest_mem + offset, data, len);
}

void VirtualMachine::write_gdt_entry(const GdtEntry& entry, size_t offset) {
    offset -= PHYS_OFFSET;
    check_guest_mem_bounds(offset, GDT_ENTRY_SIZE);
    entry.encode(guest_mem + offset);
}

void VirtualMachine::write_idt_entry(const IdtEntry& entry, size_t offset) {
    offset -= PHYS_OFFSET;
    check_guest_mem_bounds(offset, IDT_ENTRY_SIZE);
    entry.encode(guest_mem + offset);
}

void VirtualMachine::check_guest_mem_bounds(uint32_t offset, uint32_t len) {
    if(offset + len < offset || offset + len > config.mem_size) {
        throw std::runtime_error("memory access out of bounds");
    }
}

VirtualMachine::~VirtualMachine() {
    close(vm_fd);
    close(kvm_device_fd);
}

void VirtualMachine::stop_gracefully() {
    std::lock_guard<std::mutex> vcpu_lg(vcpu_mutex);

    for(auto& vcpu : vcpus) {
        vcpu -> thread_handle.join();
    }
}

void VirtualMachine::process_io_in(uint16_t port, uint8_t *value, size_t size, int vcpu_fd) {
    throw std::runtime_error("not implemented");
}

void VirtualMachine::process_io_out(uint16_t port, uint8_t *value, size_t size, int vcpu_fd) {
    switch(port) {
        
        case 0x3f01: // debug print
#ifdef DEBUG_PRINT
            if(size != 4) {
                throw std::runtime_error("size must be 4");
            } else {
                uint32_t phys_addr = translate_address(vcpu_fd, * (uint32_t *) value);
                size_t start_offset = phys_addr - PHYS_OFFSET;
                size_t offset = start_offset;
                while(offset < config.mem_size) {
                    if(guest_mem[offset] == 0) {
                        break;
                    }
                    offset++;
                }
                if(offset >= config.mem_size) {
                    throw std::runtime_error("string overflow");
                } else {
                    printf("%.*s\n", (int) (offset - start_offset + 1), &guest_mem[start_offset]);
                }
            }
#endif
            break;
        case 0x3f03: { // syscall forwarding
            kvm_regs regs;
            chk_result(ioctl(vcpu_fd, KVM_GET_REGS, &regs));
            uint64_t rdx_offset = translate_address(vcpu_fd, regs.rsp) - PHYS_OFFSET;
            check_guest_mem_bounds(rdx_offset, sizeof(uint64_t));
            regs.rdx = * (uint64_t *) &guest_mem[rdx_offset];

            /*printf("[+] SYSCALL\n");
            printf("rip = 0x%llx, rax = 0x%llx, rbx = 0x%llx, rcx = 0x%llx\n", regs.rip, regs.rax, regs.rbx, regs.rcx);
            printf("rdx = 0x%llx, rdi = 0x%llx, rsi = 0x%llx\n", regs.rdx, regs.rdi, regs.rsi);
            printf("rsp = 0x%llx\n", regs.rsp);*/

            switch(regs.rax) {
                case 0x01: { // write
                    uint64_t payload_offset = translate_address(vcpu_fd, regs.rsi) - PHYS_OFFSET;
                    check_guest_mem_bounds(payload_offset, regs.rdx);
                    regs.rax = syscall(0x01, regs.rdi, &guest_mem[payload_offset], regs.rdx);
                    break;
                }
                default:
                    throw std::runtime_error("unsupported syscall");
            }
            chk_result(ioctl(vcpu_fd, KVM_SET_REGS, &regs));
            break;
        }
        default:
            fprintf(stderr, "unsupported port: 0x%x\n", port);
            throw std::runtime_error("unsupported port");
    }
}

void VirtualMachine::put_vcpu(RemoteVCpu *vcpu) {
    std::lock_guard<std::mutex> vcpu_lg(vcpu_mutex);

    if(vcpus.size() >= max_vcpus) {
        throw std::runtime_error("too many vcpus");
    }

    vcpus.push_back(std::unique_ptr<RemoteVCpu>(vcpu));
}

RemoteVCpu::RemoteVCpu(VirtualMachine& vm, int id) {
    state = std::shared_ptr<VCpuState>(new VCpuState());
    std::shared_ptr<VCpuState> local_state = state;

    thread_handle = std::thread([&vm, local_state, id]() {
        int vcpu_fd = chk_result(ioctl(vm.vm_fd, KVM_CREATE_VCPU, id));

        kvm_run *run = (kvm_run *) mmap(0, vm.vcpu_mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, vcpu_fd, 0);
        if(!run) {
            throw std::runtime_error("unable to map vcpu");
        }

        kvm_sregs sregs;
        chk_result(ioctl(vcpu_fd, KVM_GET_SREGS, &sregs));
        sregs.cs.base = 0;
        sregs.cs.selector = 0;
        chk_result(ioctl(vcpu_fd, KVM_SET_SREGS, &sregs));

        kvm_regs regs;
        memset(&regs, 0, sizeof(regs));
        regs.rip = PHYS_OFFSET;
        chk_result(ioctl(vcpu_fd, KVM_SET_REGS, &regs));

        bool should_run = true;

        while(should_run) {
            should_run = false;
            chk_result(ioctl(vcpu_fd, KVM_RUN, 0));

            switch(run -> exit_reason) {
                case KVM_EXIT_HLT:
                    printf("halt\n");
                    break;
                case KVM_EXIT_IO:
                    //printf("io direction=%d size=%d port=0x%x count=0x%x\n", run->io.direction, run->io.size, run->io.port, run->io.count);
                    if(run->io.direction == KVM_EXIT_IO_OUT) {
                        vm.process_io_out(
                            run->io.port,
                            ((uint8_t *) run) + run->io.data_offset,
                            run->io.size,
                            vcpu_fd
                        );
                    } else {
                        vm.process_io_in(
                            run->io.port,
                            ((uint8_t *) run) + run->io.data_offset,
                            run->io.size,
                            vcpu_fd
                        );
                    }
                    should_run = true;
                    break;
                case KVM_EXIT_FAIL_ENTRY:
                    printf("fail retry\n");
                    break;
                case KVM_EXIT_INTERNAL_ERROR:
                    printf("internal error\n");
                    break;
                default:
                    printf("exit reason = 0x%x\n", run -> exit_reason);
            }
        }

        chk_result(ioctl(vcpu_fd, KVM_GET_SREGS, &sregs));
        printf("cr0 = 0x%llx, cr2 = 0x%llx, cr3 = 0x%llx, cr4 = 0x%llx\n", sregs.cr0, sregs.cr2, sregs.cr3, sregs.cr4);

        chk_result(ioctl(vcpu_fd, KVM_GET_REGS, &regs));
        printf("rip = 0x%llx, rax = 0x%llx, rbx = 0x%llx, rcx = 0x%llx\n", regs.rip, regs.rax, regs.rbx, regs.rcx);
        printf("rdx = 0x%llx, rdi = 0x%llx, rsi = 0x%llx\n", regs.rdx, regs.rdi, regs.rsi);
        printf("rsp = 0x%llx\n", regs.rsp);

        close(vcpu_fd);
    });

}

RemoteVCpu::~RemoteVCpu() {

}