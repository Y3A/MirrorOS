#include "config.h"
#include "status.h"
#include "types.h"
#include "memory/heap/heap.h"
#include "memory/heap/kheap.h"

MIRRORSTATUS kheap_init(void)
{
    return heap_init((PVOID)KHEAP_START, KHEAP_SZ, (PVOID)KERNEL_HEAP_FREE_BIN_HEAD);
}

PVOID kmalloc(ULONG chunk_size)
{
    return heap_allocate((PVOID)KERNEL_HEAP_FREE_BIN_HEAD, chunk_size);
}

PVOID kzalloc(ULONG chunk_size)
{
    PVOID ptr = kmalloc(chunk_size);
    if (!ptr)
        return NULL;
        
    unbound_memset(ptr, 0, chunk_size);
    return ptr;
}

VOID kfree(PVOID addr)
{
    heap_free((PVOID)KERNEL_HEAP_FREE_BIN_HEAD, addr);
}
