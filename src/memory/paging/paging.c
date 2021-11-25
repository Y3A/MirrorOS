#include "paging.h"
#include "memory/memory.h"

static uint32_t * cur_dir = NULL;
void paging_load_dir(uint32_t * pagedir);

PPAGE_CHUNK new_page_chunk(uint8_t flags)
{
    // one page chunk is one filled page directory
    uint32_t * new_pagedir_entry = mmap(sizeof(uint32_t) * PAGEDIR_TOTAL_ENTRIES);
    // 1024 pagedir entries
    for (int i = 0; i < PAGEDIR_TOTAL_ENTRIES; i++)
    {
        size_t offset = i * PAGETABLE_TOTAL_ENTRIES * PAGE_SZ;

        // 1024 pagetable entires per pagedir entry
        uint32_t * pagetable_entry = mmap(sizeof(uint32_t) * PAGETABLE_TOTAL_ENTRIES);
        for (int j = 0; j < PAGETABLE_TOTAL_ENTRIES; j++)
            pagetable_entry[j] = ( (offset + (j * PAGE_SZ)) | flags );
        
        // set pagedir entries to writeable by default
        new_pagedir_entry[i] = (uint32_t)pagetable_entry | flags | PAGING_RDWR;
    }

    // get a new page chunk and return a pointer to it
    PPAGE_CHUNK new_chunk = (PPAGE_CHUNK)mmap(sizeof(PAGE_CHUNK));
    new_chunk->pagedir_entry = new_pagedir_entry;

    return new_chunk;
}

void paging_switch_dir(uint32_t * pagedir)
// function to switch page directories
{
    paging_load_dir(pagedir);
    cur_dir = pagedir;
}