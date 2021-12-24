#ifndef STRING_H
#define STRING_H

#include "stdbool.h"
#include <stddef.h>

size_t strlen(const char * str);
int char2int(char c);
bool isdigit(char c);
size_t strnlen(const char * str, size_t max);
char * strcpy(char * dest, const char * src);
int strcmp (const char * str1, const char * str2);
char tolower(char s1);
int istrncmp(const char* s1, const char* s2, int n);

#endif