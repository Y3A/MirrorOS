#ifndef DISK_H
#define DISK_H

typedef unsigned int DISK_TYPE;

typedef struct
{
    DISK_TYPE type;
    int sector_size;
    int id;
    struct _FILESYSTEM * fs;
    
    void * fs_internal;
} DISK, *PDISK;

#include "fs/file.h"

#define DISK_REAL 0; // real physical hard disk

int disk_read_block(PDISK disk, unsigned int lba, int sectors, void * buf);
PDISK disk_get(int idx);
void disk_search_init(void);


#endif