/*
 * My Experience Operating System
 * Copyright (C) 2019, Andreas Lindström
 */
#include <lib/types.h>
#include <console.h>
#include <gdt.h>
#include <memorym.h>
#include <hardware/interrupts.h>
#include <hardware/pci.h>
#include <drivers/driver.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <multitasking.h>
#include <multiboot.h>

using namespace myxp;
using namespace myxp::hardware;
using namespace myxp::drivers;

uintptr_t initial_esp = 0;

class PrintfKeyboardEventHandler : public KeyboardEventHandler
{
public:
    void OnKeyDown(char c)
    {
        char *tmp = " ";
        tmp[0] = c;

        printf(tmp);
    }
};


class MouseToConsole : public MouseEventHandler
{

public:
    MouseToConsole()
    {

    }

    virtual void OnActivate()
    {
        xPosition = 40;
        yPosition = 12;

        ShiftConsoleColors(xPosition, yPosition);
    }

    virtual void OnMouseUp(myxp::uint8_t button)
    {

    }

    virtual void OnMouseDown(myxp::uint8_t button)
    {

    }

    virtual void OnMouseMove(myxp::int8_t xpos, myxp::int8_t ypos)
    {
        ShiftConsoleColors(xPosition, yPosition);

        xPosition += xpos;
        if(xPosition < 0)
        {
            xPosition = 0;
        }
        else if(xPosition >= 80)
        {
            xPosition = 79;
        }

        yPosition += ypos;
        if(yPosition < 0)
        {
            yPosition = 0;
        }
        else if(yPosition >= 25)
        {
            yPosition = 24;
        }

        ShiftConsoleColors(xPosition, yPosition);
    }
};

void TaskA()
{
    while(true)
    {
        printf("A");
    }
}

void TaskB()
{
    while(true)
    {
        printf("B");
    }
}

typedef void (*constructor)();

extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void callConstructors()
{
    for (constructor *i = &start_ctors; i != &end_ctors; i++)
    {
        (*i)();
    }
}

extern "C" void kernelMain(struct multiboot_info *multiboot_struct, myxp::uint32_t multiboot_magic, myxp::uint32_t esp)
{
    initial_esp = esp;

    // Printing a boot message
    SetConsoleColor(7, 0);
    ClearScreen();
    printf("My eXPerience\n============================\n\n");

    if(multiboot_magic != MULTIBOOT_BOOTLOADER_MAGIC)
    {
        panic("Did not boot from GRUB!");
    }

    GlobalDescriptorTable gdt;
    InterruptManager interrupts(0x20, &gdt);
    MemoryManager memoryManager(&interrupts);
    
    printf("Initializing System...\n");
    uint32_t lastMod = (uintptr_t)&end;
/*     printf("\nHeap Start: 0x");
	printfHex32(lastMod);
	printf("\n"); */
    if(multiboot_struct->flags & MULTIBOOT_INFO_MODS)
    {
        printf("Modules: ");
        printf(multiboot_struct->mods_count);
        printf("\n");
    }
    else
    {
        sysmsg("Found no mods from boot loader.");
    }
    

    if((uintptr_t)multiboot_struct > lastMod)
    {
        lastMod = (uintptr_t)multiboot_struct + sizeof(struct multiboot_info);
    }

    while(lastMod & 0x7FF)
    {
        lastMod++;
    }

    memoryManager.kmalloc_startAt(lastMod);

    /* printf("\nNew Heap Start: 0x");
	printfHex32(lastMod);
	printf("\n"); */

    if(multiboot_struct->flags & MULTIBOOT_MEMORY_INFO)
    {
        uint32_t ram = multiboot_struct->mem_upper + multiboot_struct->mem_lower;
        memoryManager.InitializePaging(ram);
    }
    else
    {
        panic("We did not recieve any RAM information from GRUB!");
    }
    
    if(multiboot_struct->flags & MULTIBOOT_INFO_MEM_MAP)
    {
        sysmsg("Mapping Memory");

        multiboot_memory_map_t *memMap = (void*)multiboot_struct->mmap_addr;

        while((uintptr_t)memMap < multiboot_struct->mmap_addr + multiboot_struct->mmap_length)
        {
            if(memMap->type == 2)
            {
                for(unsigned long long int i = 0; i < memMap->len; i += 0x1000)
                {
                    if(memMap->addr + i > 0xFFFFFFFF)
                    {
                        break;
                    }

                    memoryManager.MarkPagingSystem((memMap->addr + i) & 0xFFFFF000);
                }
            }

            memMap = (multiboot_memory_map_t*)((uintptr_t)memMap + memMap->size + sizeof(uintptr_t));
        }
    }

    memoryManager.CompletePaging();
    interrupts.Activate();

    //int a = 1 / 0;

    printf("\nTesting Memory Heap:\n");

    uintptr_t ptr = memoryManager.kmalloc(8);
    //uint32_t *ptr = (uint32_t*)0xA0000000;
    //uint32_t pf = *ptr;
    //asm volatile("jmp .");
    
    //TaskManager taskManager;

    //Task task1(&gdt, TaskA);
    //Task task2(&gdt, TaskB);
    //memoryManager.SetInterruptManager(&interrupts);

    //sysmsg("Task manager up and running");
    
/* 
    printf("RAM: ");
    printf(string::itoa((*memupper) / 1024, 0, 10));

    printf(" kB\nHeap Memory: 0x");
    printfHex((heap >> 24) & 0xFF);
    printfHex((heap >> 16) & 0xFF);
    printfHex((heap >> 8) & 0xFF);
    printfHex((heap) & 0xFF);

    uint32_t allocated = kmalloc(1024);
    printf("\nAllocated Memory: 0x");
    printfHex(((myxp::size_t)allocated >> 24) & 0xFF);
    printfHex(((myxp::size_t)allocated >> 16) & 0xFF);
    printfHex(((myxp::size_t)allocated >> 8) & 0xFF);
    printfHex(((myxp::size_t)allocated) & 0xFF);
    printf("\n\n"); */

/*     DriverManager driverMgr;

    PrintfKeyboardEventHandler keyHandler;
    KeyboardDriver keyboard(&interrupts, &keyHandler);
    driverMgr.AddDriver(&keyboard);

    MouseToConsole mtc;
    MouseDriver mouse(&interrupts, &mtc);
    driverMgr.AddDriver(&mouse);

    printf("Scanning PCI Slots...\n");
    PeripheralComponentInterconnectController PCIController;
    PCIController.SelectDrivers(&driverMgr, &interrupts);
    printf("\n");

    // Activate all added drivers
    //panic("Something went wrong!");

    driverMgr.ActivateAll();

    // Activate interrupts
    interrupts.Activate();

    uint32_t ptr = (uint32_t)0xA0000000;
    printfHex32(ptr); */
    //uint32_t pageFault = *ptr;

    //taskManager.AddTask(&task1);
    //taskManager.AddTask(&task2);

    // Infinte loop since the CPU will continue to
    // execute instructions after we are done.
    for(;;) {
        asm volatile("hlt");
    }
}