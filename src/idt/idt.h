#ifndef IDT_H
#define IDT_H

#include "types.h"

#define TOTAL_INTERRUPTS 256

#define IDT_DIV_BY_ZERO 0x0
#define IDT_KEY_PRESS   0x21

#define PIC_MASTER      0x20 // IO base address for master PIC
#define PICM_COMMAND    PIC_MASTER
#define PIC_SLAVE       0xa0 // slave
#define PICS_COMMAND    PIC_SLAVE
#define PIC_EOI         0x20 // End - of - interrupt command code

// if irq above or equal to 8(we remapped to 0x28)
// have to acknowledge slave also
// https://wiki.osdev.org/8259_PIC#End_of_Interrupt
// mov eax, PIC_EOI
// out PICS_COMMAND, eax

struct _IDT_ENTRY
/* https://wiki.osdev.org/Interrupt_Descriptor_Table#Gate_Descriptor_2 */
{
    WORD offset_low; // Offset bits 0 to 15
    WORD selector;
    BYTE zero;
    BYTE type_attr; // Type and Attributes (P, DPL, S)
    WORD offset_high; // Offset bits 16 to 31
} PACKED;

struct _IDT_DESC
/* https://wiki.osdev.org/Interrupt_Descriptor_Table#IDTR */
{
    WORD limit; // Size of IDT minus 1
    DWORD base; // Starting address of IDT
} PACKED;

typedef struct _IDT_ENTRY IDT_ENTRY, *PIDT_ENTRY;
typedef struct _IDT_DESC IDT_DESC, *PIDT_DESC;

typedef struct
{
    DWORD edi;
    DWORD esi;
    DWORD ebp;
    DWORD reserved;
    DWORD ebx;
    DWORD edx;
    DWORD ecx;
    DWORD eax;
    DWORD ip;
    DWORD cs;
    DWORD flags;
    DWORD esp;
    DWORD ss;
} PACKED INTERRUPT_FRAME, *PINTERRUPT_FRAME;

VOID idt_init(VOID);
VOID idt_set(INT interrupt_number, PVOID isr_addr);
VOID idt_load(PIDT_DESC idt_desc_addr);

typedef VOID (*ISR)(PINTERRUPT_FRAME frame);
VOID idt_register_interrupt_callback(DWORD interrupt_no, ISR callback);
VOID interrupt_handler(DWORD interrupt_no, PINTERRUPT_FRAME frame);
VOID idt_complete_interrupt_master(VOID);
VOID idt_complete_interrupt_slave(VOID);

static void idt_key_pressed(PINTERRUPT_FRAME frame);

#endif