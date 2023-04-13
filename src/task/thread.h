#ifndef THREAD_H
#define THREAD_H

#include "types.h"

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
    DWORD eax;    // 0
    DWORD ebx;    // 4
    DWORD ecx;    // 8
    DWORD edx;    // 12
    DWORD esi;    // 16
    DWORD edi;    // 20

    // specific
    DWORD eip;    // 24
    DWORD ebp;    // 28
    DWORD esp;    // 32
    DWORD cs;     // 36
    DWORD ss;     // 40
    DWORD eflags; // 44
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
    struct _PROCESS     *parent_process;

    // circular doubly linked list of all threads in the process
    struct _THREAD      *next;
    struct _THREAD      *prev;

} THREAD, *PTHREAD;

PTHREAD thread_get_next_thread(PTHREAD thread);

PTHREAD thread_create_thread(struct _PROCESS *parent_process, SUBROUTINE start, THREAD_CREATE_OPTIONS options);
void    thread_start_thread(PTHREAD thread);
PTHREAD thread_shutdown_thread(PTHREAD thread);

#endif