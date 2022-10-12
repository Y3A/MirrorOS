#include "types.h"
#include "drivers/vga.h"
#include "string/string.h"

DWORD g_cur_x, g_cur_y;

VOID vga_init(VOID)
{
    g_cur_x = 0;
    g_cur_y = 0;

    for (DWORD y = 0; y < VGA_HEIGHT; y++)
        for (DWORD x = 0; x < VGA_WIDTH; x++)
            vga_putchar(x, y, ' ', 0);
}

WORD vga_makechar(BYTE c, BYTE colour)
{
    return (colour << 8 | c);
}

VOID vga_putchar(DWORD x, DWORD y, BYTE c, BYTE colour)
{
    PWORD v_mem = (PWORD)VGA_TEXT_BUF;
    v_mem[(y * VGA_WIDTH) + x] = vga_makechar(c, colour);
}

VOID vga_checkputchar(BYTE c, BYTE colour)
{
    if (c == '\x0') // end of str
        return;

    if (c == '\n') // newline
    {
        g_cur_x = 0;
        g_cur_y++;
        return;
    }

    vga_putchar(g_cur_x++, g_cur_y, c, colour);
    if (g_cur_x >= VGA_WIDTH)
    {
        g_cur_x = 0;
        g_cur_y++;
    }

    return;
}

VOID vga_print(PCSTR str)
{
    vga_write(str, COL_WHITE);
}

VOID vga_warn(PCSTR str)
{
    vga_write(str, COL_RED);
}

VOID vga_write(PCSTR str, BYTE colour)
{
    ULONG len = unbound_strlen(str);
    for (DWORD i = 0; i < len; i++)
        vga_checkputchar(str[i], colour);
}