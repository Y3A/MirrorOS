#ifndef GDT_H
#define GDT_H

#include "types.h"

// https://wiki.osdev.org/Getting_to_Ring_3#GDT
typedef struct
{
    DWORD limit_low : 16;
    DWORD base_low : 24;
    DWORD accessed : 1;
    DWORD read_write : 1;             // readable for code, writable for data
    DWORD conforming_expand_down : 1; // conforming for code, expand down for data
    DWORD code : 1;                   // 1 for code, 0 for data
    DWORD code_data_segment : 1;      // should be 1 for everything but TSS and LDT
    DWORD DPL : 2;                    // privilege level
    DWORD present : 1;
    DWORD limit_high : 4;
    DWORD reserved : 1; // only used in software; has no effect on hardware
    DWORD long_mode : 1;
    DWORD big : 1;  // 32-bit opcodes for code, uint32_t stack for data
    DWORD gran : 1; // 1 to use 4k page addressing, 0 for byte addressing
    DWORD base_high : 8;
} PACKED GDT_ENTRY, *PGDT_ENTRY;


// a more humane gdt version, where we operate on
// will build a function to convert these members to the ones above
typedef struct
{
    DWORD base;
    DWORD limit;
    DWORD flags;
} GDT_READABLE, *PGDT_READABLE;

// https://wiki.osdev.org/GDT_Tutorial#Flat_.2F_Long_Mode_Setup
#define GDT_FLAG_KERNEL_CODE 0x9A
#define GDT_FLAG_KERNEL_DATA 0x92
#define GDT_FLAG_USER_CODE   0xFA
#define GDT_FLAG_USER_DATA   0xF2
#define GDT_FLAG_TSS         0x89

#define GDT_KERNEL_CS 0b001000    // index 1 in GDT, 0 for GDT and 00 for privileged(ring 0)
#define GDT_KERNEL_DATA 0b010000  // index 2 in GDT, 0 for GDT and 00 for privileged(ring 0)
#define GDT_USER_CS 0b011011      // index 3 in GDT, 0 for GDT and 11 for unprivileged(ring 3)
#define GDT_USER_DATA 0b100011    // index 4 in GDT, 0 for GDT and 11 for unprivileged(ring 3)

#define GDT_TOTAL_SEGMENTS 6
#define GDT_TSS_OFFSET 0x28

VOID gdt_load(IN PGDT_ENTRY gdt, IN DWORD size);
VOID gdt_readable_to_gdt_entry(OUT PGDT_ENTRY gdt, IN PGDT_READABLE gdt_readable, IN DWORD count);

#endif