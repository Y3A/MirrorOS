#include "idt/idt.h"
#include "config.h"
#include "memory/memory.h"

struct IDT_ENTRY idt[TOTAL_INTERRUPTS];
struct IDT_DESC idt_desc;

#include "kernel.h"

void idt_init(void)
{
    memset(idt, 0, sizeof(idt));
    idt_desc.limit = sizeof(idt)-1;
    idt_desc.base = (uint32_t)idt;

    for (int i = 0; i < TOTAL_INTERRUPTS; i++)
        idt_set(i, idt_void);

    idt_set(0, idt_0);
    idt_set(0x21, idt_33);

    idt_load(&idt_desc);
}

void idt_load(struct IDT_DESC * idt_desc_addr)
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

void idt_set(int i, void * addr)
{
    // i for interrupt number, addr for address of interrupt service routines
    struct IDT_ENTRY * entry = &idt[i];
    entry->offset_low = (uint32_t)addr & 0x0000ffff;
    entry->selector = GDT_CS;
    entry->zero = 0;
    entry->type_attr = 0b11101110;
    entry->offset_high = (uint32_t)addr >> 16;
}