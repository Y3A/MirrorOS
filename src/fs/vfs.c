#include "status.h"
#include "fs/vfs.h"
#include "string/string.h"
#include "memory/heap/kheap.h"

static PVFS_NODE vfs_root_node = NULL;
static PVFS_FILE vfs_root_file = NULL;

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

MIRRORSTATUS vfs_open(PSTR filepath, PFILE outfd)
{
    // return no such device if vfs search fails
    MIRRORSTATUS status = STATUS_ENODEV;
    PSTR         cpy = NULL, cur = NULL;
    PVFS_NODE    node = NULL;
    PPATH        path = NULL;
    FILE         fd = NULL;

    if (!filepath || *filepath == '\0') {
        status = STATUS_EINVAL;
        goto out;
    }

    for (cpy = kzalloc(unbound_strlen(filepath) + 1), unbound_strcpy(cpy, filepath); *cpy; cpy[unbound_strlen(cpy) - 1] = 0)
        if ((node = vfs_exists(vfs_root_node, cpy))) {
            cur = cpy;
            // for a path like /home/a.txt, advance to a.txt
            while (*filepath++ == *cur++);

            // if file already opened, increment refcount
            // we wanna keep the /, and the previous line advances one too much
            // so back by 2
            if ((fd = (FILE)vfs_find_file_object(node, filepath - 2))) {
                ((PVFS_FILE)fd)->refcount++;
                *outfd = fd;
                status = STATUS_SUCCESS;
                goto out;
            }

            path = create_path(filepath - 2);

            // only allow opening of regular files
            if (node->ops->gettype(node->ops, path) != VFS_TYPE_REG_FILE)
            {
                status = STATUS_EINVAL;
                goto out;
            }

            // create an internal vfs file object and return ptr to user
            fd = (FILE)vfs_create_file_object(node, path, filepath - 2);
            if (!fd) {
                status = STATUS_EINVAL;
                goto out;
            }

            *outfd = fd;
            status = STATUS_SUCCESS;
            goto out;
        }

out:
    if (!MIRROR_SUCCESS(status))
        if (path)
            delete_path(path);

    if (cpy)
        kfree(cpy);
    return status;
}

void vfs_close(FILE fd)
{
    if (!fd)
        return;

    PVFS_FILE file_obj = (PVFS_FILE)fd;
    file_obj->refcount--;

    if (file_obj->refcount < 1)
        vfs_remove_file_object(file_obj);
    
    return;
}

PVFS_FILE vfs_create_file_object(PVFS_NODE node, PPATH path, PSTR strpath)
{
    PVFS_FILE file_obj = NULL, prev = NULL;
    PSTR      strpath_stor = kzalloc(unbound_strlen(strpath) + 1);

    file_obj = (PVFS_FILE)kzalloc(sizeof(VFS_FILE));
    if (!file_obj)
        return NULL;

    file_obj->refcount = 1;
    file_obj->node = node;
    file_obj->path = path;
    file_obj->strpath = strpath_stor;
    unbound_strcpy(file_obj->strpath, strpath);

    if (!vfs_root_file) {
        vfs_root_file = file_obj;
        file_obj->next = file_obj;
        file_obj->prev = file_obj;
    }
    else {
        // add back
        prev = vfs_root_file->prev;
        prev->next = file_obj;
        file_obj->prev = prev;
        file_obj->next = vfs_root_file;
        vfs_root_file->prev = file_obj;
    }

    return file_obj;
}

PVFS_FILE vfs_find_file_object(PVFS_NODE node, PSTR strpath)
{
    if (!vfs_root_file)
        return NULL;

    PVFS_FILE cur = vfs_root_file;

    do {
        if (cur->node == node && unbound_strcmp(strpath, cur->strpath) == 0)
            return cur;

        cur = cur->next;
    } while (cur != vfs_root_file);

    return NULL;
}

void vfs_remove_file_object(PVFS_FILE file_obj)
{
    PVFS_FILE prev, next;

    if (!file_obj || !file_obj->prev || !file_obj->next)
        return;

    if (vfs_root_file == file_obj)
        vfs_root_file = NULL;
    else {
        // unlink
        prev = file_obj->prev;
        next = file_obj->next;
        
        prev->next = next;
        next->prev = prev;
    }

    kfree(file_obj->strpath);
    delete_path(file_obj->path);
    
    kfree(file_obj);
    
    return;
}

MIRRORSTATUS vfs_read(FILE fd, PBYTE buf, DWORD offset, DWORD size)
{
    MIRRORSTATUS status = STATUS_SUCCESS;
    PVFS_FILE    file = (PVFS_FILE)fd;
    PVFS_NODE    node = NULL;
    PPATH        path = NULL;

    if (!buf || !file || !file->node || !file->path) {
        status = STATUS_EBADF;
        goto out;
    }

    node = file->node;
    path = file->path;

    status = node->ops->read(node->ops, path, buf, offset, size);

out:
    return status;
}