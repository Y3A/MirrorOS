#ifndef TSS_H
#define TSS_H

#include "types.h"

typedef struct
{
    // Task State Segment
    // https://wiki.osdev.org/Task_State_Segment
    DWORD LINK;
    DWORD ESP0;
    DWORD SS0;
    DWORD ESP1;
    DWORD SS1;
    DWORD ESP2;
    DWORD SS2;
    DWORD CR3;
    DWORD EIP;
    DWORD EFLAGS;
    DWORD EAX;
    DWORD ECX;
    DWORD EDX;
    DWORD EBX;
    DWORD ESP;
    DWORD EBP;
    DWORD ESI;
    DWORD EDI;
    DWORD ES;
    DWORD CS;
    DWORD SS;
    DWORD DS;
    DWORD FS;
    DWORD GS;
    DWORD LDTR;
    WORD  _reserved;
    WORD  IOPB;
    DWORD SSP;
} PACKED TSS, *PTSS;

void tss_init_tss(PTSS tss, DWORD stack_addr, DWORD data_selector);
void tss_load_tss(DWORD tss_segment);

#endif