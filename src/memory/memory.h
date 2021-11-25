#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

#define PAGE_ALLOC_START 0x2100000
#define PAGE_ALLOC_MAX 0x2500000
#define MAX_PAGE_IDX 1024
#define PAGE_ALLOC_TABLE 0x00007f00
#define PAGE_SZ 4096

void * memset(void * ptr, int c, size_t size);

void * page_alloc(void);
void page_free(void * page);

#endif