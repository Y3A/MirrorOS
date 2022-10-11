#include <stddef.h>
#include <stdint.h>

#include "config.h"
#include "kernel.h"
#include "status.h"
#include "disk/streamer.h"
#include "memory/memory.h"
#include "string/string.h"
#include "fs/fat16/fat16.h"
#include "memory/heap/kheap.h"

static void fat16_init_internal(PDISK disk, PFAT16_INTERNAL internal);
static int fat16_get_root_dir(PDISK disk, PFAT16_INTERNAL internal, PFAT16_DIR root_dir);
static int fat16_get_total_items_for_dir(PFAT16_INTERNAL internal, PDISK disk, int dir_sector_start);
static PFAT16_INTERNAL_ITEM get_dir_entry(PDISK disk, PPATH_PART part);
static PFAT16_INTERNAL_ITEM dir_get_item(PDISK disk, PFAT16_DIR dir, const char * path);
static PFAT16_INTERNAL_ITEM new_fat16_internal_item(PDISK disk, PFAT16_ITEM item);
static void get_full_filename(PFAT16_ITEM item, char * out, size_t sz);
static PFAT16_DIR load_dir(PDISK disk, PFAT16_ITEM item);
static PFAT16_ITEM clone_dir_item(PFAT16_ITEM item, size_t sz);
static int cluster_to_sector(PFAT16_INTERNAL internal, int cluster);
static int read_internal(PDISK disk, int starting_cluster, int offset, int total, void * out);
static int read_internal_stream(PDISK disk, PSTREAM stream, int cluster, int offset, int total, void * out);
static int get_cluster_at_offset(PDISK disk, int start_cluster, int offset);
static void free_dir(PFAT16_DIR dir);
static void free_internal_item(PFAT16_INTERNAL_ITEM item);
static int16_t fat16_get_entry(PDISK disk, int cluster);
static void fat16_to_proper_string(char** out, const char* in);

FILESYSTEM fat16_fs = 
{
    .resolve = fat16_resolve,
    .open = fat16_open,
    .read = fat16_read
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
        return STATUS_ENOMEM;
    }

    if (streamer_read(stream, &(internal->header), sizeof(internal->header)) < 0)
    {
        if (stream)
            streamer_close(stream);
        kfree(internal);
        return STATUS_EIO;
    }
    
    if (internal->header.extended_header.boot_signature != 0x29)
    {
        if (stream)
            streamer_close(stream);
        kfree(internal);
        return STATUS_ENODEV;
    }

    // it is fat16!
    // check if we can load root dir
    if (fat16_get_root_dir(disk, internal, &internal->root_dir) < 0)
    {
        if (stream)
            streamer_close(stream);
        kfree(internal);
        return STATUS_EIO;
    }

    if (stream)
        streamer_close(stream);

    disk->fs_internal = internal;
    disk->fs = &fat16_fs;
    return 0;
}

void * fat16_open(PDISK disk, PPATH_PART path, FILE_MODE mode)
{
    PFAT16_ITEM_DESCRIPTOR desc = NULL;
    desc = (PFAT16_ITEM_DESCRIPTOR)kzalloc(sizeof(FAT16_ITEM_DESCRIPTOR));
    if (!desc)
        return (void *)STATUS_ENOMEM;

    desc->item = get_dir_entry(disk, path);
    if (!desc->item)
    {
        kfree(desc);
        return (void *)STATUS_EIO;
    }

    desc->seek_pos = 0;
    return desc;
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

    PFAT16_ITEM dir_file = (PFAT16_ITEM)kzalloc(root_dir_sz);

    if (!dir_file)
        return STATUS_ENOMEM;
    
    PSTREAM new_stream = internal->cluster_read_stream;
    streamer_seek(new_stream, (disk->sector_size * root_dir_sector_start));
    if (streamer_read(new_stream, dir_file, root_dir_sz) < 0)
    {
        kfree(dir_file);
        return STATUS_EIO;
    }

    root_dir->first_item = dir_file;
    root_dir->total_items = total_items;
    root_dir->start_sector = root_dir_sector_start;
    root_dir->end_sector = root_dir_sector_start + (root_dir_sz / disk->sector_size);

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
            return -STATUS_EIO;
        count++;

    } while (cur.filename[0] != 0x00);
    
    return count-1;
}

static PFAT16_INTERNAL_ITEM get_dir_entry(PDISK disk, PPATH_PART path)
{
    PFAT16_INTERNAL internal = disk->fs_internal;
    PFAT16_INTERNAL_ITEM cur = NULL;
    PFAT16_DIR cur_dir = &internal->root_dir;
    
    do
    {
        cur = dir_get_item(disk, cur_dir, path->part);
        if (!cur)
            goto out;

    } while (path->next != NULL && (path = path->next) && (cur_dir = cur->directory));
    
    if (cur->type == FAT16_TYPE_DIRECTORY)
        // attempting to open a directory will fail
        cur = NULL;

out:
    return cur;
}

static PFAT16_INTERNAL_ITEM dir_get_item(PDISK disk, PFAT16_DIR dir, const char * path)
{
    PFAT16_INTERNAL_ITEM item = NULL;
    char filename[MAX_PATH];
    for (int i = 0; i < dir->total_items; i++)
    {
        // combine filename and extension
        get_full_filename(&(dir->first_item[i]), filename, sizeof(filename));
        if (istrncmp(filename, path, sizeof(filename)) == 0)
            // found file/dir
            item = new_fat16_internal_item(disk, &(dir->first_item[i]));
    }
    return item;
}

static PFAT16_INTERNAL_ITEM new_fat16_internal_item(PDISK disk, PFAT16_ITEM item)
{
    PFAT16_INTERNAL_ITEM internal_item = kzalloc(sizeof(FAT16_INTERNAL_ITEM));
    if (!internal_item)
        return NULL;
    
    if (item->attribute & FAT16_DIRECTORY)
    {
        internal_item->directory = load_dir(disk, item);
        internal_item->type = FAT16_TYPE_DIRECTORY;
    }
    else
    {
        internal_item->item = clone_dir_item(item, sizeof(FAT16_ITEM));
        internal_item->type = FAT16_TYPE_FILE;
    }

    return internal_item;
}

static void get_full_filename(PFAT16_ITEM item, char * out, size_t sz)
{
    memset(out, 0, sz);
    char *out_tmp = out;
    fat16_to_proper_string(&out_tmp, (const char*) item->filename);
    if (item->ext[0] != 0x00 && item->ext[0] != 0x20)
    {
        *out_tmp++ = '.';
        fat16_to_proper_string(&out_tmp, (const char*) item->ext);
    }
}

static void fat16_to_proper_string(char** out, const char* in)
{
    while(*in != 0x00 && *in != 0x20)
    {
        **out = *in;
        *out += 1;
        in +=1;
    }

    if (*in == 0x20)
    {
        **out = 0x00;
    }
}

static PFAT16_DIR load_dir(PDISK disk, PFAT16_ITEM item)
{
    int res = 0;
    PFAT16_DIR dir = NULL;
    PFAT16_INTERNAL internal = disk->fs_internal;

    if (!(item->attribute & FAT16_DIRECTORY))
    {
        res = STATUS_EINVAL;
        goto out;
    }

    dir = (PFAT16_DIR)kzalloc(sizeof(FAT16_DIR));
    if (!dir)
    {
        res = STATUS_ENOMEM;
        goto out;
    }

    int cluster = item->first_cluster_number;
    int sector = cluster_to_sector(internal, cluster);
    int total_items = fat16_get_total_items_for_dir(internal, disk, sector);
    dir->total_items = total_items;
    size_t dir_sz = total_items * sizeof(FAT16_ITEM);
    dir->first_item = (PFAT16_ITEM)kzalloc(dir_sz);
    if (!dir->first_item)
    {
        res = STATUS_ENOMEM;
        goto out;
    }

    res = read_internal(disk, cluster, 0, dir_sz, dir->first_item);

    if (res)
        goto out;

out:
    if (res)
        free_dir(dir);
    return dir;
}

static PFAT16_ITEM clone_dir_item(PFAT16_ITEM item, size_t sz)
{
    PFAT16_ITEM copy = NULL;
    if (sz < sizeof(FAT16_ITEM))
        return NULL;
    
    copy = (PFAT16_ITEM)kzalloc(sz);
    if (!copy)
        return NULL;
    
    memcpy(copy, item, sz);
    return copy;
}

static int cluster_to_sector(PFAT16_INTERNAL internal, int cluster)
{
    return internal->root_dir.end_sector + ((cluster-2) * internal->header.sectors_per_cluster);
}

static int read_internal(PDISK disk, int starting_cluster, int offset, int total, void * out)
{
    PFAT16_INTERNAL internal = (PFAT16_INTERNAL)(disk->fs_internal);
    return read_internal_stream(disk, internal->cluster_read_stream, starting_cluster, offset, total, out);
}

static int read_internal_stream(PDISK disk, PSTREAM stream, int cluster, int offset, int total, void * out)
{
    // read in reverse order
    int res= 0;
    PFAT16_INTERNAL internal = (PFAT16_INTERNAL)(disk->fs_internal);
    int cluster_sz_bytes = disk->sector_size * internal->header.sectors_per_cluster;
    int total_clusters = get_cluster_at_offset(disk, cluster, offset);
    if (total_clusters < 0)
    {
        res = total_clusters;
        goto out;
    }

    int offset_after_cluster = offset % cluster_sz_bytes;

    int start_sector = cluster_to_sector(internal, total_clusters);
    int start_pos = (start_sector * disk->sector_size) + offset_after_cluster;
    int to_read = total > cluster_sz_bytes ? cluster_sz_bytes : total;

    streamer_seek(stream,start_pos);

    res = streamer_read(stream, out, to_read);
    if (res < 0)
        goto out;
    
    total -= to_read;
    if (total > 0)
        res = read_internal_stream(disk, stream, cluster, offset+to_read, total, out+to_read);

out:
    return res;
}

static int get_cluster_at_offset(PDISK disk, int start_cluster, int offset)
{
    int res = 0;
    PFAT16_INTERNAL internal = (PFAT16_INTERNAL)(disk->fs_internal);
    int cluster_sz_bytes = disk->sector_size * internal->header.sectors_per_cluster;
    int cur = start_cluster;
    int clusters_ahead = offset / cluster_sz_bytes;

    for (int i = 0; i < clusters_ahead; i++)
    {
        int16_t entry = fat16_get_entry(disk, cur);
        switch (entry)
        {
            case FAT16_BAD_SECTOR:
            case FAT16_UNUSED:
                res = STATUS_EIO;
                goto out;
            
            default:
                cur = entry;
                break;
        }
    }
    res = cur;

out:
    return res;
}

static void free_dir(PFAT16_DIR dir)
{
    if (!dir)
        return;
    
    if(dir->first_item)
        kfree(dir->first_item);
    
    kfree(dir);
}

static void free_internal_item(PFAT16_INTERNAL_ITEM item)
{
    if (item->type == FAT16_TYPE_DIRECTORY)
        free_dir(item->directory);
    else if (item->type == FAT16_TYPE_FILE)
        kfree(item->item);
    
    kfree(item);
}

static int16_t fat16_get_entry(PDISK disk, int cluster)
{
    int16_t res = -1;
    PFAT16_INTERNAL internal = (PFAT16_INTERNAL)(disk->fs_internal);
    PSTREAM stream = internal->fat16_read_stream;
    if (!stream)
        return res;
    
    uint32_t fat_position = internal->header.reserved_sector_count * disk->sector_size;
    streamer_seek(stream, fat_position + (cluster * 2));
    int16_t in = 0;
    res = streamer_read(stream, &in, sizeof(in));
    if (res < 0)
        return res;
    
    return in;
}

int fat16_read(PDISK disk, void * desc, uint32_t size, uint32_t nmemb, char * out)
{
    int res = 0; // return successful read count
    PFAT16_ITEM_DESCRIPTOR descriptor = desc;
    PFAT16_ITEM item = descriptor->item->item;
    int offset = descriptor->seek_pos;
    for (uint32_t i = 0; i < nmemb; i++)
    {
        res = read_internal(disk, item->first_cluster_number, offset, size, out);
        if (!MIRROR_SUCCESS(res))
            goto endfunc;

        out += size;
        offset += size;
    }

    res = nmemb;

endfunc:
    return res;
}