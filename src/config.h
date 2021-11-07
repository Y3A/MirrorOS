#ifndef CONFIG_H
#define CONFIG_H

#define TOTAL_INTERRUPTS 256

#define GDT_CS 0b1000 // index 1 in GDT, 0 for GDT and 00 for privileged
#define GDT_DATA 0b10000 // index 2 in GDT, 0 for GDT and 00 for privileged

#define KERNEL_HEAP_FREE_BIN_HEAD 0x00007E00

#endif