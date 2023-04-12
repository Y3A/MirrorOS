#include "task/process.h"
#include "task/thread.h"
#include "memory/heap/kheap.h"

PTHREAD thread_get_next_thread(PTHREAD thread)
{
    return thread->next;
}

PTHREAD thread_create_thread(PPROCESS parent_process, SUBROUTINE start, THREAD_CREATE_OPTIONS options)
{
    PTHREAD thread, prev;
    
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

    // allocate stack, use default if not mapped yet, else find another free address

    // set regs
    

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