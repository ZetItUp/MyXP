/*
 * My Experience Operating System
 * Copyright (C) 2019, Andreas Lindström
 */
#include <multitasking.h>
#include <gdt.h>

myxp::Task::Task(myxp::GlobalDescriptorTable *gdt, void entryPoint())
{
    cpuState = (CPUState*)(stack + TASK_MEMORY_SIZE - sizeof(CPUState));

    cpuState->eax = 0;
    cpuState->ebx = 0;
    cpuState->ecx = 0;
    cpuState->edx = 0;

    cpuState->esi = 0;
    cpuState->edi = 0;
    cpuState->ebp = 0;

    cpuState->eip = (myxp::uint32_t)entryPoint;
    cpuState->cs = gdt->CodeSegmentDescriptor();
    cpuState->eflags = 0x202;
}

myxp::Task::~Task()
{

}

myxp::TaskManager::TaskManager()
{
    numTasks = 0;
    currentTask = -1;
}

myxp::TaskManager::~TaskManager()
{

}

bool myxp::TaskManager::AddTask(myxp::Task *task)
{
    if(numTasks >= TASK_MAX_TASKS)
    {
        return false;
    }

    tasks[numTasks++] = task;

    return true;
}

myxp::CPUState* myxp::TaskManager::Schedule(myxp::CPUState *cpuState)
{
    if(numTasks <= 0)
    {
        return cpuState;
    }

    if(currentTask >= 0)
    {
        tasks[currentTask]->cpuState = cpuState;
    }

    if(++currentTask >= numTasks)
    {
        currentTask %= numTasks;
    }

    return tasks[currentTask]->cpuState;
}