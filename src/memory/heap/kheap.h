#ifndef KHEAP_H
#define KHEAP_H

#include "heap.h"
#include "memory/memory.h"

#define KHEAP_START HEAP_START
#define KHEAP_SZ 1024*1024

void kheap_init(void);

void * kmalloc(size_t chunk_size);
void kfree(void * addr);
void * kzalloc(size_t chunk_size);

#endif