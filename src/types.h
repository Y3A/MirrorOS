#ifndef TYPES_H
#define TYPES_H

#define TRUE ((BOOL)(1))
#define FALSE ((BOOL)(0))

#ifndef NULL
#define NULL ((PVOID)(0))
#endif

typedef signed char CHAR;
typedef CHAR *PCHAR;

typedef signed char INT8;
typedef INT8 *PINT8;

typedef unsigned char UCHAR;
typedef UCHAR *PUCHAR;

typedef unsigned char UINT8;
typedef UINT8 *PUINT8;

typedef unsigned char BYTE;
typedef BYTE *PBYTE;

typedef signed short SHORT;
typedef SHORT *PSHORT;

typedef signed short INT16;
typedef INT16 *PINT16;

typedef unsigned short USHORT;
typedef USHORT *PUSHORT;

typedef unsigned short UINT16;
typedef UINT16 *PUINT16;

typedef unsigned short WORD;
typedef WORD *PWORD;

typedef signed int INT;
typedef INT *PINT;

typedef signed int INT32;
typedef INT32 *PINT32;

typedef signed int LONG;
typedef LONG *PLONG;

typedef unsigned int UINT;
typedef UINT *PUINT;

typedef unsigned int UINT32;
typedef UINT32 *PUINT32;

typedef unsigned int DWORD;
typedef DWORD *PDWORD;

typedef unsigned int ULONG;
typedef ULONG *PULONG;

typedef char *PSTR;

typedef const char *PCSTR;

typedef void *PVOID;

typedef const void *PCVOID;

typedef int BOOL;

typedef void VOID;

typedef INT MIRRORSTATUS;

#endif