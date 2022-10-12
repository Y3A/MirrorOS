#ifndef ATA_H
#define ATA_H

#include "types.h"

/*
    * The master drive will be our OS
    * Slave drive will be our storage(filesystem)
*/

VOID ata_read(PVOID buf, ULONG size, BYTE bitmask);
VOID ata_read_master(PVOID buf, ULONG size);
VOID ata_read_slave(PVOID buf, ULONG size);
VOID ata_read_sectors(PVOID buf, BYTE sectors);

#endif