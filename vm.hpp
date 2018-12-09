#pragma once

#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include "gdt.hpp"

const size_t PHYS_OFFSET = 0x0;

class VirtualMachine;
class VMConfig;
class RemoteVCpu;
class VCpuState;

class VMConfig {
    public:
        size_t mem_size;
};

class VirtualMachine {
    private:
        int kvm_device_fd;
        int vm_fd;
        size_t max_vcpus;
        int vcpu_mmap_size;
        uint8_t *guest_mem;
        VMConfig config;

        std::mutex vcpu_mutex;
        std::vector<std::unique_ptr<RemoteVCpu>> vcpus;

    public:
        VirtualMachine(VMConfig config);
        virtual ~VirtualMachine();
        void stop_gracefully();
        void write_memory(const uint8_t *data, size_t len, size_t offset);
        void write_gdt_entry(const GdtEntry& entry, size_t offset);
        void write_idt_entry(const IdtEntry& entry, size_t offset);
        void process_io_in(uint16_t port, uint8_t *value, size_t size, int vcpu_fd);
        void process_io_out(uint16_t port, uint8_t *value, size_t size, int vcpu_fd);
        void put_vcpu(RemoteVCpu *vcpu);

    friend class RemoteVCpu;
};

class RemoteVCpu {
    private:
        std::thread thread_handle;
        std::shared_ptr<VCpuState> state;

    public:
        RemoteVCpu(VirtualMachine& vm, int id);
        RemoteVCpu(const RemoteVCpu& that) = delete;
        virtual ~RemoteVCpu();

    friend class VirtualMachine;
};

class VCpuState {

};