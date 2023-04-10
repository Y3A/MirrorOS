#include "process.h"
#include "thread.h"
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