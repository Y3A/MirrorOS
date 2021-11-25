#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

#define MMAP_START 0x2100000
#define MMAP_MAX 0x3000000

void * memset(void * ptr, int c, size_t size);
void * mmap(size_t size);

#endif