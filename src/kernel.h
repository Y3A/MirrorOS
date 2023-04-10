#ifndef KERNEL_H
#define KERNEL_H

#include "types.h"

#define KERNEL_STACK_BASE 0x600000
#define MEMORY_BASE 0
#define MEMORY_LIMIT 0xffffffff

VOID kernel_main(VOID);
VOID kernel_panic(PCSTR msg);

#endif