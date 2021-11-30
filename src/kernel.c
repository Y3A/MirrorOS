#include <stdint.h>
#include <stddef.h>

#include "kernel.h"
#include "disk/disk.h"
#include "idt/idt.h"
#include "io/io.h"
#include "memory/paging/paging.h"
#include "memory/heap/kheap.h"

int cur_x, cur_y;

void terminal_init(void)
{
    cur_x = 0;
    cur_y = 0;
    for (int y = 0; y < VGA_HEIGHT; y++)
        for (int x = 0; x < VGA_WIDTH; x++)
            terminal_putchar(x, y, ' ', 0);
}

uint16_t terminal_make_char(char c, char colour)
{
    return (colour << 8 | c);
}

size_t strlen(const char * str)
{
    size_t ptr = 0;
    while (str[ptr++]) ;
    return ptr;
}

void terminal_putchar(int x, int y, char c, char colour)
{
    uint16_t * v_mem = (uint16_t *)VGA_TEXT_BUF;
    v_mem[(y * VGA_WIDTH) + x] = terminal_make_char(c, colour);
}

void terminal_writechar(char c, char colour)
{
    if (c == '\x0') // end of str
        return;
    if (c == '\n') // newline
    {
        cur_x = 0;
        cur_y++;
        return;
    }
    terminal_putchar(cur_x++, cur_y, c, colour);
    if (cur_x >= VGA_WIDTH)
    {
        cur_x = 0;
        cur_y++;
    }
}

void terminal_print(const char * str)
{
    terminal_write(str, COL_WHITE);
}

void terminal_warn(const char * str)
{
    terminal_write(str, COL_RED);
}

void terminal_write(const char * str, char colour)
{
    size_t len = strlen(str);
    for (int i = 0; i < len; i++)
        terminal_writechar(str[i], colour);
}

static PPAGE_CHUNK kernel_pagechunk = NULL;

void kernel_main(void)
{
    terminal_init(); // initialise terminal
    kheap_init(); // initialise kernel heap
    disk_search_init(); // initialise disks
    idt_init(); // initialise IDT
    
    // setup paging
    kernel_pagechunk = new_page_chunk(PAGING_RDWR | PAGING_IS_PRESENT | PAGING_USER_ACCESS);
    paging_switch_dir(kernel_pagechunk->pagedir);

    // enable paging
    paging_enable();

    __asm__("sti;"); // enable interrupts
    
    terminal_print("[+] All Initialised\n");

    /* tests
    
    char * buf = page_alloc();
    pagetable_set_entry(kernel_pagechunk->pagedir, (void *)0x400000, (uint32_t)buf | PAGING_IS_PRESENT | PAGING_RDWR | PAGING_USER_ACCESS);
    char * start = (char *)0x400000;
    start ++;
    start --;
    disk_read_block(disk_get(0), 0, 1, buf);

    end tests */

    while (1);
}