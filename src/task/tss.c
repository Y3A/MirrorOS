#include "tss.h"

void tss_init_tss(PTSS tss, DWORD stack_addr, DWORD data_selector)
{
    tss->ESP0 = stack_addr;
    tss->SS0 = data_selector;

    return;
}