#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

#define PAGE_ALLOC_START 0x2100000
#define PAGE_ALLOC_MAX 0x2500000
#define MAX_PAGE_IDX 4096*10
#define PAGE_ALLOC_TABLE 0x00007f00
#define PAGE_SZ 4096

// kernel memory functions, usermode code eventually call these after checks
PVOID unbound_memset(PVOID ptr, INT c, ULONG size);
INT unbound_memcmp(PVOID s1, PVOID s2, INT count);
PVOID unbound_memcpy(PVOID dest, PVOID src, INT len);

PVOID page_alloc(VOID);
PVOID page_alloc_zero(VOID);
VOID page_free(PVOID page);

#endif