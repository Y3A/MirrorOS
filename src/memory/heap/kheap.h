#ifndef KHEAP_H
#define KHEAP_H

#include "heap.h"
#include "types.h"
#include "memory/memory.h"

#define KHEAP_START HEAP_START
#define KHEAP_SZ 1024*1024

MIRRORSTATUS kheap_init(VOID);

PVOID kmalloc(ULONG chunk_size);
VOID kfree(PVOID addr);
PVOID kzalloc(ULONG chunk_size);

#endif