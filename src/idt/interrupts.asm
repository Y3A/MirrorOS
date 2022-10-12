section .asm

PIC_MASTER EQU 0x20	; IO base address for master PIC
PICM_COMMAND EQU PIC_MASTER
PIC_SLAVE EQU 0xa0; slave
PICS_COMMAND EQU PIC_SLAVE
PIC_EOI EQU 0x20 ; End-of-interrupt command code

; if irq above or equal to 8(we remapped to 0x28)
; have to acknowledge slave also
; mov eax, PIC_EOI
; out PICS_COMMAND, eax

global idt_div_by_zero
global idt_key_press
global idt_void
extern vga_warn
extern vga_print

idt_void:
cli
pushad

mov eax, PIC_EOI
out PICM_COMMAND, eax

popad
sti
iret

idt_key_press:
cli
pushad

push keyb_alert
call vga_print
add esp, 4

mov eax, PIC_EOI
out PICM_COMMAND, eax

popad
sti
iret

idt_div_by_zero:
cli
pushad

push warning
call vga_warn
add esp, 4

mov eax, PIC_EOI
out PICM_COMMAND, eax

popad
sti
iret

section .rodata
warning: db "[-] Division by zero!", 0xa, 0
keyb_alert: db "[+] Key Pressed!", 0xa, 0