section .asm
global gdt_load

gdt_load:
    mov ax, [esp+0x8]
    mov [gdt_descriptor], ax
    mov eax, [esp+0x4]
    mov [gdt_descriptor+0x2], eax ; check boot.asm for explanation
    lgdt [gdt_descriptor]
    ret

section .data
gdt_descriptor:
    dw 0x0 ; size
    dd 0x0 ; gdt start