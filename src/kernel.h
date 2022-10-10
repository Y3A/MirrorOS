#ifndef KERNEL_H
#define KERNEL_H

#include "types.h"

#define VGA_TEXT_BUF 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 20

#define COL_BLACK 0
#define COL_RED 4
#define COL_WHITE 15

VOID kernel_main(VOID);
WORD terminal_make_char(BYTE c, BYTE colour);
VOID terminal_putchar(DWORD x, DWORD y, BYTE c, BYTE colour);
VOID terminal_checkputchar(BYTE c, BYTE colour);
VOID terminal_print(PCSTR str);
VOID terminal_warn(PCSTR str);
VOID terminal_write(PCSTR str, BYTE colour);
VOID terminal_init(VOID);

#endif