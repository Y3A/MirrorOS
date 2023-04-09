#ifndef CONFIG_H
#define CONFIG_H

#define TOTAL_INTERRUPTS 256

#define GDT_CS 0b1000 // index 1 in GDT, 0 for GDT and 00 for privileged
#define GDT_DATA 0b10000 // index 2 in GDT, 0 for GDT and 00 for privileged

#define KERNEL_HEAP_FREE_BIN_HEAD 0x00007E00

#define KERNEL_STACK_LIMIT 0x600000

#define SECTOR_SZ 512

#define TOTAL_GDT_SEGMENTS 6
#define GDT_TSS_OFFSET 0x28

#define MEMORY_BASE 0
#define MEMORY_LIMIT 0xffffffff

#endif