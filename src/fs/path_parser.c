#include "fs/path_parser.h"
#include "kernel.h"
#include "status.h"
#include "memory/heap/kheap.h"
#include "config.h"
#include "stddef.h"
#include "string/string.h"
#include "memory/memory.h"

/*
 * example valid path: 0:/a/b.txt
 * where 0 is drive number
 */

/*
 * PATH_ROOT and PATH_PART structs are like linked lists
 * PATH_ROOT contains drive number and pointer to first PATH_PART
 * PATH_PART contains its respective part(a string) and pointer to next part
 */

static int path_valid_format(const char * path);
static int get_drive_by_path(const char ** path);
static PPATH_ROOT create_root(int drive_no);
static const char * get_path_part(const char ** path);

PPATH_PART parse_part(PPATH_PART prev, const char ** path);

static int path_valid_format(const char * path)
{
    size_t len = strnlen(path, MAX_PATH);
    return ( len >= 3  && isdigit(path[0]) && !memcmp((void *)&path[1], ":/", 2) );
}

static int get_drive_by_path(const char ** path)
{
    if (!path_valid_format(*path))
        return STATUS_EINVAL;

    int drive_no = char2int(**path);

    *path += 3; // skip drive_no

    return drive_no;
}

static PPATH_ROOT create_root(int drive_no)
{
    PPATH_ROOT root = (PPATH_ROOT)kzalloc(sizeof(PATH_ROOT));
    root->drive_no = drive_no;
    root->first = NULL;

    return root;
}

static const char * get_path_part(const char ** path)
{
    int i = 0;
    char * part = (char *)kzalloc(MAX_PATH);
    while (**path != '/' && (**path))
        part[i++] = *(*path)++;
    
    while (**path == '/')
        // skip forward slash
        (*path)++;

    if (!i)
    {
        kfree(part);
        part = NULL;
    }

    return part;

}

PPATH_PART parse_part(PPATH_PART prev, const char ** path)
{
    const char * str = get_path_part(path);
    if (!str)
        return NULL;

    PPATH_PART newpart = (PPATH_PART)kzalloc(sizeof(PATH_PART));   
    newpart->part = str;
    newpart->next = NULL;

    if (prev)
        prev->next = newpart;

    return newpart;
}

void path_cleanup(PPATH_ROOT root)
{
    /* kzalloc-ed a bunch of path parts and paths
     * cleaning up here
     */

    PPATH_PART cur =root->first;
    PPATH_PART prev;
    while (cur)
    {
        prev = cur;
        cur = cur->next;
        kfree((void *)prev->part);
        kfree(prev);
    }

    kfree(root);
}

PPATH_ROOT parse(const char * path, const char * cur_dir_path)
{
    const char * copy = path;
    PPATH_ROOT root = NULL;
    PPATH_PART first_part = NULL;
    PPATH_PART next_parts = NULL;

    if (strlen(path) > MAX_PATH)
        return NULL;

    int ret = get_drive_by_path(&copy);

    if (ret < 0)
        return NULL;

    root = create_root(ret);

    if (!root)
        return NULL;

    first_part = parse_part(NULL, &copy);

    if (!first_part)
        return NULL;

    root->first = first_part;
    next_parts = parse_part(first_part, &copy);

    while (next_parts)
        next_parts = parse_part(next_parts, &copy);

    return root;
}