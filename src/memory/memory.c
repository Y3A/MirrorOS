#include <stdint.h>
#include <stddef.h>
#include "memory.h"

uint32_t * cur_mmap = (uint32_t *)MMAP_START;

void * memset(void * ptr, int c, size_t size)
{
    char * base = (char *)ptr;
    for (int i = 0; i < size; i++)
        base[i] = (char)c;
    
    return (void *)base;
}

void * mmap(size_t size)
{
    uint32_t * old = cur_mmap;
    cur_mmap += size;

    return old; 
}