#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>

#define ERR_I(value) (int)value
#define ISERR(value) ((int)value < 0)

#define VGA_TEXT_BUF 0xb8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 20

#define COL_BLACK 0
#define COL_RED 4
#define COL_WHITE 15

extern int cur_x, cur_y;

void kernel_main(void);
uint16_t terminal_make_char(char, char);
void terminal_putchar(int, int, char, char);
void terminal_writechar(char, char);
void terminal_print(const char *);
void terminal_warn(const char *);
void terminal_write(const char *, char);
void terminal_init(void);

#endif