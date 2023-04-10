#ifndef PROCESS_H
#define PROCESS_H

#define SYSTEM_PID             0

#define MAX_PROCESS_NAME       255
#define MAX_STACK_SIZE         16 * 1024 // 16kb
#define DEFAULT_PROCESS_ENTRY  0x400000
#define DEFAULT_STACK_BASE     0x3ff000

#include "thread.h"
#include "types.h"

typedef struct _PROCESS
{
    WORD                pid;
    CHAR                process_name[MAX_PROCESS_NAME + 1];
    PULONG              page_dir;
    // circular doubly linked list of threads
    PTHREAD             cur_thread;

    // circular doubly linked list of all processes on the system
    struct _PROCESS     *next;
    struct _PROCESS     *prev;

} PROCESS, *PPROCESS;

// cycle through processes for scheduling
PPROCESS process_get_current_process(void);
void process_set_current_process(PPROCESS process);
PPROCESS process_get_next_process(void);

// cycle through threads for scheduling
PTHREAD process_get_current_thread(PPROCESS process);
void process_set_current_thread(PPROCESS process, PTHREAD thread);

void process_delete_process(PPROCESS process);

#endif