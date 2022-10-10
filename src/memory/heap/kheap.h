#ifndef KHEAP_H
#define KHEAP_H

#include "heap.h"
#include "types.h"
#include "memory/memory.h"

#define KHEAP_START HEAP_START
#define KHEAP_SZ 1024*1024

void kheap_init(void);

void * kmalloc(ULONG chunk_size);
void kfree(void * addr);
void * kzalloc(ULONG chunk_size);

#endif