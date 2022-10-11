#include "config.h"
#include "status.h"
#include "disk/disk.h"
#include "io/io.h"
#include "memory/memory.h"

DISK curdisk;

void disk_read_sector(int lba, int sectors, void * buf)
{
    outsb(0x1f2, (sectors & 0xff));
    outsb(0x1f3, (lba & 0xff));
    outsb(0x1f4, (lba >> 8)&0xff);
    outsb(0x1f5, (lba >> 16)&0xff);
    outsb(0x1f6, ((lba >> 24)|0xe0)&0xff);
    outsb(0x1f7, 0x20);

    unsigned short * ptr = (unsigned short *)buf;
    for (int i = 0; i < sectors; i++)
    {
        // loop until ready to read
        for (char b = insb(0x1f7); !(b & 8); b=insb(0x1f7)) ;

        // copy from hard disk to mem
        for (int j = 0; j < 256; j++)
            *ptr++ = insw(0x1f0);
    }
}

void disk_search_init(void)
{
    memset(&curdisk, 0, sizeof(curdisk));
    curdisk.type = DISK_REAL;
    curdisk.sector_size = SECTOR_SZ;
    curdisk.id = 0;
    curdisk.fs = fs_resolve(&curdisk);
}

PDISK disk_get(int idx)
{
    return (idx == 0 ? &curdisk : NULL);
}

int disk_read_block(PDISK disk, unsigned int lba, int sectors, void * buf)
{
    if (disk != &curdisk)
        return STATUS_ENXIO;

    disk_read_sector(lba, sectors, buf);
    return 0;
}