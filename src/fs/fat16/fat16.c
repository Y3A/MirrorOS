#include <stddef.h>

#include "status.h"
#include "disk/streamer.h"
#include "memory/memory.h"
#include "string/string.h"
#include "fs/fat16/fat16.h"
#include "memory/heap/kheap.h"

static void fat16_init_internal(PDISK disk, PFAT16_INTERNAL internal);
static int fat16_get_root_dir(PDISK disk, PFAT16_INTERNAL internal, PFAT16_DIR root_dir);
static int fat16_get_total_items_for_dir(PFAT16_INTERNAL internal, PDISK disk, int dir_sector_start);

FILESYSTEM fat16_fs = 
{
    .resolve = fat16_resolve,
    .open = fat16_open
};

PFILESYSTEM fat16_init(void)
{
    strcpy(fat16_fs.name, "FAT16");
    return &fat16_fs;
}

int fat16_resolve(PDISK disk)
{
    PFAT16_INTERNAL internal = (PFAT16_INTERNAL)kzalloc(sizeof(FAT16_INTERNAL));
    fat16_init_internal(disk, internal);

    PSTREAM stream = new_streamer(disk->id);
    if (!stream)
    {
        kfree(internal);
        return -ENOMEM;
    }

    if (streamer_read(stream, &(internal->header), sizeof(internal->header)) < 0)
    {
        if (stream)
            streamer_close(stream);
        kfree(internal);
        return -EIO;
    }
    
    if (internal->header.extended_header.boot_signature != 0x29)
    {
        if (stream)
            streamer_close(stream);
        kfree(internal);
        return -ENODEV;
    }

    // it is fat16!
    // check if we can load root dir
    if (fat16_get_root_dir(disk, internal, internal->root_dir) < 0)
    {
        if (stream)
            streamer_close(stream);
        kfree(internal);
        return -EIO;
    }

    if (stream)
        streamer_close(stream);

    disk->fs_internal = internal;
    disk->fs = &fat16_fs;
    return 0;
}

void * fat16_open(PDISK disk, PPATH_PART path, FILE_MODE mode)
{
    return NULL;
}

static void fat16_init_internal(PDISK disk, PFAT16_INTERNAL internal)
{
    memset(internal, 0, sizeof(FAT16_INTERNAL));
    internal->cluster_read_stream = new_streamer(disk->id);
    internal->fat16_read_stream = new_streamer(disk->id);
}

static int fat16_get_root_dir(PDISK disk, PFAT16_INTERNAL internal, PFAT16_DIR root_dir)
{
    PFAT16_HEADER cur_header = &(internal->header);
    int root_dir_sector_start = cur_header->reserved_sector_count + (cur_header->number_of_fat * cur_header->sectors_per_fat);
    int root_dir_entries = cur_header->root_entry_count;
    size_t root_dir_sz = root_dir_entries * sizeof(FAT16_ITEM);
    int total_root_dir_sectors = root_dir_sz / disk->sector_size;
    if (root_dir_sz % disk->sector_size)
        total_root_dir_sectors++;
    
    int total_items = fat16_get_total_items_for_dir(internal, disk, root_dir_sector_start);

    PFAT16_ITEM dir_file = (PFAT16_ITEM)kzalloc(sizeof(FAT16_ITEM));

    if (!dir_file)
        return -ENOMEM;
    
    PSTREAM new_stream = internal->cluster_read_stream;
    streamer_seek(new_stream, (disk->sector_size * root_dir_sector_start));
    if (streamer_read(new_stream, dir_file, root_dir_sz) < 0)
    {
        kfree(dir_file);
        return -EIO;
    }

    root_dir->dir_as_file = dir_file;
    root_dir->total_items = total_items;
    root_dir->start_sector = root_dir_sector_start;

    return 0;
}

static int fat16_get_total_items_for_dir(PFAT16_INTERNAL internal, PDISK disk, int dir_sector_start)
{
    FAT16_ITEM cur;
    memset(&cur, 0, sizeof(cur));
    size_t start_pos = dir_sector_start * disk->sector_size;
    PSTREAM stream = internal->cluster_read_stream;
    streamer_seek(stream, start_pos);

    int count = 0;

    do
    {
        if (streamer_read(stream, &cur, sizeof(cur)) < 0)
            return -EIO;
        count++;

    } while (cur.filename[0] != 0x00);
    
    return count-1;
}