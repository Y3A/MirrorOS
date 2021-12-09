#include <stddef.h>

#include "config.h"
#include "kernel.h"
#include "status.h"
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
    if (!(0 > fd && fd < MAX_FD))
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

int fopen(const char * filename, const char * mode)
{
    return -EIO;
}