#include "status.h"
#include "types.h"
#include "drivers/ata.h"
#include "fs/ext2fs.h"
#include "memory/heap/kheap.h"

MIRRORSTATUS ext2fs_init(VOID)
{
    MIRRORSTATUS        status = STATUS_SUCCESS;
    EXT2FS              ext2fs = { 0 };
    PEXT2FS_SUPERBLOCK  sb = (PEXT2FS_SUPERBLOCK)kzalloc(SUPERBLOCK_SZ);
    PEXT2FS_BGD         bgd = NULL;
    ULONG               read_cur = SUPERBLOCK_OFFSET; // init cursor for superblock reading


    if (!sb) {
        status = STATUS_ENOMEM;
        goto out;
    }

    if (!MIRROR_SUCCESS(status = ata_read(sb, SUPERBLOCK_SZ, read_cur, SLAVE_DRIVE)))
        goto out;

    if (sb->s_magic != EXT2FS_SUPER_MAGIC) {
        status = STATUS_EINVAL; // not an ext2 fs
        goto out;
    }

    ext2fs.sb = sb;
    ext2fs.total_blocks = sb->s_blocks_count;
    ext2fs.total_inodes = sb->s_inode_count;
    ext2fs.blocks_per_block_group = sb->s_blocks_per_group;
    ext2fs.inodes_per_block_group = sb->s_inodes_per_group;
    ext2fs.block_sz = 1024 << sb->s_log_block_size;

    ext2fs.block_groups = ceil_div(ext2fs.total_blocks, ext2fs.blocks_per_block_group);
    if (ext2fs.block_groups != ceil_div(ext2fs.total_inodes, ext2fs.inodes_per_block_group)) {
        // https://wiki.osdev.org/Ext2#Determining_the_Number_of_Block_Groups
        status = STATUS_EINVAL; // corrupted drive
        goto out;
    }

    if (ext2fs.block_sz == 1024)
        read_cur = 2 * ext2fs.block_sz;
    else
        read_cur = 1 * ext2fs.block_sz; // bgd table is 1 block after superblock

    bgd = (PEXT2FS_BGD)kzalloc(sizeof(EXT2FS_BGD) * ext2fs.block_groups);
    if (!bgd) {
        status = STATUS_ENOMEM;
        goto out;
    }

    if (!MIRROR_SUCCESS(status = ata_read(bgd, \
    sizeof(EXT2FS_BGD) * ext2fs.block_groups, read_cur, SLAVE_DRIVE)))
        goto out;

    // work in progress;

out:
    if (sb)
        kfree(sb);

    if (bgd)
        kfree(bgd);

    return status;
}