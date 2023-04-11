#include "kernel.h"
#include "status.h"
#include "types.h"
#include "drivers/vga.h"
#include "drivers/ata.h"
#include "fs/ext2fs.h"
#include "fs/vfs.h"
#include "gdt/gdt.h"
#include "idt/idt.h"
#include "task/tss.h"
#include "memory/paging/paging.h"
#include "memory/heap/kheap.h"

VOID kernel_panic(PCSTR msg)
{
    vga_warn(msg);
    while (1) ;
}

TSS tss = { 0 };
GDT_ENTRY gdt[GDT_TOTAL_SEGMENTS] = { 0 };
GDT_READABLE gdt_readable[GDT_TOTAL_SEGMENTS] = {
    { 0 }, // null segment
    { .base = MEMORY_BASE, .limit = MEMORY_LIMIT, .flags = GDT_FLAG_KERNEL_CODE },
    { .base = MEMORY_BASE, .limit = MEMORY_LIMIT, .flags = GDT_FLAG_KERNEL_DATA},
    { .base = MEMORY_BASE, .limit = MEMORY_LIMIT, .flags = GDT_FLAG_USER_CODE},
    { .base = MEMORY_BASE, .limit = MEMORY_LIMIT, .flags = GDT_FLAG_USER_DATA},
    { .base = (DWORD)&tss, .limit = sizeof(tss), .flags = GDT_FLAG_TSS}
};


VOID kernel_main(VOID)
{
    MIRRORSTATUS        status = STATUS_SUCCESS;
    PULONG              kernel_pagedir = NULL;
    PVOID               main_fs;

    // initialise gdt
    gdt_readable_to_gdt_entry((PGDT_ENTRY)&gdt, (PGDT_READABLE)&gdt_readable, GDT_TOTAL_SEGMENTS);
    gdt_load((PGDT_ENTRY)&gdt, sizeof(gdt));

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

    // initialise and load TSS
    tss_init_tss(&tss, KERNEL_STACK_BASE, GDT_KERNEL_DATA);
    tss_load_tss(GDT_TSS_OFFSET);
    
    // setup paging
    status = paging_new_pagedir(&kernel_pagedir, PAGING_RDWR | PAGING_IS_PRESENT | PAGING_USER_ACCESS);
    if (!MIRROR_SUCCESS(status))
        kernel_panic("[-] Paging Initialization Failed\n");
        
    paging_switch_pagedir(kernel_pagedir);

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

    /* tests
    char *xx = kzalloc(0x20);
    char *yy = kzalloc(0x60);
    char *zz = kzalloc(0x123);
    char *aa = kzalloc(0x31);
    kfree(zz);
    kfree(yy);
    
    char *bb = kzalloc(0x30);
    char *cc = kzalloc(0x30);
    char *dd = kzalloc(0x31);
    char *ee = kzalloc(0x200);
    kfree(cc);
    kfree(dd);
    kfree(xx);
    kfree(aa);
    kfree(bb);
    kfree(ee);
    CHAR buf[100];
    FILE fd;
    if (!MIRROR_SUCCESS(vfs_open("/test2.txt", &fd)))
        vga_warn("Open error");
    vga_print("safe\n");
    if (!MIRROR_SUCCESS(vfs_read(fd, (PBYTE)buf, 0, sizeof(buf))))
        vga_warn("Read error");
    else
        vga_print((PCSTR)buf);

    unbound_memset(buf, 0, sizeof(buf));
    if (!MIRROR_SUCCESS(vfs_open("/test2.txt", &fd)))
        vga_warn("Open error");
    if (!MIRROR_SUCCESS(vfs_open("/test2.txt", &fd)))
        vga_warn("Open error");

    if (!MIRROR_SUCCESS(vfs_read(fd, (PBYTE)buf, 0, sizeof(buf))))
        vga_warn("Read error");

    vga_print((PCSTR)buf);

    vfs_close(fd);
    vfs_close(fd);
    vga_print("nn\n");
    vfs_close(fd);
    vga_print("yy\n");
    if (!MIRROR_SUCCESS(vfs_open("/test2.txt", &fd)))
        vga_warn("Open error");
    if (!MIRROR_SUCCESS(vfs_open("/test2.txt", &fd)))
        vga_warn("Open error");
    */

    while (1);
}