#include "status.h"
#include "types.h"
#include "fs/vfs.h"
#include "string/string.h"
#include "memory/heap/kheap.h"

PVFS_NODE g_vfs_root_node = NULL; // file system root!

PVFS_NODE vfs_lookup_node_recur(PVFS_NODE cur, PCSTR path)
{
    PVFS_NODE ret = NULL;

    if (!cur)
        return NULL; // base case 1

    if (unbound_strlen(cur->path) == unbound_strlen(path) && unbound_strcmp(cur->path, path) == 0)
        return cur; // base case 2

    if ((ret = vfs_lookup_node_recur(cur->nextsibling, path)))
        return ret;

    return vfs_lookup_node_recur(cur->firstchild, path);
}

PVFS_NODE vfs_lookup_node(PCSTR path)
{
    return vfs_lookup_node_recur(g_vfs_root_node, path);
}

MIRRORSTATUS vfs_init(VOID)
{
    MIRRORSTATUS status = STATUS_SUCCESS;

    if (g_vfs_root_node) {
        status = STATUS_EEXIST;
        goto out;
    }
    
    g_vfs_root_node = kzalloc(sizeof(VFS_NODE));
    if (!g_vfs_root_node) {
        status = STATUS_ENOMEM;
        goto out;
    }

    g_vfs_root_node->path = VFS_ROOT_PATH;

out:
    if (!MIRROR_SUCCESS(status)) {
        if (g_vfs_root_node)
            kfree(g_vfs_root_node);
    }

    return status;
}

MIRRORSTATUS vfs_mount(PVFS_NODE root_node, PCSTR path)
{
    MIRRORSTATUS status;

    root_node->type |= VFS_MOUNTPOINT;

    status = vfs_tree_add(root_node, path);
    if (!MIRROR_SUCCESS(status))
        goto out;

    status = vfs_tree_populate(root_node);
    if (!MIRROR_SUCCESS(status)) {
        vfs_tree_remove(path);
        goto out;
    }

out:
    return status;
}

MIRRORSTATUS vfs_tree_add(PVFS_NODE node, PCSTR path)
{
    MIRRORSTATUS status = STATUS_SUCCESS;
    PSTR         cpy;
    PVFS_NODE    cur = NULL;

    if (!vfs_lookup_node(path)) {
        status = STATUS_EEXIST;
        goto out;
    }

    node->path = path;

    cpy = kzalloc(unbound_strlen(path) + 1);
    if (!cpy) {
        status = STATUS_ENOMEM;
        goto out;
    }

    /*
     * scan backwards to find file
     * for example, searching /mnt/x/ will become /mnt/x, /mnt/ ... until /
     */
    for (unbound_strcpy(cpy, path); *cpy; cpy[unbound_strlen(cpy) - 1] = 0)
        if ((cur = vfs_lookup_node(cpy))) {
            if (cur->firstchild) {
                for (cur = cur->firstchild; cur->nextsibling; cur = cur->nextsibling) ;
                cur->nextsibling = node;
                goto out;
            }
            else {
                cur->firstchild = node;
                goto out;
            }
        }

    if (!cur) {
        cur = g_vfs_root_node;

        if (cur->firstchild)
        {
            for (cur = cur->firstchild; cur->nextsibling; cur = cur->nextsibling) ;
            cur->nextsibling = node;
            goto out;
        }
        else
        {
            cur->firstchild = node;
            goto out;
        }
    }

out:
    if (!MIRROR_SUCCESS(status)) {
        if (cpy)
            kfree(cpy);
    }

    return status;
}

MIRRORSTATUS vfs_tree_populate(PVFS_NODE node)
{
    return STATUS_SUCCESS;
}

VOID vfs_tree_remove(PCSTR path)
{
    return;
}