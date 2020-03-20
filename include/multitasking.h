/*
 * My Experience Operating System
 * Copyright (C) 2019, Andreas Lindström
 */
#ifndef __MYXP__MULTITASKING_H
#define __MYXP__MULTITASKING_H

#include <lib/types.h>
#include <gdt.h>

namespace myxp
{
    #define TASK_MAX_TASKS 512
    #define TASK_MEMORY_SIZE 4096

    struct CPUState
    {
        myxp::uint32_t eax;
        myxp::uint32_t ebx;
        myxp::uint32_t ecx;
        myxp::uint32_t edx;
        
        myxp::uint32_t esi;
        myxp::uint32_t edi;
        myxp::uint32_t ebp;

        myxp::uint32_t error;

        myxp::uint32_t eip;
        myxp::uint32_t cs;
        myxp::uint32_t eflags;
        myxp::uint32_t esp;
        myxp::uint32_t ss;
    } __attribute__((packed));

    class Task
    {
        friend class TaskManager;
    private:
        myxp::uint8_t stack[TASK_MEMORY_SIZE];  // TODO: Is 4 kb enough?
        CPUState *cpuState;
    public:
        Task(GlobalDescriptorTable *gdt, void entryPoint());
        ~Task();
    };

    class TaskManager
    {
    private:
        Task *tasks[TASK_MAX_TASKS];
        int numTasks;
        int currentTask;

    public:
        TaskManager();
        ~TaskManager();

        bool AddTask(Task *task);
        CPUState* Schedule(CPUState *cpuState);
    };
}

#endif