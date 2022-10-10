#include "config.h"
#include "kernel.h"
#include "status.h"
#include "types.h"
#include "memory/heap/heap.h"
#include "memory/heap/kheap.h"

void kheap_init(void)
{
    if (!MIRROR_SUCCESS(heap_init((void *)KHEAP_START, KHEAP_SZ, (void *)KERNEL_HEAP_FREE_BIN_HEAD)))
    {    
        terminal_warn("[-]HEAP INIT FAILED\n");
        while (1) ;
    }
}

void * kmalloc(ULONG chunk_size)
{
    return heap_allocate((void *)KERNEL_HEAP_FREE_BIN_HEAD, chunk_size);
}

void * kzalloc(ULONG chunk_size)
{
    void * ptr = kmalloc(chunk_size);
    if (!ptr)
        return NULL;
        
    memset(ptr, 0, chunk_size);
    return ptr;
}

void kfree(void * addr)
{
    heap_free((void *)KERNEL_HEAP_FREE_BIN_HEAD, addr);
}
