#include "status.h"
#include "fs/vfs.h"
#include "string/string.h"
#include "memory/heap/kheap.h"

static PVFS_NODE vfs_root_node = NULL;

MIRRORSTATUS vfs_init(VOID)
{
    MIRRORSTATUS status = STATUS_SUCCESS;
    
out:
    return status;
}

MIRRORSTATUS vfs_mount(PVOID filesystem, PCSTR mnt_point)
{
    MIRRORSTATUS status = STATUS_SUCCESS;
    PSTR         path;

    if (!vfs_root_node) {
        // root node does not exist, only allow mounting of root nodes!
        if (unbound_strcmp(mnt_point, VFS_ROOT) != 0) {
            status = STATUS_EINVAL;
            goto out;
        }
        vfs_root_node = kzalloc(sizeof(VFS_NODE));
        path = kzalloc(unbound_strlen(mnt_point) + 1);
        unbound_strcpy(path, mnt_point);
        vfs_root_node->path = path;
        vfs_root_node->ops = (PVFS_OPS)filesystem;

        goto out;
    }

    // root already mounted
    if (unbound_strcmp(mnt_point, VFS_ROOT) == 0)
    {
        status = STATUS_EEXIST;
        goto out;
    }

out:
    return status;
}