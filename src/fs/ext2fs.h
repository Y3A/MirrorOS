#ifndef EXT2FS_H
#define EXT2FS_H

#include "types.h"
#include "drivers/ata.h"
#include "fs/vfs.h"

#define EXT2FS_ROOT_DIRECTORY_INODE 2

// -- file format --
#define EXT2_S_IFSOCK       0xC000 // socket
#define EXT2_S_IFLNK        0xA000 // symbolic link
#define EXT2_S_IFREG        0x8000 // regular file
#define EXT2_S_IFBLK        0x6000 // block device
#define EXT2_S_IFDIR        0x4000 // directory
#define EXT2_S_IFCHR        0x2000 // character device
#define EXT2_S_IFIFO        0x1000 // fifo
// -- process execution user/group override --
#define EXT2_S_ISUID        0x0800 // Set process User ID
#define EXT2_S_ISGID        0x0400 // Set process Group ID
#define EXT2_S_ISVTX        0x0200 // sticky bit
// -- access rights --
#define EXT2_S_IRUSR        0x0100 // user read
#define EXT2_S_IWUSR        0x0080 // user write
#define EXT2_S_IXUSR        0x0040 // user execute
#define EXT2_S_IRGRP        0x0020 // group read
#define EXT2_S_IWGRP        0x0010 // group write
#define EXT2_S_IXGRP        0x0008 // group execute
#define EXT2_S_IROTH        0x0004 // others read
#define EXT2_S_IWOTH        0x0002 // others write
#define EXT2_S_IXOTH        0x0001 // others execute

// file format dir entry
#define EXT2_FT_UNKNOWN 0
#define EXT2_FT_REG_FILE 1
#define EXT2_FT_DIR 2
#define EXT2_FT_CHRDEV 3
#define EXT2_FT_BLKDEV 4
#define EXT2_FT_FIFO 5
#define EXT2_FT_SOCK 6
#define EXT2_FT_SYMLINK 7

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

typedef struct _EXT2FS_INODE
{
    WORD i_mode;
    WORD i_uid;
    ULONG i_size;
    ULONG i_atime;
    ULONG i_ctime;
    ULONG i_mtime;
    ULONG i_dtime;
    WORD i_gid;
    WORD i_links_count;
    ULONG i_blocks;
    ULONG i_flags;
    ULONG i_osd1;
    ULONG i_block[15];
    ULONG i_generation;
    ULONG i_file_acl;
    ULONG i_dir_acl;
    ULONG i_faddr;
    BYTE i_osd2[12];
} __attribute__((__packed__)) EXT2FS_INODE, *PEXT2FS_INODE;

typedef struct _EXT2FS
{
    VFS_OPS ops;
    EXT2FS_SUPERBLOCK sb;
    PEXT2FS_BGD bgd;
    ULONG block_sz;
    ULONG total_inodes;
    ULONG total_blocks;
    ULONG block_groups;
    ULONG blocks_per_block_group;
    ULONG inodes_per_block_group;
    PEXT2FS_INODE root;
} __attribute__((__packed__)) EXT2FS, *PEXT2FS;

typedef struct
{
    DWORD inode;
    WORD rec_len;
    BYTE name_len;
    BYTE file_type;
    CHAR name[1];
} __attribute__((__packed__)) EXT2FS_DIR_ENTRY, *PEXT2FS_DIR_ENTRY;

// just to please the compiler
typedef EXT2FS EXT2FS;
typedef EXT2FS_SUPERBLOCK EXT2FS_SUPERBLOCK;
typedef PEXT2FS PEXT2FS;
typedef EXT2FS_BGD EXT2FS_BGD;
typedef PEXT2FS_BGD PEXT2FS_BGD;

PVOID ext2fs_init(DWORD drive_offset);

// Populate empty inode_buf with inode metadata
MIRRORSTATUS ext2fs_read_inode_metadata(PEXT2FS ext2fs, OUT PEXT2FS_INODE inode_buf, IN ULONG inode_idx);
// Write populated inode_buf to disk
MIRRORSTATUS ext2fs_write_inode_metadata(PEXT2FS ext2fs, IN PEXT2FS_INODE inode_buf, IN ULONG inode_idx);

// Given populated inode_buf, read file data referenced by that inode
MIRRORSTATUS ext2fs_read_inode_filedata(PEXT2FS ext2fs, IN PEXT2FS_INODE inode_buf, IN ULONG offset, IN ULONG size, OUT PBYTE buf);

// Given populated inode_buf, write file data to that inode
MIRRORSTATUS ext2fs_write_inode_filedata(PEXT2FS ext2fs, IN PEXT2FS_INODE inode_buf, IN ULONG offset, IN ULONG size, IN PBYTE buf);

// Given path, get inode index of last part
DWORD ext2fs_get_inode_idx(PEXT2FS ext2fs, PPATH path);

// Operations
MIRRORSTATUS ext2fs_open(PVOID internal, PPATH path, DWORD flags);
VOID ext2fs_close(PVOID internal);
MIRRORSTATUS ext2fs_read(PVOID internal, PPATH path, PBYTE buf, DWORD offset, DWORD size);
VFS_TYPE ext2fs_gettype(PVOID internal, PPATH path);

#endif