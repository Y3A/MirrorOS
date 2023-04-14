#include "idt/idt.h"
#include "task/process.h"
#include "task/scheduler.h"
#include "memory/paging/paging.h"

void scheduler_schedule_next_thread(void)
{
    ;
}

void scheduler_update_thread_regs(PINTERRUPT_FRAME frame)
{
    PPROCESS cur_proc;
    PTHREAD  cur_thread;

    cur_proc = process_get_current_process();
    cur_thread = cur_proc->cur_thread;

    cur_thread->regs.eip = frame->ip;
    cur_thread->regs.cs = frame->cs;
    cur_thread->regs.eflags = frame->flags;
    cur_thread->regs.esp = frame->esp;
    cur_thread->regs.ss = frame->ss;
    cur_thread->regs.eax = frame->eax;
    cur_thread->regs.ebp = frame->ebp;
    cur_thread->regs.ebx = frame->ebx;
    cur_thread->regs.ecx = frame->ecx;
    cur_thread->regs.edi = frame->edi;
    cur_thread->regs.edx = frame->edx;
    cur_thread->regs.esi = frame->esi;

    return;
}

/*
void scheduler_test_run(void)
{
    PPROCESS cur_proc = process_get_current_process();
    PTHREAD cur_thread = cur_proc->cur_thread;
    paging_switch_pagedir(cur_proc->page_dir);
    scheduler_run_thread(&(cur_thread->regs));
}
*/