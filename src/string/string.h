#ifndef STRING_H
#define STRING_H

#include "types.h"

ULONG unbound_strlen(PCSTR str);
INT char2int(CHAR c);
BOOL isdigit(CHAR c);
ULONG unbound_strnlen(PCSTR str, ULONG max);
PSTR unbound_strcpy(PSTR dest, PCSTR src);
INT unbound_strcmp (PCSTR str1, PCSTR str2);
CHAR tolower(CHAR s1);
INT unbound_istrncmp(PCSTR s1, PCSTR s2, INT n);

#endif