#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <stddef.h>

#define PAGING_CACHE_DISABLED 0b10000
#define PAGING_WRITE_THROUGH  0b01000
#define PAGING_USER_ACCESS    0b00100
#define PAGING_RDWR           0b00010
#define PAGING_IS_PRESENT        0b00001

#define PAGETABLE_TOTAL_ENTRIES 1024
#define PAGEDIR_TOTAL_ENTRIES 1024
#define PAGE_SZ 4096

typedef struct
{
    uint32_t * pagedir_entry;
} PAGE_CHUNK, * PPAGE_CHUNK;

PPAGE_CHUNK new_page_chunk(uint8_t flags);
void paging_switch_dir(uint32_t * pagedir);

// DO NOT ENABLE until we load(switch) a pagedir
void paging_enable(void);

#endif