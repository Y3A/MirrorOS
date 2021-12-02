#ifndef STRING_H
#define STRING_H

#include "stdbool.h"
#include <stddef.h>

size_t strlen(const char * str);
int char2int(char c);
bool isdigit(char c);
size_t strnlen(const char * str, size_t max);

#endif