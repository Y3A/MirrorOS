section .asm

global idt_0
global idt_33
global idt_void
extern terminal_warn
extern terminal_print

idt_void:
cli
pushad

mov eax, 0x20
out 0x20, eax

popad
sti
iret

idt_33:
cli
pushad

push keyb_alert
call terminal_print
add esp, 4

mov eax, 0x20
out 0x20, eax

popad
sti
iret

idt_0:
cli
pushad

push warning
call terminal_warn
add esp, 4

mov eax, 0x20
out 0x20, eax

popad
sti
iret

section .rodata
warning: db "Division by zero!", 0xa, 0
keyb_alert: db "Key Pressed!", 0xa, 0