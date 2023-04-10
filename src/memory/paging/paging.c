#include "paging.h"
#include "status.h"
#include "types.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"

MIRRORSTATUS paging_new_pagedir(PULONG *out_pagedir, WORD flags)
{
    // outputs a filled page dir
    MIRRORSTATUS    status = STATUS_SUCCESS;
    PULONG          new_pagedir = NULL;
    ULONG           pagetable_offset = 0;
    PULONG          new_pagetable = NULL;

    new_pagedir = page_alloc_zero();
    if (!new_pagedir) {
        status = STATUS_ENOMEM;
        goto out;
    }

    // 1024 pagetables per pagedir
    for (ULONG pagetable_idx = 0; pagetable_idx < PAGING_PAGETABLES_PER_PAGEDIR; pagetable_idx++)
    {
        new_pagetable = page_alloc_zero();
        if (!new_pagetable) {
            status = STATUS_ENOMEM;
            goto out;
        }

        pagetable_offset = pagetable_idx * PAGING_PAGES_PER_PAGETABLE * PAGE_SZ;

        // 1024 pages per pagetable
        for (ULONG page_idx = 0; page_idx < PAGING_PAGES_PER_PAGETABLE; page_idx++)
            // default PTEs point to themselves, until we set them to point to virtual memory
            new_pagetable[page_idx] = ((pagetable_offset + (page_idx * PAGE_SZ)) | flags);
        
        // set pagetables to writeable by default
        new_pagedir[pagetable_idx] = (DWORD)new_pagetable | flags | PAGING_RDWR;
    }

    *out_pagedir = new_pagedir;

out:
    if (!MIRROR_SUCCESS(status)) {
        if (new_pagedir) {
            for (ULONG pagetable_idx = 0; pagetable_idx < PAGING_PAGETABLES_PER_PAGEDIR; pagetable_idx++)
                if (new_pagedir[pagetable_idx])
                    page_free((PVOID)unmask_addr(new_pagedir[pagetable_idx]));

        page_free((PVOID)new_pagedir);
        }
    }
    return status;
}

VOID paging_delete_pagedir(PULONG pagedir)
{
    // free all pages(and physical memory) associated with a pagedir
    PULONG cur_pagetable;
    ULONG  cur_page, cur_pagetable_offset, cur_page_offset;

    if (!pagedir)
        return;

    for (ULONG pagetable_idx = 0; pagetable_idx < PAGING_PAGETABLES_PER_PAGEDIR; pagetable_idx++) {
        cur_pagetable = (PULONG)unmask_addr(pagedir[pagetable_idx]);
        cur_pagetable_offset = pagetable_idx * PAGING_PAGES_PER_PAGETABLE * PAGE_SZ;
        for (ULONG page_idx = 0; page_idx < PAGING_PAGES_PER_PAGETABLE; page_idx++) {
            cur_page = cur_pagetable[page_idx];
            cur_page_offset = cur_pagetable_offset + (page_idx * PAGE_SZ);
            // free all physical memory associated with the page
            // our poor implementation meant that we can free the same page twice
            // so it's fine to just go with a dumb loop
            if (unmask_addr(cur_page) != cur_page_offset && cur_page & PAGING_MAPPED)
                page_free((PVOID)unmask_addr(cur_page));
        }
        page_free((PVOID)cur_pagetable);
    }

    page_free((PVOID)pagedir);
    return;
}

VOID paging_switch_pagedir(PULONG pagedir)
{
    // switch page directories
    paging_load_pagedir(pagedir);
}

MIRRORSTATUS paging_get_pagedir_pagetable_idx(PVOID vaddr, PULONG pagedir_idx, PULONG pagetable_idx)
{
    MIRRORSTATUS status = STATUS_SUCCESS;

    if (!paging_is_aligned(vaddr)) {
        status = STATUS_EINVAL;
        goto out;
    }
    
    *pagedir_idx = ((ULONG)vaddr / \
                (PAGING_PAGES_PER_PAGETABLE * PAGE_SZ ));

    *pagetable_idx = (((ULONG)vaddr % \
                (PAGING_PAGES_PER_PAGETABLE * PAGE_SZ)) \
                / PAGE_SZ);

out:
    return status;
}

BOOL paging_is_aligned(PVOID addr)
{
    return ((ULONG)addr % PAGE_SZ) == 0;
}

MIRRORSTATUS paging_set_pagetable_entry(PULONG pagedir, PVOID vaddr, ULONG paddr_flags)
{
    MIRRORSTATUS    status = STATUS_SUCCESS;
    ULONG           pagedir_idx = 0, pagetable_idx = 0;

    if (!paging_is_aligned(vaddr)) {
        status = STATUS_EINVAL;
        goto out;
    }

    status = paging_get_pagedir_pagetable_idx(vaddr, &pagedir_idx, &pagetable_idx);
    if (!MIRROR_SUCCESS(status))
        goto out;

    ULONG pagedir_entry = pagedir[pagedir_idx];
    ULONG pagetable = (ULONG)unmask_addr(pagedir_entry); // mask off flag bits

    // add on mapped flag so we can cleanup
    ((PULONG)pagetable)[pagetable_idx] = paddr_flags | PAGING_MAPPED;

out:
    return status;
}

PVOID paging_align_address(PVOID ptr)
{
    DWORD delta;

    if ((delta = (ULONG)ptr % PAGING_PAGE_SZ))
        // upsize to page size
        return (PVOID)((ULONG)ptr + PAGING_PAGE_SZ - delta);

    return ptr;
}