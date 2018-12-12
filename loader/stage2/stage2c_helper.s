.globl init_regs
init_regs:
mov $0, %rax
mov %rax, %ds
mov %rax, %es
mov %rax, %ss
ret

.globl init_syscall
init_syscall:
# IA32_KERNEL_GS_BASE
mov $0xC0000102, %ecx
mov $0xffff8000, %edx
mov $0x00001000, %eax
wrmsr

mov $0xffff800002000000, %rax
mov $0xffff800000001000, %rcx
mov %rax, (%rcx)

mov $0xC0000080, %ecx
rdmsr
or $1, %rax
wrmsr

mov $0xC0000082, %ecx
mov %rdi, %rdx
shr $32, %rdx
mov %rdi, %rax
wrmsr
ret

.globl handle_page_fault
handle_page_fault:
pop %rax # error code
mov $0x0e, %rax
pop %rbx # rip
pop %rcx
pop %rcx
pop %rcx # rsp
hlt

.globl handle_double_fault
handle_double_fault:
pop %rax # error code
mov $0x08, %rax
pop %rbx # rip
pop %rcx
pop %rcx
pop %rcx # rsp
hlt

.globl handle_gpf
handle_gpf:
pop %rdi # error code
mov $0x0d, %rax
pop %rbx # rip
pop %rcx
pop %rcx
pop %rcx # rsp
hlt

.globl handle_div_by_zero
handle_div_by_zero:
mov $0, %rax
hlt

.globl flush_tlb
flush_tlb:
mov %cr3, %rax
mov %rax, %cr3
ret

.globl enter_userspace
enter_userspace:
mov %rdi, %rcx
mov %rsi, %rsp
sysretq
