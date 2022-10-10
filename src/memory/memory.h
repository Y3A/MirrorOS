#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

#define PAGE_ALLOC_START 0x2100000
#define PAGE_ALLOC_MAX 0x2500000
#define MAX_PAGE_IDX 1024
#define PAGE_ALLOC_TABLE 0x00007f00
#define PAGE_SZ 4096

PVOID memset(PVOID ptr, INT c, ULONG size);
INT memcmp(PVOID s1, PVOID s2, INT count);
PVOID memcpy(PVOID dest, PVOID src, INT len);

PVOID page_alloc(VOID);
VOID page_free(PVOID page);

#endif