#include "status.h"
#include "gdt/gdt.h"
#include "task/process.h"
#include "task/thread.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"

PTHREAD thread_get_next_thread(PTHREAD thread)
{
    return thread->next;
}

PTHREAD thread_create_thread(PPROCESS parent_process, SUBROUTINE start, THREAD_CREATE_OPTIONS options)
{
    PTHREAD         thread = NULL, prev;
    ULONG           stack;
    PVOID           data;
    PULONG          pagedir = parent_process->page_dir;
    DWORD           pages_to_commit;
    MIRRORSTATUS    status;
    
    thread = kzalloc(sizeof(THREAD));
    if (!thread)
        return NULL;

    thread->parent_process = parent_process;

    switch (options) {
        case (THREAD_START):
            thread->state = THREAD_RUNNING;
            break;
        
        case (THREAD_SUSPEND):
            thread->state = THREAD_SUSPENDED;
            break;

        default:
            thread->state = THREAD_RUNNING;
    }

    // link thread
    if (!parent_process->cur_thread) {
        thread->next = thread;
        thread->prev = thread;
        parent_process->cur_thread = thread;
    }
    else {
        // add to tail
        prev = parent_process->cur_thread->prev;
        prev->next = thread;
        thread->next = parent_process->cur_thread;
    }

    // thread id is process bound
    if (thread->prev->tid)
        thread->tid = thread->prev->tid + 1;
    else
        thread->tid = 0;


    // allocate stack
    pages_to_commit = ((DWORD)MAX_STACK_SIZE / PAGING_PAGE_SZ);
    stack = parent_process->available_stack_addr;
    // update next available stack address for next thread
    if (parent_process->available_stack_addr == (ULONG)DEFAULT_STACK_BASE)
        parent_process->available_stack_addr = (ULONG)ALT_STACK_START;
    else
        parent_process->available_stack_addr += ((DWORD)MAX_STACK_SIZE + PAGING_PAGE_SZ);

    for (int i = 0; i < pages_to_commit; i++) {
        data = page_alloc();
        if (!data) {
            // no mem left
            if (thread)
                thread_shutdown_thread(thread);
            return NULL;
        }
        status = paging_set_pagetable_entry(pagedir, (PVOID)((ULONG)stack - i * PAGING_PAGE_SZ),
                                            (DWORD)data | PAGING_PRESENT | PAGING_RDWR | PAGING_USER_ACCESS);
        if (!MIRROR_SUCCESS(status)) {
            if (thread)
                thread_shutdown_thread(thread);
            return NULL;
        }
    }

    // set regs
    thread->regs.ss = GDT_USER_DATA;
    thread->regs.cs = GDT_USER_CS;
    thread->regs.eip = start;
    thread->regs.esp = stack;
    thread->regs.ebp = stack;

    return thread;
}

void thread_start_thread(PTHREAD thread)
{
    thread->state = THREAD_RUNNING;
    return;
}

PTHREAD thread_shutdown_thread(PTHREAD thread)
{
    // returns next thread or NULL
    PTHREAD next, prev;

    next = thread->next;
    prev = thread->prev;

    if ((DWORD)next == (DWORD)prev && (DWORD)prev == (DWORD)thread) {
        // only thread
        kfree(thread);
        return NULL;
    }

    // unlink
    prev->next = next;
    next->prev = prev;

    kfree(thread);
    return next;
}