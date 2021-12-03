#ifndef STREAMER_H
#define STREAMER_H

#include "disk/disk.h"

typedef struct
{
    int pos;
    PDISK disk;
} STREAM, *PSTREAM;

PSTREAM new_streamer(int disk_no);
void streamer_seek(PSTREAM stream, int pos);
int streamer_read(PSTREAM stream, void * userbuf, int total);
void streamer_close(PSTREAM stream);

#endif