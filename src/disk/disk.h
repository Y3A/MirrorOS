#ifndef DISK_H
#define DISK_H

typedef unsigned int DISK_TYPE;

#define DISK_REAL 0; // real physical hard disk

typedef struct
{
    DISK_TYPE type;
    int sector_size;
} DISK, *PDISK;

int disk_read_block(PDISK disk, unsigned int lba, int sectors, void * buf);
PDISK disk_get(int idx);
void disk_search_init(void);


#endif