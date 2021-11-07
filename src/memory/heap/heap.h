#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>

#define MIN_CHUNK_SZ 0x20
#define CHUNK_ALIGN_SZ 0x10
#define HEAP_ALIGN_SZ 0x1000
#define HEAP_PAGE_SZ 0x1000

#define HEAP_START 0x1000000
#define HEAP_MAX 0x2000000

#define HEAP_FREE_CHUNK_STORAGE 0x2000000

#define PREV_INUSE 1

#define chunksize_nomask(s) ((s) &~ PREV_INUSE)
#define prev_inuse(s) ((s) & PREV_INUSE)

int heap_init(void * start, size_t heap_size, void * free_bin_head);
void * heap_allocate(void * free_bin_head, size_t chunk_size);
size_t align_heap_chunks(size_t chunk_size);

void * heap_find_available(void * free_bin_head, size_t chunk_size);
void heap_free(void * free_bin_head, void * chunk_addr);
size_t chunksize_at_mem(void * addr);
void unlink(void * cur);

void chunk_set_size(void * addr, size_t chunk_size);
void chunk_set_fd(void * addr, void * fd);
void chunk_set_bk(void * addr, void * bk);

void * chunk_fd(void * cur);
void * chunk_bk(void * cur);

#endif