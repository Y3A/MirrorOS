#include "types.h"
#include "string/string.h"

BOOL isdigit(CHAR c)
{
    return c >= 48 && c <= 57;
}

ULONG unbound_strlen(PCSTR str)
{
    ULONG ptr = 0;
    while (str[ptr++]) ;
    return ptr-1;
}

ULONG unbound_strnlen(PCSTR str, ULONG max)
{
    ULONG ptr = 0;
    for (ptr = 0; ptr < max && str[ptr]; ptr++) ;
    return ptr;
}

INT char2int(CHAR c)
{
    return c - 48;
}

PSTR unbound_strcpy(PSTR dest, PCSTR src)
{
    while (*src)
        *(dest++) = *(src++);
    *dest = 0;
    
    return dest;
}

INT unbound_strcmp(PCSTR str1, PCSTR str2)
{
    while (*str1 && *str2)
        if (*str1++ != *str2++)
            return str1[-1] < str2[-1] ? -1 : 1;
    return 0;
}

CHAR tolower(CHAR s1)
{
    if (s1 >= 65 && s1 <= 90)
    {
        s1 += 32;
    }

    return s1;
}

INT unbound_istrncmp(PCSTR s1, PCSTR s2, INT n)
{
    BYTE u1, u2;

    while(n-- > 0)
    {
        u1 = (BYTE)*s1++;
        u2 = (BYTE)*s2++;
        if (u1 != u2 && (tolower(u1) != tolower(u2)))
            return u1 - u2;
        if (u1 == '\0')
            return 0;
    }

    return 0;
}