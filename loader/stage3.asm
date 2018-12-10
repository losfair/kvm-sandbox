[bits 32]
org 0x08048000

mov ecx, esp
mov edx, body
call do_sysenter

body:

mov eax, 42
mov ebx, 1
mov ecx, 2
mov edx, 3
mov esi, 4
mov edi, 5
mov ebp, 6
int 0x80

mov edx, 0
mov eax, 0
mov edi, 0
div edi

inf_loop:
mov ecx, esp
mov edx, inf_loop
call do_sysenter

do_sysenter:
sysenter
