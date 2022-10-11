#ifndef PAGING_H
#define PAGING_H

#include "types.h"

#define PAGING_CACHE_DISABLED 0b10000
#define PAGING_WRITE_THROUGH  0b01000
#define PAGING_USER_ACCESS    0b00100
#define PAGING_RDWR           0b00010
#define PAGING_IS_PRESENT     0b00001

#define PAGING_PAGES_PER_PAGETABLE 1024
#define PAGING_PAGETABLES_PER_PAGEDIR 1024
#define PAGING_PAGE_SZ 4096

typedef struct
{
    PULONG pagedir;
} PAGE_CHUNK, *PPAGE_CHUNK;

// fetch pagetable and pagedir index given a virtual address
MIRRORSTATUS paging_get_pagedir_pagetable_idx(PVOID vaddr, PULONG pagedir_idx, PULONG pagetable_idx);
BOOL paging_is_aligned(PVOID addr);
// populate a pagetable entry to map a virtual address to a physical address with flags
MIRRORSTATUS paging_set_pagetable_entry(PULONG pagedir, PVOID vaddr, ULONG paddr_flags);

MIRRORSTATUS paging_new_pagechunk(PPAGE_CHUNK *out_pagechunk, WORD flags);
VOID paging_switch_pagedir(PULONG pagedir);
VOID paging_load_pagedir(PULONG pagedir);

// DO NOT ENABLE until we load(switch) a pagedir
VOID paging_enable_paging(VOID);

#endif