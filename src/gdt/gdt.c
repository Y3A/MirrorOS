#include "gdt/gdt.h"

VOID gdt_readable_to_gdt_entry(IN OUT PGDT_ENTRY gdt, IN PGDT_READABLE gdt_readable, IN DWORD count)
{
    for (int i = 0; i < count; i++) {
        gdt[i].limit_low = gdt_readable[i].limit & 0xffff;
        gdt[i].base_low = gdt_readable[i].base & 0xffffff;
        *(PBYTE)((DWORD)&gdt[i] + (DWORD)5) = gdt_readable[i].flags; // set all at once
        gdt[i].limit_high = (gdt_readable[i].limit >> 16) & 0xff;
        gdt[i].reserved = 0;
        gdt[i].long_mode = 0;
        gdt[i].big = 1;  // it's 32 bits
        gdt[i].gran = 1; // 4KB page addressing
        gdt[i].base_high = 0;
    }
}