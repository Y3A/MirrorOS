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