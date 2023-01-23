#ifndef ATA_H
#define ATA_H

#include "types.h"

/*
    * The master drive will be our OS
    * Slave drive will be our storage(filesystem)
*/

typedef BYTE DRIVE_TYPE;

#define MASTER_DRIVE ((DRIVE_TYPE)(0xe0))
#define SLAVE_DRIVE  ((DRIVE_TYPE)(0xf0))

#define MASTER_IDENTIFY ((BYTE)(0xa0))
#define SLAVE_IDENTIFY ((BYTE)(0xb0))

#define CONTROL_REG 0x3f6
#define CONTROL_RESET 0x4
#define CONTROL_ZERO 0x0

#define ATA_READ ((BYTE)(0x20))
#define ATA_WRITE ((BYTE)(0x30))
#define ATA_IDENTIFY ((BYTE)(0xec))

#define DRIVE_HEAD 0

VOID ata_init(VOID);
VOID ata_reset(VOID);
MIRRORSTATUS ata_identify_master(VOID);
MIRRORSTATUS ata_identify_slave(VOID);
MIRRORSTATUS ata_read(PVOID buf, ULONG size, ULONG offset, DRIVE_TYPE type);
VOID ata_read_sectors(PVOID buf, BYTE sectors, DWORD lba, DRIVE_TYPE type);
MIRRORSTATUS ata_write(PVOID buf, ULONG size, ULONG offset, DRIVE_TYPE type);
VOID ata_write_sectors(PVOID buf, BYTE sectors, DWORD lba, DRIVE_TYPE type);
MIRRORSTATUS ata_read_filesystem(PVOID buf, ULONG size, ULONG offset);
MIRRORSTATUS ata_write_filesystem(PVOID buf, ULONG size, ULONG offset);

#endif