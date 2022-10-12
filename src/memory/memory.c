#include "memory.h"
#include "types.h"

PVOID unbound_memset(PVOID ptr, INT c, ULONG size)
{
    PSTR base = (PSTR)ptr;
    for (INT i = 0; i < size; i++)
        base[i] = (CHAR)c;
    
    return (PVOID)base;
}

PVOID page_alloc(VOID)
{
    PBYTE start = (PBYTE)PAGE_ALLOC_TABLE;
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

PVOID page_alloc_zero(VOID)
{
    PVOID new_page = page_alloc();
    if (!new_page)
        return NULL;

    unbound_memset(new_page, 0, PAGE_SZ);
    
    return new_page;
}

VOID page_free(PVOID page)
{
    PBYTE start = (PBYTE)PAGE_ALLOC_TABLE;
    for (INT i = 0; i < MAX_PAGE_IDX; i++)
        if (*(start + i) == 1 && (PULONG)(PAGE_ALLOC_START + (i * PAGE_SZ)) == (PULONG)page)
            *(start + i) = 0;
}

INT unbound_memcmp(PVOID s1, PVOID s2, INT count)
{
    PSTR c1 = s1;
    PSTR c2 = s2;
    while (count-- > 0)
        if (*c1++ != *c2++)
            return c1[-1] < c2[-1] ? -1 : 1;
    return 0;
}

PVOID unbound_memcpy(PVOID dest, PVOID src, INT len)
{
    PSTR d = dest;
    PSTR s = src;
    for (INT i = 0; i < len; i++)
        *d++ = *s++;
        
    return d;
}