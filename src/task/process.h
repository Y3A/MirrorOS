#ifndef PROCESS_H
#define PROCESS_H

#define MAX_PROCESS_NAME       255
#define MAX_STACK_SIZE         16 * 1024 // 16kb
#define DEFAULT_PROCESS_ENTRY  0x400000
#define DEFAULT_STACK_BASE     0x3ff000

#include "thread.h"
#include "types.h"

typedef struct _PROCESS
{
    WORD        pid;
    CHAR        process_name[MAX_PROCESS_NAME + 1];
    PULONG      page_dir;
    PTHREAD     thread_head;

    // circular doubly linked list of all processes on the system
    _PROCESS    *next;
    _PROCESS    *prev;

} PROCESS, *PPROCESS;

#endif