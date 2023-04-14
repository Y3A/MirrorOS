#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "task/thread.h"

void scheduler_schedule_next_thread(void);
void scheduler_update_thread_regs(PINTERRUPT_FRAME frame);

// asm funcs
void scheduler_load_user_segments(void);
void scheduler_load_kernel_segments(void);
void scheduler_run_thread(REGISTERS *regs);

// test
// void scheduler_test_run(void);

#endif