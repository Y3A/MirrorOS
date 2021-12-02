#ifndef PATHPARSER_H
#define PATHPARSER_H

typedef struct _PATH_PART
{
    const char * part;
    struct _PATH_PART * next;
} PATH_PART, *PPATH_PART;


typedef struct
{
    int drive_no;
    PPATH_PART first;
} PATH_ROOT, *PPATH_ROOT;

void path_cleanup(PPATH_ROOT root);
PPATH_ROOT parse(const char * path, const char * cur_dir_path);


#endif