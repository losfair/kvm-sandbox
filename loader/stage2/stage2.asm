[bits 32]

org 0x100000

mov esp, 0x2000000
jmp 0x100200

times 256-($-$$) db 0

[bits 64]
mov rax, 0xffff800000100800
push rax
mov rax, rsp
mov rsp, 0xffff800002000000
jmp [rax]

times 512-($-$$) db 0
