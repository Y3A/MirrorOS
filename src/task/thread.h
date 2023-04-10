#ifndef THREAD_H
#define THREAD_H

#include "types.h"

typedef enum
{
    THREAD_RUNNING,
    THREAD_SLEEPING,
    THREAD_WAITING
} STATE;

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

    // circular doubly linked list of all threads in the process
    struct _THREAD      *next;
    struct _THREAD      *prev;

} THREAD, *PTHREAD;

PTHREAD thread_get_next_thread(PTHREAD thread);

PTHREAD thread_shutdown_thread(PTHREAD thread);

#endif