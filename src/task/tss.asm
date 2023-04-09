section .asm

global tss_load_tss

tss_load_tss:
    push ebp
    mov ebp, esp
    ; https://wiki.osdev.org/Task_State_Segment#TSS_in_software_multitasking
    mov ax, [ebp+8]
    ltr ax
    leave
    ret