#ifndef IDT_H
#define IDT_H

#include "types.h"

#define IDT_DIV_BY_ZERO 0x0
#define IDT_KEY_PRESS   0x21

struct _IDT_ENTRY
/* https://wiki.osdev.org/Interrupt_Descriptor_Table#Gate_Descriptor_2 */
{
    WORD offset_low; // Offset bits 0 to 15
    WORD selector;
    BYTE zero;
    BYTE type_attr; // Type and Attributes (P, DPL, S)
    WORD offset_high; // Offset bits 16 to 31
} __attribute__((packed));

struct _IDT_DESC
/* https://wiki.osdev.org/Interrupt_Descriptor_Table#IDTR */
{
    WORD limit; // Size of IDT minus 1
    DWORD base; // Starting address of IDT
} __attribute__((packed));

typedef struct _IDT_ENTRY IDT_ENTRY, *PIDT_ENTRY;
typedef struct _IDT_DESC IDT_DESC, *PIDT_DESC;

VOID idt_init(VOID);
VOID idt_set(INT interrupt_number, PVOID isr_addr);
VOID idt_load(PIDT_DESC idt_desc_addr);

/* Actual Interrupts */
VOID idt_void(VOID);
VOID idt_div_by_zero(VOID);
VOID idt_key_press(VOID);

#endif