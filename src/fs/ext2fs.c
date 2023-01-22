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

VOID ext2fs_open(PVOID internal, DWORD flags)
{
    return;
}

VOID ext2fs_close(PVOID internal)
{
    return;
}