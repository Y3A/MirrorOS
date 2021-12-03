#include "disk/streamer.h"
#include "config.h"
#include "memory/heap/kheap.h"

PSTREAM new_streamer(int disk_no)
{
    // initialise streamer with disk
    PDISK disk = disk_get(disk_no);
    if (!disk)
        return NULL;

    PSTREAM streamer = kzalloc(sizeof(STREAM));
    streamer->pos = 0;
    streamer->disk = disk;

    return streamer;
}

void streamer_seek(PSTREAM stream, int pos)
{
    stream->pos = pos;
}

int streamer_read(PSTREAM stream, void * userbuf, int total)
{

    // read from disk into local buf first, then copy to user buf
    int sector = stream->pos / SECTOR_SZ;
    int offset = stream->pos % SECTOR_SZ;
    char buf[SECTOR_SZ];
    int ret;

    ret = disk_read_block(stream->disk, sector, 1, buf);
    if (ret < 0)
        return ret;

    int read_count = total > SECTOR_SZ ? SECTOR_SZ : total;

    for (int i = 0; i < read_count; i++)
        *(char *)userbuf++ = ((char *)buf)[offset+i];

    // if total > sector size, adjust stream and read again
    stream->pos += read_count;
    if (total > SECTOR_SZ)
        ret = streamer_read(stream, userbuf, total-SECTOR_SZ);

    return ret;
}

void streamer_close(PSTREAM stream)
{
    kfree(stream);
}

