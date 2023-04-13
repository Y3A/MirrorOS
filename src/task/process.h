#ifndef PROCESS_H
#define PROCESS_H

#define SYSTEM_PID             0

#define MAX_PROCESS_NAME       255
#define DEFAULT_PROCESS_ENTRY  0x400000
#define DEFAULT_STACK_BASE     0x3ff000
#define ALT_STACK_START        0X1000000
#define MAX_STACK_SIZE         16 * 1024 // 16kb

#include "types.h"

typedef struct _PROCESS
{
    WORD                pid;
    CHAR                process_name[MAX_PROCESS_NAME + 1];
    PULONG              page_dir;
    ULONG               available_stack_addr;
    // circular doubly linked list of threads
    struct _THREAD      *cur_thread;

    // circular doubly linked list of all processes on the system
    struct _PROCESS     *next;
    struct _PROCESS     *prev;

} PROCESS, *PPROCESS;

#include "thread.h"

// cycle through processes for scheduling
PPROCESS process_get_current_process(void);
void process_set_current_process(PPROCESS process);
PPROCESS process_get_next_process(void);

// cycle through threads for scheduling
PTHREAD process_get_current_thread(PPROCESS process);
void process_set_current_thread(PPROCESS process, PTHREAD thread);

MIRRORSTATUS process_create_process(PSTR filepath, PPROCESS *out_process);
void process_delete_process(PPROCESS process);

// internal
void process_link_process(PPROCESS process);
DWORD process_allocate_pid(void);

#endif