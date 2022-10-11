#ifndef STATUS_H
#define STATUS_H

#include "types.h"

#define STATUS_SUCCESS       0 /* Operation was successful */
#define	STATUS_EPERM        -1	/* Operation not permitted */
#define	STATUS_ENOENT       -2	/* No such file or directory */
#define	STATUS_ESRCH        -3	/* No such process */
#define	STATUS_EINTR        -4	/* Interrupted system call */
#define	STATUS_EIO          -5	/* I/O error */
#define	STATUS_ENXIO        -6	/* No such device or address */
#define	STATUS_E2BIG        -7	/* Argument list too long */
#define	STATUS_ENOEXEC      -8	/* Exec format error */
#define	STATUS_EBADF        -9	/* Bad file number */
#define	STATUS_ECHILD       -10	/* No child processes */
#define	STATUS_EAGAIN       -11	/* Try again */
#define	STATUS_ENOMEM       -12	/* Out of memory */
#define	STATUS_EACCES       -13	/* Permission denied */
#define	STATUS_EFAULT       -14	/* Bad address */
#define	STATUS_ENOTBLK      -15	/* Block device required */
#define	STATUS_EBUSY        -16	/* Device or resource busy */
#define	STATUS_EEXIST       -17	/* File exists */
#define	STATUS_EXDEV        -18	/* Cross-device link */
#define	STATUS_ENODEV       -19	/* No such device */
#define	STATUS_ENOTDIR      -20	/* Not a directory */
#define	STATUS_EISDIR       -21	/* Is a directory */
#define	STATUS_EINVAL       -22	/* Invalid argument */
#define	STATUS_ENFILE       -23	/* File table overflow */
#define	STATUS_EMFILE       -24	/* Too many open files */
#define	STATUS_ENOTTY       -25	/* Not a typewriter */
#define	STATUS_ETXTBSY      -26	/* Text file busy */
#define	STATUS_EFBIG        -27	/* File too large */
#define	STATUS_ENOSPC       -28	/* No space left on device */
#define	STATUS_ESPIPE       -29	/* Illegal seek */
#define	STATUS_EROFS        -30	/* Read-only file system */
#define	STATUS_EMLINK       -31	/* Too many links */
#define	STATUS_EPIPE        -32	/* Broken pipe */
#define	STATUS_EDOM         -33	/* Math argument out of domain of func */
#define	STATUS_ERANGE       -34	/* Math result not representable */

#define MIRROR_STATUS(value) ((INT)value)
#define MIRROR_SUCCESS(value) (MIRROR_STATUS(value) >= 0)

#endif