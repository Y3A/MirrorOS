BITS 32
global _start ; export label

extern kernel_main

GDT_CS equ 1000b ; index 1 in GDT, 0 for GDT and 00 for privileged
GDT_DATA equ 10000b ; index 2 in GDT, 0 for GDT and 00 for privileged

_start: ; load the data segment registers
mov ax, GDT_DATA
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax
mov ss, ax

; enable A20 line
in al, 0x92
or al, 2
out 0x92, al
;

; reinitialize the PIC controllers, giving them specified vector offsets
;   rather than 8h and 70h, as configured by default

PIC_MASTER EQU 0x20		; /* IO base address for master PIC */
PIC_SLAVE EQU 0xA0		; /* IO base address for slave PIC */
PICM_COMMAND EQU PIC_MASTER
PICM_DATA EQU (PIC_MASTER+1)
PICS_COMMAND EQU PIC_SLAVE
PICS_DATA EQU (PIC_SLAVE+1)
 
ICW1_ICW4 EQU 0x01	; ICW4 (not) needed
ICW1_SINGLE	EQU 0x02 ; Single (cascade) mode
ICW1_INTERVAL4 EQU 0x04		; Call address interval 4 (8)
ICW1_LEVEL EQU 0x08	; Level triggered (edge) mode
ICW1_INIT EQU 0x10	; Initialization - required!
 
ICW4_8086 EQU 0x01	; 8086/88 (MCS-80/85) mode
ICW4_AUTO EQU 0x02	; Auto (normal) EOI
ICW4_BUF_SLAVE EQU 0x08	; Buffered mode/slave
ICW4_BUF_MASTER EQU 0x0C	; Buffered mode/master
ICW4_SFNM EQU 0x10	; Special fully nested (not)

; remap master PIC

mov al, ICW1_INIT
or al, ICW1_ICW4 ; require ICW4
out PICM_COMMAND, al

mov al, 0x20 ; Map ISRs starting from interrupt 0x20
out PICM_DATA, al

mov al, ICW4_8086 ; x86 mode
out PICM_DATA, al

; Done remapping master PIC

mov ebp, 0x200000
mov esp, ebp

call kernel_main

; 16 byte alignment
; so we don't mess up the compiler generated asm
times 512-($ - $$) db 0 