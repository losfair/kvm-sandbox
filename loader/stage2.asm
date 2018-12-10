[bits 32]

org 0x100000

mov esp, 0x2000000
jmp 0x100200

times 512-($-$$) db 0
