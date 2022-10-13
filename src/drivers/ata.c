#include "config.h"
#include "status.h"
#include "types.h"
#include "drivers/ata.h"
#include "drivers/vga.h"
#include "io/io.h"
#include "memory/heap/kheap.h"

BOOL g_master_exists = FALSE, g_slave_exists = FALSE;

VOID ata_init(VOID)
{
    if (MIRROR_SUCCESS(ata_identify_master()))
        g_master_exists = TRUE;
    else
        vga_warn("[-] Master Drive (OS) Not Detected\n");

    if (MIRROR_SUCCESS(ata_identify_slave()))
        g_slave_exists = TRUE;
    else
        vga_warn("[-] Slave Drive (FileSystem) Not Detected\n");

    return;
}

VOID ata_reset(VOID)
{
    /* https://wiki.osdev.org/ATA_PIO_Mode#Resetting_a_drive_.2F_Software_Reset */
    
    outsb(CONTROL_REG, CONTROL_RESET);
    for (int i = 0; i < 30000; i++) ; // simulate io_wait
    outsb(CONTROL_REG, CONTROL_ZERO);
    for (int i = 0; i < 30000; i++) ;
}

MIRRORSTATUS ata_identify_master(VOID)
{
    /* https://wiki.osdev.org/ATA_PIO_Mode#IDENTIFY_command */

    DWORD   dummy_sectors = 0;
    DWORD   dummy_lba = 0;
    BYTE    read = 0;

    ata_reset();

    outsb(0x1f6, MASTER_IDENTIFY);
    outsb(0x1f2, (dummy_sectors & 0xff));
    outsb(0x1f3, (dummy_lba & 0xff));
    outsb(0x1f4, (dummy_lba >> 8) & 0xff);
    outsb(0x1f5, (dummy_lba >> 16) & 0xff);
    outsb(0x1f7, ATA_IDENTIFY);

    read = insb(0x1f7);

    if (read)
        return STATUS_SUCCESS;
    else
        return STATUS_ENXIO;
}

MIRRORSTATUS ata_identify_slave(VOID)
{
    /* https://wiki.osdev.org/ATA_PIO_Mode#IDENTIFY_command */

    DWORD   dummy_sectors = 0;
    DWORD   dummy_lba = 0;
    BYTE    read = 0;

    ata_reset();

    outsb(0x1f6, SLAVE_IDENTIFY);
    outsb(0x1f2, (dummy_sectors & 0xff));
    outsb(0x1f3, (dummy_lba & 0xff));
    outsb(0x1f4, (dummy_lba >> 8) & 0xff);
    outsb(0x1f5, (dummy_lba >> 16) & 0xff);
    outsb(0x1f7, ATA_IDENTIFY);

    read = insb(0x1f7);

    if (read)
        return STATUS_SUCCESS;
    else
        return STATUS_ENXIO;
}

MIRRORSTATUS ata_read(PVOID buf, ULONG size, ULONG offset, DRIVE_TYPE type)
{
    ULONG       sectors;
    DWORD       lba, remains;
    PVOID       read_buf, cpy;

    if (type == MASTER_DRIVE && !g_master_exists)
        return STATUS_ENXIO;

    if (type == SLAVE_DRIVE && !g_slave_exists)
        return STATUS_ENXIO;

    /*
     * if size is not sector(0x200) aligned, we ceiling it
     * we floor the lba and return an user specified offset into it
     */
    sectors = !(size % SECTOR_SZ) ? (size / SECTOR_SZ) : (size / SECTOR_SZ + 1);
    lba = offset / SECTOR_SZ;
    remains = offset % SECTOR_SZ;

    read_buf = kzalloc(sectors * SECTOR_SZ);
    cpy = read_buf;

    ata_read_sectors(read_buf, sectors, lba, type);

    for (DWORD i = 0; i < size; i++)
        *(PBYTE)buf++ = *(PBYTE)(cpy++ + remains);

    kfree(read_buf);

    return STATUS_SUCCESS;
}

VOID ata_read_sectors(PVOID buf, BYTE sectors, DWORD lba, DRIVE_TYPE type)
{
    /* https://wiki.osdev.org/ATA_read/write_sectors#Read_in_LBA_mode */

    PWORD write = (PWORD)buf;

    /*
     * after the identify command we issued before
     * there will be metadata left to read
     * but we don't care so we just reset to flush the data
     */ 
    ata_reset();

    outsb(0x1f2, (sectors & 0xff));
    outsb(0x1f3, (lba & 0xff));
    outsb(0x1f4, (lba >> 8) & 0xff);
    outsb(0x1f5, (lba >> 16) & 0xff);
    outsb(0x1f6, ((lba >> 24) | type) & 0xff);
    outsb(0x1f7, ATA_READ);

    for (BYTE i = 0; i < sectors; i++)
    {
        // loop until ready to read
        for (BYTE b = insb(0x1f7); !(b & 8); b = insb(0x1f7)) ;

        // copy from hard disk to mem
        for (DWORD j = 0; j < (SECTOR_SZ/2); j++)
            *write++ = insw(0x1f0);
    }

    return;
}

MIRRORSTATUS ata_write(PVOID buf, ULONG size, ULONG offset, DRIVE_TYPE type)
{
    ULONG sectors, write_buf_sz;
    DWORD lba, remains, repair_back;
    PVOID read_buf, write_buf, cpy_read, cpy_write;

    if (type == MASTER_DRIVE && !g_master_exists)
        return STATUS_ENXIO;

    if (type == SLAVE_DRIVE && !g_slave_exists)
        return STATUS_ENXIO;

    sectors = !(size % SECTOR_SZ) ? (size / SECTOR_SZ) : (size / SECTOR_SZ + 1);
    lba = offset / SECTOR_SZ;
    remains = offset % SECTOR_SZ;
    write_buf_sz = sectors * SECTOR_SZ;

    write_buf = kzalloc(write_buf_sz);
    cpy_write = write_buf;

    read_buf = kzalloc(SECTOR_SZ);
    cpy_read = read_buf;

    /*
     * if the offset is not sector aligned
     * we will have to read in 1 sector of the lba
     * so we can patch in the offset into the lba correctly
     */

    if (remains) {
        ata_read_sectors(read_buf, 1, lba, type);
        for (DWORD i = 0; i < remains; i++)
            *(PBYTE)(cpy_write++) = *(PBYTE)(cpy_read++);
    }

    unbound_memset(read_buf, 0, SECTOR_SZ);
    cpy_read = read_buf;

    // patch in the actual user buffer
    for (DWORD i = 0; i < size; i ++)
        *(PBYTE)(cpy_write++) = *(PBYTE)buf++;

    /*
     * similarly, if the eventual write is not sector aligned
     * we will have to patch in the back "unwritten" bytes
     */

    if ((remains + size) % write_buf_sz) {
        repair_back = write_buf_sz - (remains + size);
        ata_read_sectors(read_buf, 1, lba + ((remains + size) / \
            SECTOR_SZ), type);
        for (DWORD i = 0; i < repair_back; i++)
            *(PBYTE)(cpy_write++) = *(PBYTE)(cpy_read++ + \
            ((remains + size) % SECTOR_SZ));
    }

    ata_write_sectors(write_buf, sectors, lba, type);

    kfree(read_buf);
    kfree(write_buf);

    return STATUS_SUCCESS;
}

VOID ata_write_sectors(PVOID buf, BYTE sectors, DWORD lba, DRIVE_TYPE type)
{
    PWORD write = (PWORD)buf;

    ata_reset();

    outsb(0x1f2, (sectors & 0xff));
    outsb(0x1f3, (lba & 0xff));
    outsb(0x1f4, (lba >> 8) & 0xff);
    outsb(0x1f5, (lba >> 16) & 0xff);
    outsb(0x1f6, ((lba >> 24) | type) & 0xff);
    outsb(0x1f7, ATA_WRITE);

    for (BYTE i = 0; i < sectors; i++)
    {
        // loop until ready to write
        for (BYTE b = insb(0x1f7); !(b & 8); b = insb(0x1f7)) ;

        // copy from hard disk to mem
        for (DWORD j = 0; j < (SECTOR_SZ / 2); j++)
            outsw(0x1f0, *write++);
    }

    return;
}