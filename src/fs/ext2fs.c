#include "status.h"
#include "types.h"
#include "drivers/ata.h"
#include "fs/ext2fs.h"
#include "fs/vfs.h"
#include "memory/heap/kheap.h"

MIRRORSTATUS ext2fs_read_inode_metadata(PEXT2FS ext2fs, PEXT2FS_INODE inode_buf, ULONG inode_idx)
{
    MIRRORSTATUS status = STATUS_SUCCESS;
    DWORD inode_block_group;
    DWORD inode_table_block;
    DWORD inode_offset;
    ULONG inode_address;

    inode_block_group = (inode_idx - 1) / ext2fs->inodes_per_block_group; // because inodes start at 1
    inode_table_block = ext2fs->bgd[inode_block_group].bg_inode_table; // block group that conatins the table(array) of inodes
    inode_offset = (inode_idx - 1) % ext2fs->inodes_per_block_group; // index within the table
    inode_address = EXT2FS_BLOCK_OFFSET(inode_table_block, ext2fs->block_sz) \
    + sizeof(EXT2FS_INODE) * inode_offset;

    if (!MIRROR_SUCCESS(status = ata_read(inode_buf, sizeof(EXT2FS_INODE), inode_address, FILESYSTEM_DRIVE)))
        goto out;

out:
    return status;
}

MIRRORSTATUS ext2fs_write_inode_metadata(PEXT2FS ext2fs, PEXT2FS_INODE inode_buf, ULONG inode_idx)
{
    MIRRORSTATUS status = STATUS_SUCCESS;
    DWORD inode_block_group;
    DWORD inode_table_block;
    DWORD inode_offset;
    ULONG inode_address;

    inode_block_group = (inode_idx - 1) / ext2fs->inodes_per_block_group; // because inodes start at 1
    inode_table_block = ext2fs->bgd[inode_block_group].bg_inode_table;    // block group that conatins the table(array) of inodes
    inode_offset = (inode_idx - 1) % ext2fs->inodes_per_block_group;      // index within the table
    inode_address = EXT2FS_BLOCK_OFFSET(inode_table_block, ext2fs->block_sz) + sizeof(EXT2FS_INODE) * inode_offset;

    if (!MIRROR_SUCCESS(status = ata_write(inode_buf, sizeof(EXT2FS_INODE), inode_address, FILESYSTEM_DRIVE)))
        goto out;

out:
    return status;
}

MIRRORSTATUS ext2fs_init(PVFS_NODE ext2fs_node)
{
    MIRRORSTATUS        status = STATUS_SUCCESS;
    PEXT2FS             ext2fs;
    PEXT2FS_BGD         bgd;
    PEXT2FS_INODE       inode;
    ULONG               read_cur = SUPERBLOCK_OFFSET; // init cursor for superblock reading

    ext2fs = (PEXT2FS)kzalloc(sizeof(EXT2FS));
    if (!ext2fs) {
        status = STATUS_ENOMEM;
        goto out;
    }

    if (!MIRROR_SUCCESS(status = ata_read(&ext2fs->sb, SUPERBLOCK_SZ, read_cur, FILESYSTEM_DRIVE)))
        goto out;

    if (ext2fs->sb.s_magic != EXT2FS_SUPER_MAGIC) {
        status = STATUS_EINVAL; // not an ext2 fs
        goto out;
    }

    ext2fs->total_blocks = ext2fs->sb.s_blocks_count;
    ext2fs->total_inodes = ext2fs->sb.s_inode_count;
    ext2fs->blocks_per_block_group = ext2fs->sb.s_blocks_per_group;
    ext2fs->inodes_per_block_group = ext2fs->sb.s_inodes_per_group;
    ext2fs->block_sz = 1024 << ext2fs->sb.s_log_block_size;

    ext2fs->block_groups = ceil_div(ext2fs->total_blocks, ext2fs->blocks_per_block_group);
    if (ext2fs->block_groups != ceil_div(ext2fs->total_inodes, ext2fs->inodes_per_block_group)) {
        // https://wiki.osdev.org/Ext2#Determining_the_Number_of_Block_Groups
        status = STATUS_EINVAL; // corrupted drive
        goto out;
    }

    if (ext2fs->block_sz == 1024)
        read_cur = 2 * ext2fs->block_sz;
    else
        read_cur = 1 * ext2fs->block_sz; // bgd table is 1 block after superblock

    bgd = (PEXT2FS_BGD)kzalloc(sizeof(EXT2FS_BGD) * ext2fs->block_groups);
    if (!bgd) {
        status = STATUS_ENOMEM;
        goto out;
    }

    if (!MIRROR_SUCCESS(status = ata_read(bgd, \
    sizeof(EXT2FS_BGD) * ext2fs->block_groups, read_cur, FILESYSTEM_DRIVE)))
        goto out;

    ext2fs->bgd = bgd;

    inode = kzalloc(sizeof(EXT2FS_INODE));
    if (!inode) {
        status = STATUS_ENOMEM;
        goto out;
    }

    if (!MIRROR_SUCCESS(status = ext2fs_read_inode_metadata(ext2fs, inode, EXT2FS_ROOT_DIRECTORY_INODE)))
        goto out;

    ext2fs_make_vfs_node(ext2fs_node, ext2fs, inode, EXT2FS_ROOT_DIRECTORY_INODE);

out:
    if (!MIRROR_SUCCESS(status)) {
        if (bgd)
            kfree(bgd);

        if (inode)
            kfree(inode);

        if (ext2fs)
            kfree(ext2fs);
    }

    return status;
}

VOID ext2fs_make_vfs_node(PVFS_NODE ext2fs_node, PEXT2FS ext2fs, PEXT2FS_INODE inode, DWORD inode_idx)
{
    ext2fs_node->internal = ext2fs; // internal data that vfs will pass back
    if (inode->i_mode & EXT2_S_IFDIR)
        ext2fs_node->type |= VFS_DIRECTORY;

    if (inode->i_mode & EXT2_S_IRUSR)
        ext2fs_node->permissions |= VFS_USER_READ;

    if (inode->i_mode & EXT2_S_IWUSR)
        ext2fs_node->permissions |= VFS_USER_WRITE;

    if (inode->i_mode & EXT2_S_IXUSR)
        ext2fs_node->permissions |= VFS_USER_EXECUTE;

    ext2fs_node->create_time = inode->i_ctime;
    ext2fs_node->modified_time = inode->i_mtime;
    ext2fs_node->access_time = inode->i_atime;

    ext2fs_node->inode = inode_idx;

    ext2fs_node->open = ext2fs_open;
    ext2fs_node->close = ext2fs_close;

    return;
}

VOID ext2fs_open(PVOID internal, DWORD flags)
{
    return;
}

VOID ext2fs_close(PVOID internal)
{
    return;
}

VOID ext2fs_expand(PVOID internal, PVFS_NODE out_nodes, PDWORD out_nodes_count)
{
    return;
}