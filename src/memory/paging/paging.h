#ifndef PAGING_H
#define PAGING_H

#include "types.h"

#define PAGING_MAPPED         0b100000

#define PAGING_USER_ACCESS    0b000100
#define PAGING_RDWR           0b000010
#define PAGING_IS_PRESENT     0b000001

#define PAGING_PAGES_PER_PAGETABLE 1024
#define PAGING_PAGETABLES_PER_PAGEDIR 1024
#define PAGING_PAGE_SZ 4096

#define unmask_addr(addr) (addr & 0xfffff000)

// fetch pagetable and pagedir index given a virtual address
MIRRORSTATUS paging_get_pagedir_pagetable_idx(PVOID vaddr, PULONG pagedir_idx, PULONG pagetable_idx);
BOOL paging_is_aligned(PVOID addr);
// populate a pagetable entry to map a virtual address to a physical address with flags
MIRRORSTATUS paging_set_pagetable_entry(PULONG pagedir, PVOID vaddr, ULONG paddr_flags);

MIRRORSTATUS paging_new_pagedir(PULONG *out_pagedir, WORD flags);
VOID paging_switch_pagedir(PULONG pagedir);
VOID paging_load_pagedir(PULONG pagedir);

VOID paging_delete_pagedir(PULONG pagedir);

// DO NOT ENABLE until we load(switch) a pagedir
VOID paging_enable_paging(VOID);

#endif