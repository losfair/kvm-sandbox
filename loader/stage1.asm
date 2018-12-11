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
mov es, ax
mov ss, ax

; enable long mode
mov ecx, 0xC0000080          ; Set the C-register to 0xC0000080, which is the EFER MSR.
rdmsr                        ; Read from the model-specific register.
or eax, 1 << 8               ; Set the LM-bit which is the 9th bit (bit 8).
wrmsr                        ; Write to the model-specific register.

jmp 0x100000

times 512-($-$$) db 0
