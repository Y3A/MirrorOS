#include "kernel.h"
#include "status.h"
#include "types.h"
#include "disk/disk.h"
#include "disk/streamer.h"
#include "fs/fat16/fat16.h"
#include "fs/file.h"
#include "fs/path_parser.h"
#include "idt/idt.h"
#include "io/io.h"
#include "string/string.h"
#include "memory/paging/paging.h"
#include "memory/heap/kheap.h"

DWORD g_cur_x, g_cur_y;

VOID terminal_init(VOID)
{
    g_cur_x = 0;
    g_cur_y = 0;

    for (DWORD y = 0; y < VGA_HEIGHT; y++)
        for (DWORD x = 0; x < VGA_WIDTH; x++)
            terminal_putchar(x, y, ' ', 0);
}

WORD terminal_make_char(BYTE c, BYTE colour)
{
    return (colour << 8 | c);
}

VOID terminal_putchar(DWORD x, DWORD y, BYTE c, BYTE colour)
{
    PWORD v_mem = (PWORD)VGA_TEXT_BUF;
    v_mem[(y * VGA_WIDTH) + x] = terminal_make_char(c, colour);
}

VOID terminal_checkputchar(BYTE c, BYTE colour)
{
    if (c == '\x0') // end of str
        return;

    if (c == '\n') // newline
    {
        g_cur_x = 0;
        g_cur_y++;
        return;
    }

    terminal_putchar(g_cur_x++, g_cur_y, c, colour);
    if (g_cur_x >= VGA_WIDTH)
    {
        g_cur_x = 0;
        g_cur_y++;
    }

    return;
}

VOID terminal_print(PCSTR str)
{
    terminal_write(str, COL_WHITE);
}

VOID terminal_warn(PCSTR str)
{
    terminal_write(str, COL_RED);
}

VOID terminal_write(PCSTR str, BYTE colour)
{
    ULONG len = strlen(str);
    for (DWORD i = 0; i < len; i++)
        terminal_checkputchar(str[i], colour);
}

VOID kernel_panic(PCSTR msg)
{
    terminal_warn(msg);
    while (1) ;
}

VOID kernel_main(VOID)
{
    MIRRORSTATUS        status = STATUS_SUCCESS;
    PPAGE_CHUNK         kernel_pagechunk = NULL;

    // initialise terminal
    terminal_init();

    // initialise kernel heap
    status = kheap_init();
    if (!MIRROR_SUCCESS(status))
        kernel_panic("[-] Kernel Heap Initialization Failed\n");

    // initialise filesystems
    fs_init();

    // initialise disks
    disk_search_init();

    // initialise IDT
    idt_init();
    
    // setup paging
    status = paging_new_pagechunk(&kernel_pagechunk, PAGING_RDWR | PAGING_IS_PRESENT | PAGING_USER_ACCESS);
    if (!MIRROR_SUCCESS(status))
        kernel_panic("[-] Paging Initialization Failed\n");
        
    paging_switch_pagedir(kernel_pagechunk->pagedir);

    // enable paging
    paging_enable_paging();

    __asm__("sti;"); // enable interrupts
    
    terminal_print("[+] All Initialised\n");

    /*  // tests
    
    int fd = fopen("0:/hello.txt", "r");

    char buf[20];
    memset(buf, 0, sizeof(buf));
    fread(buf, 1, 20, fd);
    terminal_print(buf);

    // end tests */

    while (1);
}