#include "memory.h"

void * memset(void * ptr, int c, size_t size)
{
    char * base = (char *)ptr;
    for (int i = 0; i < size; i++)
        base[i] = (char)c;
    
    return (void *)base;
}

void * page_alloc(void)
{
    char * start = (char *)PAGE_ALLOC_TABLE;
    for ( int i = 0; i < MAX_PAGE_IDX; i++ )
    {
        if ( *(start + i) == 0)
        {
            *(start + i) = 1;
            return (void *)(PAGE_ALLOC_START + (PAGE_SZ * i));
        }
    }
    return NULL;
}

void page_free(void * page)
{
    char * start = (char *)PAGE_ALLOC_TABLE;
    for ( int i = 0; i < MAX_PAGE_IDX; i++ )
        if ( (start + (i * PAGE_SZ)) == (char *)page )
            *(start + i) = 0;
}