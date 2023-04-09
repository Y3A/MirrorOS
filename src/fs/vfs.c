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

PVFS_NODE vfs_exists(PVFS_NODE node, PSTR filepath)
{
    PVFS_NODE ret = NULL;
    if (!node)
        return NULL;

    if (unbound_strlen(node->path) == unbound_strlen(filepath) && unbound_strcmp(node->path, filepath) == 0)
        return node;

    ret = vfs_exists(node->nextsibling, filepath);
    if (ret)
        return ret;

    ret = vfs_exists(node->firstchild, filepath);

    return ret;
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

MIRRORSTATUS vfs_read(PSTR filepath, PBYTE buf, DWORD offset, DWORD size)
{
    // return no such device if vfs search fails
    MIRRORSTATUS status = STATUS_ENODEV;
    PSTR         cpy, cur;
    PVFS_NODE    node;
    PPATH        path;

    if (!buf || !filepath || *filepath == '\0') {
        status = STATUS_EINVAL;
        goto out;
    }

    for (cpy = kzalloc(unbound_strlen(filepath) + 1), unbound_strcpy(cpy, filepath); *cpy; cpy[unbound_strlen(cpy) - 1] = 0) {
        if ((node = vfs_exists(vfs_root_node, cpy))) {
            cur = cpy;
            // for a path like /home/a.txt, advance to a.txt
            while (*filepath++ == *cur++);
            // we wanna keep the /, and the previous line advances one too much
            // so back by 2
            path = create_path(filepath-2);
            if (node->ops->gettype(node->ops, path) != VFS_TYPE_REG_FILE) {
                status = STATUS_EINVAL;
                goto out;
            }
            status = node->ops->read(node->ops, path, buf, offset, size);
            goto out;
        }
    }

out:
    if (cpy)
        kfree(cpy);
    if (path)
        delete_path(path);
    return status;
}