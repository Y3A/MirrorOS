#ifndef IDT_H
#define IDT_H

#include <stdint.h>

struct IDT_ENTRY
{
    uint16_t offset_low; // Offset bits 0 to 15
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr; // Type and Attributes (P, DPL, S)
    uint16_t offset_high; // Offset bits 16 to 31
} __attribute__((packed));

struct IDT_DESC
{
    uint16_t limit; // Size of IDT minus 1
    uint32_t base; // Starting address of IDT
} __attribute__((packed));

void idt_init(void);
void idt_set(int, void *);
void idt_load(struct IDT_DESC *);

//Actual Interrupts
void idt_void(void);
void idt_0(void);
void idt_33(void);

#endif