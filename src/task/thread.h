#ifndef THREAD_H
#define THREAD_H

#include "types.h"

#define DEFAULT_STACK_BASE 0x3ff000

typedef enum
{
    THREAD_RUNNING,
    THREAD_SUSPENDED,
    THREAD_WAITING
} STATE;

typedef enum
{
    THREAD_START,
    THREAD_SUSPEND
} THREAD_CREATE_OPTIONS;

typedef struct
{
    // general purpose
    DWORD eax;
    DWORD ebx;
    DWORD eci;
    DWORD edx;
    DWORD esi;
    DWORD edi;

    // specific
    DWORD eip;
    DWORD ebp;
    DWORD esp;
    DWORD cs;
    DWORD ss;
    DWORD eflags;
} REGISTERS, *PREGISTERS;

typedef struct _THREAD
{
    // thread struct
    // here we follow the windows philosophy
    // where processes are containers and do nothing
    // threads are the actual runners
    WORD                tid;
    REGISTERS           regs;
    STATE               state;
    PPROCESS            parent_process;

    // circular doubly linked list of all threads in the process
    struct _THREAD      *next;
    struct _THREAD      *prev;

} THREAD, *PTHREAD;

PTHREAD thread_get_next_thread(PTHREAD thread);

PTHREAD thread_create_thread(PPROCESS parent_process, SUBROUTINE start, THREAD_CREATE_OPTIONS options);
void    thread_start_thread(PTHREAD thread);
PTHREAD thread_shutdown_thread(PTHREAD thread);

#endif