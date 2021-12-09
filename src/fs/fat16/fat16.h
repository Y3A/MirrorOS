#ifndef FAT16_H
#define FAT16_h

#include "disk/streamer.h"
#include "fs/file.h"

#define FAT16_SIGNATURE          0x29
#define FAT16_BAD_SECTOR         0xff7
#define FAT16_UNUSED             0x0

// fat directory entry attributes bitmask

#define FAT16_READ_ONLY          0x1
#define FAT16_HIDDEN             0x2
#define FAT16_SYSTEM             0x4
#define FAT16_VOLUME_ID          0x8
#define FAT16_DIRECTORY          0x10
#define FAT16_ARCHIVE            0x20

typedef struct _fat16_header_extended
{
	//extended fat12 and fat16 stuff
	unsigned char		bios_drive_num;
	unsigned char		reserved1;
	unsigned char		boot_signature;
	unsigned int		volume_id;
	unsigned char		volume_label[11];
	unsigned char		fat_type_label[8];
 
}__attribute__((packed)) FAT16_HEADER_EXT, *PFAT16_HEADER_EXT;

typedef struct _fat16_header
{

	unsigned char 		bootjmp[3];
	unsigned char 		oem_name[8];
	unsigned short 	    bytes_per_sector;
	unsigned char		sectors_per_cluster;
	unsigned short		reserved_sector_count;
	unsigned char		number_of_fat;
	unsigned short		root_entry_count;
	unsigned short		total_sectors_small;
	unsigned char		media_type;
	unsigned short		sectors_per_fat;
	unsigned short		sectors_per_track;
	unsigned short		head_side_count;
	unsigned int 		hidden_sector_count;
	unsigned int 		total_sectors_large;
 
	FAT16_HEADER_EXT    extended_header;

}__attribute__((packed)) FAT16_HEADER, *PFAT16_HEADER;

typedef struct _fat16_item
{

    char filename[8];
    char ext[3];
    unsigned char attribute;
    unsigned char reserved1;
    unsigned char creation_time_tenth_of_sec;
    unsigned short creation_time;
    unsigned short creation_date;
    unsigned short last_access_date;
    unsigned short zero_field;
    unsigned short last_modified_time;
    unsigned short last_modified_date;
    unsigned short first_cluster_number;
    unsigned int item_size;

}__attribute__((packed)) FAT16_ITEM, *PFAT16_ITEM;

typedef struct _fat16_item_descriptor
{
    
    PFAT16_ITEM item;
    unsigned int seek_pos;

}__attribute__((packed)) FAT16_ITEM_DESCRIPTOR, *PFAT16_ITEM_DESCRIPTOR;

typedef struct _fat16_dir
{

    PFAT16_ITEM dir_as_file;
    int total_items;
    int start_sector;

}__attribute__((packed)) FAT16_DIR, *PFAT16_DIR;

typedef struct _fat16_internal
{

    // for internal use
    FAT16_HEADER header;
    PFAT16_DIR root_dir;
    PSTREAM cluster_read_stream;
    PSTREAM fat16_read_stream;

}__attribute__((packed)) FAT16_INTERNAL, *PFAT16_INTERNAL;

PFILESYSTEM fat16_init(void);
int fat16_resolve(PDISK disk);
void * fat16_open(PDISK disk, PPATH_PART path, FILE_MODE mode);

#endif