[bits 32]
org 0x08000000

mov ecx, esp
mov edx, body
call do_sysenter

body:

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
