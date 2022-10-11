#ifndef HEAP_H
#define HEAP_H

#include "types.h"

#define HEAP_MIN_CHUNK_SZ 0x20
#define HEAP_CHUNK_ALIGN_SZ 0x10
#define HEAP_ALIGN_SZ 0x1000
#define HEAP_PAGE_SZ 0x1000

#define HEAP_START 0x1000000
#define HEAP_MAX 0x2000000

#define PREV_INUSE 1

#define chunksize_nomask(s) ((s) &~ PREV_INUSE)
#define prev_inuse(s) ((s) & PREV_INUSE)

MIRRORSTATUS heap_init(PVOID start, ULONG heap_size, PVOID free_bin_head);
PVOID heap_allocate(PVOID free_bin_head, ULONG chunk_size);
ULONG heap_align_heap_chunk(ULONG chunk_size);

PVOID heap_find_available(PVOID free_bin_head, ULONG chunk_size);
VOID heap_free(PVOID free_bin_head, PVOID chunk_addr);
ULONG chunksize_at_mem(PVOID addr);
VOID unlink(PVOID cur);

VOID chunk_set_size(PVOID addr, ULONG chunk_size);
VOID chunk_set_fd(PVOID addr, PVOID fd);
VOID chunk_set_bk(PVOID addr, PVOID bk);

PVOID chunk_fd(PVOID cur);
PVOID chunk_bk(PVOID cur);

#endif