section .asm

extern interrupt_handler

global isr_table

TOTAL_INTERRUPTS EQU 256
PIC_MASTER EQU 0x20	; IO base address for master PIC
PICM_COMMAND EQU PIC_MASTER
PIC_SLAVE EQU 0xa0; slave
PICS_COMMAND EQU PIC_SLAVE
PIC_EOI EQU 0x20 ; End-of-interrupt command code

; if irq above or equal to 8(we remapped to 0x28)
; have to acknowledge slave also
; mov eax, PIC_EOI
; out PICS_COMMAND, eax

; struct interrupt_frame
; {
;     uword_t ip;
;     uword_t cs;
;     uword_t flags;
;     uword_t sp;
;     uword_t ss;
; };

; interrupt frame already pushed to stack
; we also push GPRs

; define a macro called interrupts with 1 argument
%macro interrupts 1
    global int%1
    int%1:
        pushad
        push esp ; ptr to frame and GPRs
        push dword %1
        call interrupt_handler
        add esp, 8 ; cleanup
        popad
        iretd
%endmacro

; repeat times for all interrupts
%assign i 0
%rep TOTAL_INTERRUPTS
    interrupts i
%assign i i+1
%endrep

section .data

; build an isr array
%macro isr_entry 1
    dd int%1
%endmacro

isr_table:
%assign i 0
%rep TOTAL_INTERRUPTS
    isr_entry i
%assign i i+1
%endrep