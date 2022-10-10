#include "memory.h"
#include "types.h"

PVOID memset(PVOID ptr, INT c, ULONG size)
{
    PSTR base = (PSTR)ptr;
    for (INT i = 0; i < size; i++)
        base[i] = (char)c;
    
    return (PVOID)base;
}

PVOID page_alloc(VOID)
{
    PSTR start = (PSTR)PAGE_ALLOC_TABLE;
    for (INT i = 0; i < MAX_PAGE_IDX; i++)
    {
        if (*(start + i) == 0)
        {
            *(start + i) = 1;
            return (PVOID)(PAGE_ALLOC_START + (PAGE_SZ * i));
        }
    }
    return NULL;
}

VOID page_free(PVOID page)
{
    PSTR start = (PSTR)PAGE_ALLOC_TABLE;
    for (INT i = 0; i < MAX_PAGE_IDX; i++)
        if ((start + (i * PAGE_SZ)) == (PSTR)page)
            *(start + i) = 0;
}

INT memcmp(PVOID s1, PVOID s2, INT count)
{
    PSTR c1 = s1;
    PSTR c2 = s2;
    while (count-- > 0)
        if (*c1++ != *c2++)
            return c1[-1] < c2[-1] ? -1 : 1;
    return 0;
}

PVOID memcpy(PVOID dest, PVOID src, INT len)
{
    PSTR d = dest;
    PSTR s = src;
    for (INT i = 0; i < len; i++)
        *d++ = *s++;
        
    return d;
}