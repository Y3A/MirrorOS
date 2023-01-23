#include "kernel.h"
#include "status.h"
#include "types.h"
#include "drivers/vga.h"
#include "drivers/ata.h"
#include "fs/ext2fs.h"
#include "fs/vfs.h"
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
    PVOID               main_fs;

    // initialise vga driver
    vga_init();

    // initialize ata driver
    ata_init();

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

    // initialize filesystem
    status = vfs_init();
    if (!MIRROR_SUCCESS(status))
        kernel_panic("[-] VFS Initialization Failed\n");
        
    main_fs = ext2fs_init(DRIVE_HEAD);
    if (!main_fs) {
        vga_warn("[-] EXT2 FileSystem Initialization Failed\n");
        vga_warn("[-] No FileSystem In Use\n");
    }
    else {
        status = vfs_mount(main_fs, VFS_ROOT);
        if (!MIRROR_SUCCESS(status))
            kernel_panic("[-] VFS Mount Root Failed\n");
    }

    __asm__("sti;"); // enable interrupts
    
    vga_print("[+] All Initialised\n");

    while (1);
}