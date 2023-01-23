#ifndef VFS_H
#define VFS_H

#include "path.h"

#define VFS_ROOT "/"

typedef DWORD VFS_TYPE;

#define VFS_TYPE_BAD ((VFS_TYPE)0x0)
#define VFS_TYPE_REG_FILE ((VFS_TYPE)0x1)
#define VFS_TYPE_DIR ((VFS_TYPE)0x4)

typedef MIRRORSTATUS (*_OPEN)(PVOID internal, PPATH path, DWORD flags);
typedef VOID (*_CLOSE)(PVOID internal);
typedef MIRRORSTATUS (*_LISTDIR)(PVOID internal, PPATH path, PBYTE buf, DWORD size);
typedef MIRRORSTATUS (*_READ)(PVOID internal, PPATH path, PBYTE buf, DWORD offset, DWORD size);
typedef VFS_TYPE (*_GETTYPE)(PVOID internal, PPATH path);

typedef struct
{
    _OPEN open;
    _CLOSE close;
    _LISTDIR listdir;
    _READ read;
    _GETTYPE gettype;

} VFS_OPS, *PVFS_OPS;

typedef struct _VFS_NODE
{
    PSTR path;
    PVFS_OPS ops;
    struct _VFS_NODE *firstchild;
    struct _VFS_NODE *nextsibling;
} VFS_NODE, *PVFS_NODE;

MIRRORSTATUS vfs_init(VOID);
MIRRORSTATUS vfs_mount(PVOID filesystem, PCSTR mnt_point);
MIRRORSTATUS vfs_read(PSTR filepath, PBYTE buf, DWORD offset, DWORD size);

#endif