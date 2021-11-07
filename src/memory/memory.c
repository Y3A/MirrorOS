#include "memory.h"

void * memset(void * ptr, int c, size_t size)
{
    char * base = (char *)ptr;
    for (int i = 0; i < size; i++)
        base[i] = (char)c;
    
    return (void *)base;
}