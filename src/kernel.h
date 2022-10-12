#ifndef KERNEL_H
#define KERNEL_H

#include "types.h"

VOID kernel_main(VOID);
VOID kernel_panic(PCSTR msg);

#endif