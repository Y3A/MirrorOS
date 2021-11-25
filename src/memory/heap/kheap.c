#include "memory/heap/heap.h"
#include "memory/heap/kheap.h"
#include "kernel.h"
#include "config.h"

void kheap_init(void)
{
    if ( heap_init((void *)KHEAP_START, KHEAP_SZ, (void *)KERNEL_HEAP_FREE_BIN_HEAD) )
    {    
        terminal_warn("[-]HEAP INIT FAILED\n");
        while (1) ;
    }
}

void * kmalloc(size_t chunk_size)
{
    return heap_allocate((void *)KERNEL_HEAP_FREE_BIN_HEAD, chunk_size);
}

void * kzalloc(size_t chunk_size)
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
