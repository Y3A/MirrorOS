#ifndef VFS_H
#define VFS_H

#include "types.h"

#define VFS_ROOT_PATH "!@#" // dummy path as root

#define VFS_FILE            0x01
#define VFS_DIRECTORY       0x02
#define VFS_CHARDEVICE      0x04
#define VFS_BLOCKDEVICE     0x08
#define VFS_PIPE            0x10
#define VFS_SYMLINK         0x20
#define VFS_MOUNTPOINT      0x40

#define VFS_USER_READ       0x0100 // user read
#define VFS_USER_WRITE      0x0080 // user write
#define VFS_USER_EXECUTE    0x0040 // user execute

typedef VOID (*_OPEN)(PVOID internal, DWORD flags);
typedef VOID (*_CLOSE)(PVOID internal);
/* return all files under a given node as their own vfs_node */
typedef VOID (*_EXPAND)(PVOID internal, PVFS_NODE out_nodes, PDWORD out_nodes_count);

typedef struct _VFS_NODE
{
    /* non-binary directed tree with nodes representing files */

    PCSTR path;
    struct _VFS_NODE *firstchild;
    struct _VFS_NODE *nextsibling;
    DWORD refcount;
    DWORD permissions;
    DWORD type;
    DWORD create_time;
    DWORD access_time;
    DWORD modified_time;
    DWORD inode;

    _OPEN open;
    _CLOSE close;
    _EXPAND expand;

    PVOID internal;
} VFS_NODE, *PVFS_NODE;

MIRRORSTATUS vfs_tree_add(PVFS_NODE node, PCSTR path);
MIRRORSTATUS vfs_tree_populate(PVFS_NODE node);
VOID vfs_tree_remove(PCSTR path);
MIRRORSTATUS vfs_init(VOID);
MIRRORSTATUS vfs_mount(PVFS_NODE node, PCSTR path);
PVFS_NODE vfs_lookup_node(PCSTR path);
PVFS_NODE vfs_lookup_node_recur(PVFS_NODE cur, PCSTR path);

#endif