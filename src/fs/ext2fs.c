#include "status.h"
#include "types.h"
#include "drivers/ata.h"
#include "fs/ext2fs.h"
#include "fs/vfs.h"
#include "memory/heap/kheap.h"

DWORD pow(DWORD n1, DWORD n2)
{
    DWORD res = 1;
    while (n2--)
        res *= n1;
    
    return res;
}

MIRRORSTATUS ext2fs_read_inode_metadata(PEXT2FS ext2fs, PEXT2FS_INODE inode_buf, ULONG inode_idx)
{
    MIRRORSTATUS    status = STATUS_SUCCESS;
    DWORD           inode_block_group;
    DWORD           inode_table_block;
    DWORD           inode_offset;
    ULONG           inode_address;

    inode_block_group = (inode_idx - 1) / ext2fs->inodes_per_block_group; // because inodes start at 1
    inode_table_block = ext2fs->bgd[inode_block_group].bg_inode_table; // block group that conatins the table(array) of inodes
    inode_offset = (inode_idx - 1) % ext2fs->inodes_per_block_group; // index within the table
    inode_address = EXT2FS_BLOCK_OFFSET(inode_table_block, ext2fs->block_sz) \
    + sizeof(EXT2FS_INODE) * inode_offset;

    if (!MIRROR_SUCCESS(status = ata_read_filesystem(inode_buf, sizeof(EXT2FS_INODE), inode_address)))
        goto out;

out:
    return status;
}

MIRRORSTATUS ext2fs_write_inode_metadata(PEXT2FS ext2fs, PEXT2FS_INODE inode_buf, ULONG inode_idx)
{
    MIRRORSTATUS    status = STATUS_SUCCESS;
    DWORD           inode_block_group;
    DWORD           inode_table_block;
    DWORD           inode_offset;
    ULONG           inode_address;

    inode_block_group = (inode_idx - 1) / ext2fs->inodes_per_block_group; // because inodes start at 1
    inode_table_block = ext2fs->bgd[inode_block_group].bg_inode_table;    // block group that conatins the table(array) of inodes
    inode_offset = (inode_idx - 1) % ext2fs->inodes_per_block_group;      // index within the table
    inode_address = EXT2FS_BLOCK_OFFSET(inode_table_block, ext2fs->block_sz) + sizeof(EXT2FS_INODE) * inode_offset;

    if (!MIRROR_SUCCESS(status = ata_write_filesystem(inode_buf, sizeof(EXT2FS_INODE), inode_address)))
        goto out;

out:
    return status;
}

MIRRORSTATUS ext2fs_read_inode_filedata(PEXT2FS ext2fs, IN PEXT2FS_INODE inode_buf, IN ULONG offset, IN ULONG size, OUT PBYTE buf)
{
    MIRRORSTATUS status = STATUS_SUCCESS;
    DWORD        max_blocks_reserved;
    DWORD        offset_blocks, offset_remainder, cur = 0;
    DWORD        block_sz = ext2fs->block_sz;
    DWORD        drive_offset, read_sz;

    max_blocks_reserved = inode_buf->i_blocks / (2 << ext2fs->sb.s_log_block_size);
    offset_blocks = offset / block_sz;
    offset_remainder = offset - (offset_blocks * block_sz);

    if ((max_blocks_reserved <= 12 && offset_blocks+1 > max_blocks_reserved)
        || (max_blocks_reserved > 12 && offset_blocks+1 > (12 + pow((block_sz / 4), max_blocks_reserved-12))))
    {
        // too large an offset specified!
        status = STATUS_EINVAL;
        goto out;
    }

    for (int i = 0; i < max_blocks_reserved; i++) {
        if (size <= 0 || inode_buf->i_block[i] == 0) // Done reading
            break;

        if (i < 12) {
            // Direct block
            if (offset_blocks > 0) {
                offset_blocks--;
                continue;
            }
            // read sector, from offset_remainder
            drive_offset = (inode_buf->i_block[i]) * block_sz;
            if (offset_remainder) {
                read_sz = size > block_sz - offset_remainder ? block_sz - offset_remainder : size;
                drive_offset += offset_remainder;
                offset_remainder = 0;
            }
            else
                read_sz = size > block_sz ? block_sz : size;
            
            ata_read_filesystem(buf + cur, read_sz, drive_offset);
            cur += read_sz;
            size -= read_sz;
        }

        else if (i == 12) {
            // First level indirect block
        }

        else if (i == 13) {
            // Second level indirect block
        }

        else if (i == 14) {
            // Third level indirect block
        }
    }

out:
    return status;
}

PVOID ext2fs_init(DWORD drive_offset)
{
    MIRRORSTATUS    status = STATUS_SUCCESS;
    PEXT2FS         ext2fs;
    PEXT2FS_BGD     bgd;
    PEXT2FS_INODE   inode;
    ULONG           read_cur = drive_offset + SUPERBLOCK_OFFSET; // init cursor for superblock reading

    ext2fs = (PEXT2FS)kzalloc(sizeof(EXT2FS));
    if (!ext2fs)
    {
        status = STATUS_ENOMEM;
        goto out;
    }

    if (!MIRROR_SUCCESS(status = ata_read_filesystem(&ext2fs->sb, SUPERBLOCK_SZ, read_cur)))
        goto out;

    if (ext2fs->sb.s_magic != EXT2FS_SUPER_MAGIC)
    {
        status = STATUS_EINVAL; // not an ext2 fs
        goto out;
    }

    ext2fs->total_blocks = ext2fs->sb.s_blocks_count;
    ext2fs->total_inodes = ext2fs->sb.s_inode_count;
    ext2fs->blocks_per_block_group = ext2fs->sb.s_blocks_per_group;
    ext2fs->inodes_per_block_group = ext2fs->sb.s_inodes_per_group;
    ext2fs->block_sz = 1024 << ext2fs->sb.s_log_block_size;

    ext2fs->block_groups = ceil_div(ext2fs->total_blocks, ext2fs->blocks_per_block_group);
    if (ext2fs->block_groups != ceil_div(ext2fs->total_inodes, ext2fs->inodes_per_block_group))
    {
        // https://wiki.osdev.org/Ext2#Determining_the_Number_of_Block_Groups
        status = STATUS_EINVAL; // corrupted drive
        goto out;
    }

    if (ext2fs->block_sz == 1024)
        read_cur = 2 * ext2fs->block_sz;
    else
        read_cur = 1 * ext2fs->block_sz; // bgd table is 1 block after superblock

    bgd = (PEXT2FS_BGD)kzalloc(sizeof(EXT2FS_BGD) * ext2fs->block_groups);
    if (!bgd)
    {
        status = STATUS_ENOMEM;
        goto out;
    }

    if (!MIRROR_SUCCESS(status = ata_read_filesystem(bgd,
        sizeof(EXT2FS_BGD) * ext2fs->block_groups, read_cur)))
        goto out;

    ext2fs->bgd = bgd;

    inode = kzalloc(sizeof(EXT2FS_INODE));
    if (!inode)
    {
        status = STATUS_ENOMEM;
        goto out;
    }

    if (!MIRROR_SUCCESS(status = ext2fs_read_inode_metadata(ext2fs, inode, EXT2FS_ROOT_DIRECTORY_INODE)))
        goto out;

    ext2fs->root = inode;

    // Operations
    ext2fs->ops.open = ext2fs_open;
    ext2fs->ops.close = ext2fs_close;

out:
    if (!MIRROR_SUCCESS(status))
    {
        if (bgd)
            kfree(bgd);

        if (inode)
            kfree(inode);

        if (ext2fs)
            kfree(ext2fs);

        return NULL;
    }

    return ext2fs;
}

VOID ext2fs_open(PVOID internal, DWORD flags)
{
    return;
}

VOID ext2fs_close(PVOID internal)
{
    return;
}