cli

lgdt [0x3000]
mov eax, cr0 
or al, 1       ; set PE (Protection Enable) bit in CR0 (Control Register 0)
mov cr0, eax
jmp 08h:protected_mode

[bits 32]

protected_mode:

; initialize segment registers
mov ax, 10h
mov ds, ax
mov ss, ax

; initialize paging
mov eax, 0x15000
mov cr3, eax

mov eax, cr0
or eax, 0x80000000
mov cr0, eax

lgdt [0xc0003000]
lidt [0xc0014000]

mov ax, 0x2b
ltr ax

mov eax, 0x30
mov gs, eax

jmp 0xc0001000
