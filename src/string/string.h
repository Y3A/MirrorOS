#ifndef STRING_H
#define STRING_H

#include "types.h"

ULONG strlen(PCSTR str);
INT char2int(CHAR c);
BOOL isdigit(CHAR c);
ULONG strnlen(PCSTR str, ULONG max);
PSTR strcpy(PSTR dest, PCSTR src);
INT strcmp (PCSTR str1, PCSTR str2);
CHAR tolower(CHAR s1);
INT istrncmp(PCSTR s1, PCSTR s2, INT n);

#endif