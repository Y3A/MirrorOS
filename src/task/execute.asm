section .asm

global scheduler_run_thread
global scheduler_load_user_segments
global scheduler_load_kernel_segments

GDT_USER_DATA   EQU  0b100011
GDT_KERNEL_DATA EQU  0b010000

scheduler_run_thread:
    ; https://wiki.osdev.org/Getting_to_Ring_3#iret_method
    mov ebp, esp
    ; set up stack and regs from REGISTERS struct
    mov ebx, [ebp+4]
    ; ebx is now registers struct
    ; push ss
    push dword [ebx+40]
    ; push esp
    push dword [ebx+32]
    ; push eflags, enable interrupts
    pushf
    pop eax
    or eax, 0x200
    push eax
    ; push cs
    push dword [ebx+36]
    ; push eip
    push dword [ebx+24]
    ; set segment registers to ss
    mov ax, [ebx+40]
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ; set GPRs
    push ebx
    call restore_general_purpose_registers
    ; run in userland
    add esp, 4
    iretd

restore_general_purpose_registers:
    push ebp
    mov ebp, esp
    ; restore GPRs from REGISTERS struct
    mov ebx, [ebp+8]
    ; ebx is now registers struct
    mov eax, [ebx]
    mov ebp, [ebx+28]
    mov ecx, [ebx+8]
    mov edx, [ebx+12]
    mov esi, [ebx+16]
    mov edi, [ebx+20]
    mov ebx, [ebx+4]
    ; avoid clobbering ebp
    add esp, 4
    ret

scheduler_load_user_segments:
    mov ax, GDT_USER_DATA
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ret

scheduler_load_kernel_segments:
    mov ax, GDT_KERNEL_DATA
    mov ds, ax
    mov es, ax
    mov gs, ax
    mov fs, ax
    ret