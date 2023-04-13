#include "status.h"
#include "fs/vfs.h"
#include "string/string.h"
#include "task/process.h"
#include "task/thread.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"

PPROCESS g_current_process;

// we'll do periodic checks in the scheduler to refresh this
WORD    g_current_usable_pid;

PPROCESS process_get_current_process(void)
{
    return g_current_process;
}

void process_set_current_process(PPROCESS process)
{
    g_current_process = process;
}

PPROCESS process_get_next_process(void)
{
    return g_current_process->next;
}

PTHREAD process_get_current_thread(PPROCESS process)
{
    return process->cur_thread;
}

void process_set_current_thread(PPROCESS process, PTHREAD thread)
{
    process->cur_thread = thread;
    return;
}

DWORD process_allocate_pid(void)
{
    DWORD pid = g_current_usable_pid;
    g_current_usable_pid++;

    return pid;
}

MIRRORSTATUS process_create_process(PSTR filepath, PPROCESS *out_process)
{
    MIRRORSTATUS status = STATUS_SUCCESS;
    DWORD        filesz = 0;
    FILE         fd = NULL;
    PBYTE        data = NULL;
    PTHREAD      main_thread = NULL;
    PPROCESS     process = NULL;
    PULONG       pagedir = NULL;
    DWORD        pages_to_commit = 0;

    if (!filepath || unbound_strlen(filepath) > MAX_PROCESS_NAME)
        return STATUS_EINVAL;

    // allocate process structure
    process = kzalloc(sizeof(PROCESS));
    if (!process) {
        status = STATUS_ENOMEM;
        goto out;
    }

    // attach to linked list
    process_link_process(process);
    
    // set attributes
    process->pid = process_allocate_pid();
    unbound_strcpy((PSTR)(process->process_name), filepath);
    process->available_stack_addr = (ULONG)DEFAULT_STACK_BASE;

    // create new pagedir
    status = paging_new_pagedir(&pagedir, PAGING_PRESENT | PAGING_RDWR | PAGING_USER_ACCESS);
    if (!MIRROR_SUCCESS(status))
        goto out;

    // get size of file
    status = vfs_open(filepath, &fd);
    if (!MIRROR_SUCCESS(status))
        goto out;

    status = vfs_getsize(fd, &filesz);
    if (!MIRROR_SUCCESS(status))
        goto out;

    // map sufficient pages to entry point and read
    pages_to_commit = (filesz / PAGING_PAGE_SZ) + 1;

    for (int i = 0; i < pages_to_commit; i++) {
        data = page_alloc();
        if (!data) {
            status = STATUS_ENOMEM;
            data = NULL;
            goto out;
        }

        status = vfs_read(fd, data, (ULONG)FILE_HEAD + i * PAGING_PAGE_SZ,\
            filesz - i * PAGING_PAGE_SZ);
        if (!MIRROR_SUCCESS(status))
            goto out;

        // phys memory disconnected but virt memory contiguous
        // beauty of paging
        status = paging_set_pagetable_entry(pagedir, (PVOID)((ULONG)DEFAULT_PROCESS_ENTRY + i * PAGING_PAGE_SZ),\
             (DWORD)data | PAGING_PRESENT | PAGING_CODE | PAGING_USER_ACCESS);
        if (!MIRROR_SUCCESS(status))
            goto out;
    }

    // attach pagedir to process
    process->page_dir = pagedir;

    // create thread suspended
    main_thread = thread_create_thread(process, (SUBROUTINE)DEFAULT_PROCESS_ENTRY, THREAD_SUSPEND);
    if (!main_thread) {
        status = STATUS_EFAULT;
        goto out;
    }

    // start thread
    thread_start_thread(main_thread);

    // set optional output if required
    // leads to dangling ptr when process exits though
    if (out_process)
        *out_process = process;

out:
    if (fd)
        vfs_close(fd);
    if (!MIRROR_SUCCESS(status)) {
        if (process)
            process_delete_process(process);
        if (pagedir)
            paging_delete_pagedir(pagedir);
        if (data)
            page_free(data);
    }

    return status;
}

void process_link_process(PPROCESS process)
{
    if (!g_current_process) {
        process->next = process;
        process->prev = process;
        g_current_process = process;
        return;
    }

    PPROCESS prev;

    // link to tail
    prev = g_current_process->prev;
    prev->next = process;
    process->next = g_current_process;

    return;
}

void process_delete_process(PPROCESS process)
{
    PPROCESS next, prev;

    next = process->next;
    prev = process->prev;

    // only process case not possible
    // system process is always running
    // and we should silently reject if it happens
    // to be the process getting terminated
    if (process->pid == SYSTEM_PID)
        return;

    // advance global process if necessary
    if (process_get_current_process() == process)
        process_set_current_process(process_get_next_process());

    // unlink first
    prev->next = next;
    next->prev = prev;

    // cleanup threads and pagedir
    for (PTHREAD t = process_get_current_thread(process); t; t = thread_shutdown_thread(t));

    paging_delete_pagedir(process->page_dir);

    // finally free process
    kfree(process);
    return;
}