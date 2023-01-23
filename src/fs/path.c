#include "fs/path.h"
#include "memory/heap/kheap.h"

PPATH create_path(PSTR filepath)
{
    if (*filepath != '/') // expect absolute path!
        return NULL;

    PSTR  cur1 = filepath+1, cur2 = filepath+1;
    PPATH path = kzalloc(sizeof(PATH)), curpath = path;
    while (1) {
        if (*cur2 == '/' || *cur2 == '\0') {
            if (cur1 == cur2) {
                cur1++;
                cur2++;
                continue;
            }
            curpath->name = kzalloc(cur2-cur1+1);
            for (int i = 0; i < cur2-cur1; i++)
                curpath->name[i] = *(cur1+i);
            if (*cur2 == '\0')
                break;
            cur2++;
            cur1 = cur2;
            curpath->next = kzalloc(sizeof(PATH));
            curpath = curpath->next;
        }
        else
            cur2++;
    }

    if (!(path->name))
        return NULL;
    
    return path;
}

PSTR get_pathname(PPATH path)
{
    if (!path)
        return NULL;
    return path->name;
}

PPATH get_nextpath(PPATH path)
{
    if (!path)
        return NULL;
    return path->next;
}

VOID delete_path(PPATH path)
{
    if (!path || !path->name)
        return;

    PPATH cur = path, next = cur;
    while (cur) {
        next = cur->next;
        kfree(cur->name);
        kfree(cur);
        cur = next;
    }

    return;
}