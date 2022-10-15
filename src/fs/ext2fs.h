#ifndef EXT2FS_H
#define EXT2FS_H

#include "types.h"

typedef struct _EXT2FS_SUPERBLOCK
{
    /* https://www.nongnu.org/ext2-doc/ext2.html#superblock */
    /* size: 1024 */

    ULONG s_inode_count;
    ULONG s_blocks_count;
    ULONG s_r_blocks_count;
    ULONG s_free_blocks_count;
    ULONG s_free_inodes_count;
    ULONG s_first_data_block;
    ULONG s_log_block_size;
    ULONG s_log_frag_size;
    ULONG s_blocks_per_group;
    ULONG s_frags_per_group;
    ULONG s_inodes_per_group;
    ULONG s_mtime;
    ULONG s_wtime;
    WORD s_mnt_count;
    WORD s_max_mnt_count;
    WORD s_magic;
    WORD s_state;
    WORD s_errors;
    WORD s_minor_rev_level;
    ULONG s_lastcheck;
    ULONG s_checkinterval;
    ULONG s_creator_os;
    ULONG s_rev_level;
    WORD s_def_resuid;
    WORD s_def_resgid;

    ULONG s_first_ino;
    WORD s_inode_size;
    WORD s_block_group_nr;
    ULONG s_feature_compat;
    ULONG s_feature_incompat;
    ULONG s_feature_ro_compat;
    BYTE s_uuid[16];
    BYTE s_volume_name[16];
    BYTE s_last_mounted[64];
    ULONG s_algo_bitmap;

    BYTE s_prealloc_blocks;
    BYTE s_prealloc_dir_blocks;
    WORD alignment1;

    BYTE s_journal_uuid[16];
    ULONG s_journal_inum;
    ULONG s_journal_dev;
    ULONG s_last_orphan;

    ULONG s_hash_seed[4];
    BYTE s_def_hash_version;
    BYTE padding1[3];

    ULONG s_default_mount_options;
    ULONG s_first_meta_bg;
    BYTE unused1[760];

} __attribute__((__packed__)) EXT2FS_SUPERBLOCK, *PEXT2FS_SUPERBLOCK;

#define SUPERBLOCK_SZ (sizeof(EXT2FS_SUPERBLOCK))
#define SUPERBLOCK_OFFSET 1024

#define EXT2FS_SUPER_MAGIC 0xEF53

#define EXT2FS_BLOCK_OFFSET(block, block_sz) (SUPERBLOCK_OFFSET + (block - 1) * block_sz)

#define ceil_div(x, y) (x/y + (x%y != 0))

typedef struct _EXT2FS_BGD
{
    /* https://www.nongnu.org/ext2-doc/ext2.html#block-group-descriptor-structure */
    /* size: 32 */

    ULONG bg_block_bitmap;
    ULONG bg_inode_bitmap;
    ULONG bg_inode_table;
    WORD bg_free_blocks_count;
    WORD bg_free_inodes_count;
    WORD bg_used_dirs_count;
    WORD bg_pad;
    BYTE bg_reserved[12];
} __attribute__((__packed__)) EXT2FS_BGD, *PEXT2FS_BGD;

typedef struct _EXT2FS
{
    PEXT2FS_SUPERBLOCK sb;
    ULONG block_sz;
    ULONG total_inodes;
    ULONG total_blocks;
    ULONG block_groups;
    ULONG blocks_per_block_group;
    ULONG inodes_per_block_group;
} EXT2FS, *PEXT2FS;

// just to please the compiler
typedef EXT2FS EXT2FS;
typedef EXT2FS_SUPERBLOCK EXT2FS_SUPERBLOCK;
typedef PEXT2FS PEXT2FS;
typedef EXT2FS_BGD EXT2FS_BGD;
typedef PEXT2FS_BGD PEXT2FS_BGD;

MIRRORSTATUS ext2fs_init(VOID);

#endif