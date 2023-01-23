#ifndef VFS_H
#define VFS_H

#define VFS_ROOT "/"

typedef VOID (*_OPEN)(PVOID internal, DWORD flags);
typedef VOID (*_CLOSE)(PVOID internal);

typedef struct
{
    _OPEN open;
    _CLOSE close;
} VFS_OPS, *PVFS_OPS;

typedef struct
{
    PSTR path;
    PVFS_OPS ops;
} VFS_NODE, *PVFS_NODE;

MIRRORSTATUS vfs_init(VOID);
MIRRORSTATUS vfs_mount(PVOID filesystem, PCSTR mnt_point);

#endif