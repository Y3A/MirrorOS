#include <stdbool.h>
#include "paging.h"
#include "status.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"

static uint32_t * cur_dir = NULL;
void paging_load_dir(uint32_t * pagedir);

PPAGE_CHUNK new_page_chunk(uint8_t flags)
{
    // one page chunk is one filled page directory
    uint32_t * new_pagedir = page_alloc();
    // 1024 pagedir entries
    for (int i = 0; i < PAGEDIR_TOTAL_ENTRIES; i++)
    {
        size_t offset = i * PAGETABLE_TOTAL_ENTRIES * PAGE_SZ;

        // 1024 pagetable entires per pagedir entry
        uint32_t * pagetable_entry = page_alloc();
        for (int j = 0; j < PAGETABLE_TOTAL_ENTRIES; j++)
            pagetable_entry[j] = ( (offset + (j * PAGE_SZ)) | flags );
        
        // set pagedir entries to writeable by default
        new_pagedir[i] = (uint32_t)pagetable_entry | flags | PAGING_RDWR;
    }

    // get a new page chunk and return a pointer to it
    PPAGE_CHUNK new_chunk = (PPAGE_CHUNK)kzalloc(sizeof(PAGE_CHUNK));
    new_chunk->pagedir = new_pagedir;

    return new_chunk;
}

void paging_switch_dir(uint32_t * pagedir)
// function to switch page directories
{
    paging_load_dir(pagedir);
    cur_dir = pagedir;
}

int paging_get_idx(void * vaddr, uint32_t * pagedir_idx, uint32_t * pagetable_idx)
{
    if (!paging_is_aligned(vaddr))
        return -EINVAL;
    
    *pagedir_idx = ( (uint32_t)vaddr / \
                (PAGETABLE_TOTAL_ENTRIES * PAGE_SZ ) );

    *pagetable_idx = ( ( (uint32_t)vaddr % \
                (PAGETABLE_TOTAL_ENTRIES * PAGE_SZ) ) \
                / PAGE_SZ );
    return 0;
}

bool paging_is_aligned(void * addr)
{
    return ((uint32_t)addr % PAGE_SZ) == 0;
}

int pagetable_set_entry(uint32_t * pagedir, void * vaddr, uint32_t physaddr_flags)
{
    if (!paging_is_aligned(vaddr))
        return -EINVAL;

    uint32_t pagedir_idx = 0;
    uint32_t pagetable_idx = 0;

    int res = paging_get_idx(vaddr, &pagedir_idx, &pagetable_idx);

    if (res < 0)
        return res;

    uint32_t pagedir_entry = pagedir[pagedir_idx];
    uint32_t * pagetable = (uint32_t *)(pagedir_entry & 0xfffff000);

    pagetable[pagetable_idx] = physaddr_flags;

    return 0;
}