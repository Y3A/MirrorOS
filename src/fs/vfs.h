#ifndef VFS_H
#define VFS_H

#include "types.h"

#define VFS_ROOT_PATH "!@#" // dummy path as root

typedef VOID (*_OPEN)(PVOID internal, DWORD flags);
typedef VOID (*_CLOSE)(PVOID internal);

typedef struct _VFS_NODE
{
    /* non-binary directed tree */
    PCSTR path;
    struct _VFS_NODE *firstchild;
    struct _VFS_NODE *nextsibling;
    DWORD refcount;
    _OPEN open;
    _CLOSE close;

    PVOID internal;
} VFS_NODE, *PVFS_NODE;

MIRRORSTATUS vfs_init(VOID);
MIRRORSTATUS vfs_mount(PVFS_NODE node, PCSTR path);
PVFS_NODE vfs_lookup_node(PCSTR path);
PVFS_NODE vfs_lookup_node_recur(PVFS_NODE cur, PCSTR path);

#endif