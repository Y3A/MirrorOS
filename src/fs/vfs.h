#ifndef VFS_H
#define VFS_H

#define VFS_FILE            0x01
#define VFS_DIRECTORY       0x02
#define VFS_CHARDEVICE      0x04
#define VFS_BLOCKDEVICE     0x08
#define VFS_PIPE            0x10
#define VFS_SYMLINK         0x20
#define VFS_MOUNTPOINT      0x40

#define VFS_USER_READ       0x0100 // user read
#define VFS_USER_WRITE      0x0080 // user write
#define VFS_USER_EXECUTE    0x0040 // user execute

#endif