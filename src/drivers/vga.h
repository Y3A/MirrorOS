#ifndef VGA_H
#define VGA_H

#define VGA_TEXT_BUF 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 20

#define COL_BLACK 0
#define COL_RED 4
#define COL_WHITE 15

WORD vga_makechar(BYTE c, BYTE colour);
VOID vga_putchar(DWORD x, DWORD y, BYTE c, BYTE colour);
VOID vga_checkputchar(BYTE c, BYTE colour);
VOID vga_print(PCSTR str);
VOID vga_printf(PCSTR format, ...);
VOID vga_warn(PCSTR str);
VOID vga_write(PCSTR str, BYTE colour);
VOID vga_init(VOID);

#endif