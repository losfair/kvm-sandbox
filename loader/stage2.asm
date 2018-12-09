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

sti

mov dx, 0x3f01 ;output
mov eax, hello_world_s
out (dx), eax

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

mov edx, 0
mov eax, 0
mov edi, 0
;div edi

mov ecx, 0x09000000
mov edx, 0x08000000
sysexit

hlt

add_interrupt:
push esi ; handler address
push edi ; interrupt id
mov dx, 0x3f02
mov eax, esp
out (dx), eax
pop eax
pop eax
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
call some_callee
sysexit

handle_double_fault:
hlt

handle_div_by_zero:
mov edi, div_by_zero_s
call debug_print
hlt

handle_gpf:
hlt

handle_page_fault:
hlt

some_callee:
ret

hello_world_s:
db "Hello, world!", 0

sysenter_s:
db "SYSENTER", 0

div_by_zero_s:
db "Divide by zero", 0
