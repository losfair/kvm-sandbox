.globl init_regs
init_regs:
mov $0, %rax
mov %rax, %ds
mov %rax, %es
mov %rax, %ss
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
