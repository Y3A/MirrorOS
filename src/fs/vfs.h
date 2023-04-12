#ifndef VFS_H
#define VFS_H

#include "path.h"

#define VFS_ROOT "/"
#define FILE_HEAD 0

typedef DWORD VFS_TYPE;

#define VFS_TYPE_BAD ((VFS_TYPE)0x0)
#define VFS_TYPE_REG_FILE ((VFS_TYPE)0x1)
#define VFS_TYPE_DIR ((VFS_TYPE)0x4)

typedef MIRRORSTATUS (*_OPEN)(PVOID internal, PPATH path, DWORD flags);
typedef VOID (*_CLOSE)(PVOID internal);
typedef MIRRORSTATUS (*_LISTDIR)(PVOID internal, PPATH path, PBYTE buf, DWORD size);
typedef MIRRORSTATUS (*_READ)(PVOID internal, PPATH path, PBYTE buf, DWORD offset, DWORD size);
typedef VFS_TYPE (*_GETTYPE)(PVOID internal, PPATH path);
typedef MIRRORSTATUS (*_GETSIZE)(PVOID internal, PPATH path, PDWORD size);

typedef struct
{
    _OPEN open;
    _CLOSE close;
    _LISTDIR listdir;
    _READ read;
    _GETTYPE gettype;
    _GETSIZE getsize;
} VFS_OPS, *PVFS_OPS;

typedef struct _VFS_NODE
{
    PSTR path;
    PVFS_OPS ops;
    struct _VFS_NODE *firstchild;
    struct _VFS_NODE *nextsibling;
} VFS_NODE, *PVFS_NODE;

// the address is returned to user as "fd"
typedef struct _VFS_FILE
{
    WORD                refcount;
    PSTR                strpath;
    PVFS_NODE           node;
    PPATH               path;
    struct _VFS_FILE    *next;
    struct _VFS_FILE    *prev;
} VFS_FILE, *PVFS_FILE;

MIRRORSTATUS vfs_init(VOID);
MIRRORSTATUS vfs_open(PSTR filepath, PFILE outfd);
void         vfs_close(FILE fd);
MIRRORSTATUS vfs_mount(PVOID filesystem, PCSTR mnt_point);
MIRRORSTATUS vfs_read(FILE fd, PBYTE buf, DWORD offset, DWORD size);
MIRRORSTATUS vfs_getsize(FILE fd, PDWORD outsize);

// internal
PVFS_FILE vfs_create_file_object(PVFS_NODE node, PPATH path, PSTR strpath);
void      vfs_remove_file_object(PVFS_FILE file_obj);
PVFS_FILE vfs_find_file_object(PVFS_NODE node, PSTR strpath);

#endif