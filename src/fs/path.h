#ifndef PATH_H
#define PATH_H

#include "types.h"

typedef struct _PATH
{
    PSTR name;
    struct _PATH *next;
} PATH, *PPATH;

PPATH create_path(PSTR filepath);
PSTR get_pathname(PPATH path);
PPATH get_nextpath(PPATH path);
VOID delete_path(PPATH path);

#endif