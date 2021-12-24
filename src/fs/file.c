#include <stddef.h>

#include "config.h"
#include "kernel.h"
#include "status.h"
#include "disk/disk.h"
#include "fs/file.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "fs/fat16/fat16.h"

PFILESYSTEM filesystems[MAX_FS];
PFILE_DESCRIPTOR filedescriptors[MAX_FD];

static PFILESYSTEM * get_free_fs(void);
static void load_builtin_fs(void);
void load_fs(void);
static int new_fd(PFILE_DESCRIPTOR * fd_out);
static PFILE_DESCRIPTOR get_fd(int fd);
static FILE_MODE get_mode_from_str(const char * str);

static PFILESYSTEM * get_free_fs(void)
{
    for (int i = 0; i < MAX_FS; i++)
        if (filesystems[i] == 0)
            return &filesystems[i];
    return NULL;
}

void insert_fs(PFILESYSTEM fs)
{
    PFILESYSTEM * newfs;
    newfs = get_free_fs();
    if (!fs)
    {
        terminal_warn("insert fs");
        while (1) ;
    }
    *newfs = fs;
}

static void load_builtin_fs(void)
{
    insert_fs(fat16_init());
}

void load_fs(void)
{
    memset(filesystems, 0, sizeof(filesystems));
    load_builtin_fs();
}

void fs_init(void)
{
    memset(filedescriptors, 0, sizeof(filedescriptors));
    load_fs();
}

static int new_fd(PFILE_DESCRIPTOR * fd_out)
{
    for (int i =0; i < MAX_FD; i++)
        if (filedescriptors[i] == 0)
        {
            PFILE_DESCRIPTOR new = kzalloc(sizeof(FILE_DESCRIPTOR));
            new->idx = i+1; // fd starts at 1
            filedescriptors[i] = new;
            *fd_out = new;
            return 0;
        }
    return -ENOMEM;
}

static PFILE_DESCRIPTOR get_fd(int fd)
{
    if (!(0 < fd && fd < MAX_FD))
        return NULL;
    
    int idx = fd - 1;
    return filedescriptors[idx];
}

PFILESYSTEM fs_resolve(PDISK disk)
{
    PFILESYSTEM newfs = NULL;
    for (int i = 0; i < MAX_FS; i++)
        if (filesystems[i] != 0 && filesystems[i]->resolve(disk) == 0)
        {
            newfs = filesystems[i];
            break;
        }

    return newfs;
}

int fopen(const char * filename, const char * mode_str)
{
    /*
     * Example path: 0:/dir/file.txt
     */

    int res = 0;
    PPATH_ROOT root_path = parse(filename, NULL);
    if (!root_path)
    {
        res = -EINVAL;
        goto out;
    }

    if (!root_path->first)
    {
        /*
         * If path is just root, like 0:/
         * we need to reject this request
         */
        res = -EINVAL;
        goto out;
    }

    PDISK disk = disk_get(root_path->drive_no);
    /*
     * check if disk exists
     */
    if (!disk)
    {
        res = -EIO;
        goto out;
    }

    /*
     * check if disk has filesystem bind to it
     */
    if (!disk->fs)
    {
        res = -EIO;
        goto out;
    }

    FILE_MODE mode = get_mode_from_str(mode_str);
    if (ISERR(mode))
    {
        res = -EINVAL;
        goto out;
    }

    void * private_data = disk->fs->open(disk, root_path->first, mode);

    if (ISERR(private_data))
    {
        res = ERR_I(private_data);
        goto out;
    }

    PFILE_DESCRIPTOR fd = 0;
    res = new_fd(&fd);
    if (ISERR(res))
        goto out;

    fd->fs = disk->fs;
    fd->priv = private_data;
    fd->disk = disk;
    res = fd->idx; // return new fd to caller

out:
    // fopen should not return negative
    if (res < 0)
        res = 0;
    return res;
}

static FILE_MODE get_mode_from_str(const char * str)
{
    FILE_MODE mode = -1;
    switch (*str)
    {
        case 'r':
            if (*(str+1) == '+')
                mode = FILE_READWRITE;
            else
                mode = FILE_READONLY;
            break;

        case 'w':
            mode = FILE_WRITEONLY;
            break;

        case 'a':
            mode = FILE_APPENDONLY;
            break;
        
        default:
            break;
    }
    return mode;
}

int fread(void * ptr, size_t size, uint32_t nmemb, int fd)
{
    int res = 0;
    if (!size || !nmemb || fd < 1)
    {
        res = -EINVAL;
        goto out;
    }

    PFILE_DESCRIPTOR desc = get_fd(fd);
    if (!desc)
    {
        res = -EINVAL;
        goto out;
    }

    res = desc->fs->read(desc->disk, desc->priv, size, nmemb, (char *)ptr);

out:
    return res;
}