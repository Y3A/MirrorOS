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
    WORD        tid;
    REGISTERS   regs;
    STATE       state;

    // singly linked list of all threads in the process
    _THREAD     *next;

} THREAD, *PTHREAD;

#endif