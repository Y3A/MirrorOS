#include "thread.h"
#include "memory/heap/kheap.h"

PTHREAD thread_get_next_thread(PTHREAD thread)
{
    return thread->next;
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