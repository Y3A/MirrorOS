#include <stdbool.h>
#include <stddef.h>
#include "string/string.h"

bool isdigit(char c)
{
    return c >= 48 && c <= 57;
}

size_t strlen(const char * str)
{
    size_t ptr = 0;
    while (str[ptr++]) ;
    return ptr;
}

size_t strnlen(const char * str, size_t max)
{
    size_t ptr = 0;
    for (ptr = 0; ptr < max && str[ptr]; ptr++) ;
    return ptr;
}

int char2int(char c)
{
    return c - 48;
}

char * strcpy(char * dest, const char * src)
{
    while (*src)
        *(dest++) = *(src++);
    *dest = 0;
    
    return dest;
}

int strcmp(const char * str1, const char * str2)
{
    while (*str1 && *str2)
        if (*str1++ != *str2++)
            return str1[-1] < str2[-1] ? -1 : 1;
    return 0;
}

char tolower(char s1)
{
    if (s1 >= 65 && s1 <= 90)
    {
        s1 += 32;
    }

    return s1;
}

int istrncmp(const char* s1, const char* s2, int n)
{
    unsigned char u1, u2;
    while(n-- > 0)
    {
        u1 = (unsigned char)*s1++;
        u2 = (unsigned char)*s2++;
        if (u1 != u2 && tolower(u1) != tolower(u2))
            return u1 - u2;
        if (u1 == '\0')
            return 0;
    }

    return 0;
}