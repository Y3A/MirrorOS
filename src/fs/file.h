#ifndef FILE_H
#define FILE_H

#include "disk/disk.h"
#include "fs/path_parser.h"

typedef unsigned int SEEK_MODE;
typedef unsigned int FILE_MODE;

enum
{
    FILE_SEEK_SET,
    FILE_SEEK_CUR,
    FILE_SEEK_END
};

enum
{
    FILE_READONLY,
    FILE_WRITEONLY,
    FILE_READWRITE,
    FILE_APPEND
};

typedef void*(*FS_OPEN)(PDISK disk, PPATH_PART path, FILE_MODE mode);
typedef int(*FS_RESOLVE)(PDISK disk);

typedef struct _FILESYSTEM
{
    // resolve should return 0 if disk is of this filesystem
    FS_RESOLVE resolve;
    FS_OPEN open;

    char name[20];
} FILESYSTEM, *PFILESYSTEM;

typedef struct
{
    int idx;
    PFILESYSTEM fs;

    // private data for internal fs
    void * priv;

    // disk that fd should be used on
    PDISK disk;
} FILE_DESCRIPTOR, *PFILE_DESCRIPTOR;

void fs_init(void);
int fopen(const char * filename, const char * mode);
void insert_fs(PFILESYSTEM filesystem);
PFILESYSTEM fs_resolve(PDISK disk);

#endif