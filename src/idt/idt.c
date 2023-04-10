#include "config.h"
#include "kernel.h"
#include "types.h"
#include "gdt/gdt.h"
#include "idt/idt.h"
#include "memory/memory.h"

IDT_ENTRY idt[TOTAL_INTERRUPTS]; // include exceptions and irqs

VOID idt_init(VOID)
/*
    * https://wiki.osdev.org/Interrupt_Descriptor_Table#IDTR
    * Initializing IDT Entries and IDT Descriptor, then loading it
*/
{
    IDT_DESC idt_desc;

    unbound_memset(idt, 0, sizeof(idt));
    idt_desc.limit = sizeof(idt)-1;
    idt_desc.base = (DWORD)idt;

    for (INT i = 0; i < TOTAL_INTERRUPTS; i++)
        idt_set(i, idt_void);

    idt_set(IDT_DIV_BY_ZERO, idt_div_by_zero);
    idt_set(IDT_KEY_PRESS, idt_key_press);

    idt_load(&idt_desc);
}

VOID idt_load(PIDT_DESC idt_desc_addr)
{
    __asm__
    (
        ".intel_syntax noprefix;"

        "lidt [%0];"

        ".att_syntax;"
        :
        : "r" (idt_desc_addr)
    );
}

VOID idt_set(INT interrupt_number, PVOID isr_addr)
/*
    * https://wiki.osdev.org/Interrupt_Descriptor_Table#Gate_Descriptor_2
    * Mapping ISRs to an interrupt number to handle it
*/
{
    // isr_addr for address of interrupt service routines
    PIDT_ENTRY entry = &idt[interrupt_number];
    entry->offset_low = (DWORD)isr_addr & 0x0000ffff;
    entry->selector = GDT_KERNEL_CS;
    entry->zero = 0;
    entry->type_attr = 0b11101110;
    entry->offset_high = (DWORD)isr_addr >> 16;
}