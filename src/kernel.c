#include "kernel.h"
#include "status.h"
#include "types.h"
#include "drivers/vga.h"
#include "idt/idt.h"
#include "memory/paging/paging.h"
#include "memory/heap/kheap.h"

VOID kernel_panic(PCSTR msg)
{
    vga_warn(msg);
    while (1) ;
}

VOID kernel_main(VOID)
{
    MIRRORSTATUS        status = STATUS_SUCCESS;
    PPAGE_CHUNK         kernel_pagechunk = NULL;

    // initialise vga terminal
    vga_init();

    // initialise kernel heap
    status = kheap_init();
    if (!MIRROR_SUCCESS(status))
        kernel_panic("[-] Kernel Heap Initialization Failed\n");

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
    
    vga_print("[+] All Initialised\n");

    while (1);
}