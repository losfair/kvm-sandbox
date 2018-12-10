[bits 32]
org 0xc0001000

mov esp, 0xc1000000

mov edi, 0
mov esi, handle_div_by_zero
call add_interrupt

mov edi, 8
mov esi, handle_double_fault
call add_interrupt

mov edi, 0x0d
mov esi, handle_gpf
call add_interrupt

mov edi, 0x0e
mov esi, handle_page_fault
call add_interrupt

mov edi, 0x80
mov esi, handle_int_syscall
call add_interrupt_user

sti

mov edi, hello_world_s
call debug_print

mov edx, 0
mov eax, 0x08 ; ring 0 CS
mov ecx, 0x174
wrmsr
mov eax, 0xc1000000
mov ecx, 0x175 ; esp for sysenter
wrmsr
mov eax, handle_sysenter
mov ecx, 0x176 ; eip for sysenter
wrmsr

mov ecx, 0x09000000
mov edx, 0x08048000
sysexit

add_interrupt_with_type_attr:
push edx
push esi ; handler address
push edi ; interrupt id
mov dx, 0x3f02
mov eax, esp
out (dx), eax
pop edi
pop esi
pop edx
ret

add_interrupt:
push edx
mov edx, 0x8F
call add_interrupt_with_type_attr
pop edx
ret

add_interrupt_user:
push edx
mov edx, 0xEF
call add_interrupt_with_type_attr
pop edx
ret

debug_print:
push edx
mov dx, 0x3f01 ;output
mov eax, edi
out (dx), eax
pop edx
ret

handle_sysenter:
mov edi, sysenter_s
call debug_print
sysexit

handle_int_syscall:
push ebp
push edi
push esi
push edx
push ecx
push ebx
push eax
mov dx, 0x3f03
mov eax, esp
out (dx), eax
pop eax
pop ebx
pop ecx
pop edx
pop esi
pop edi
pop ebp
iret

handle_double_fault:
hlt

handle_div_by_zero:
mov edi, div_by_zero_s
call debug_print
hlt

handle_gpf:
mov edi, gpf_s
call debug_print
hlt

handle_page_fault:
mov edi, page_fault_s
call debug_print
mov ebx, [esp+4]
mov ecx, [esp+8]
hlt

hello_world_s:
db "Hello, world!", 0

sysenter_s:
db "SYSENTER", 0

page_fault_s:
db "Page fault", 0

int_syscall_s:
db "system call by interrupt", 0

div_by_zero_s:
db "Divide by zero", 0

gpf_s:
db "General protection fault", 0