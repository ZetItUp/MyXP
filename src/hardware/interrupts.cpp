/*
 * My Experience Operating System
 * Copyright (C) 2019, Andreas Lindström
 */
#include <lib/types.h>
#include <hardware/interrupts.h>
#include <multitasking.h>
#include <console.h>

using namespace myxp::hardware;



static inline void outb(myxp::uint16_t port, myxp::uint8_t val)
{
    asm volatile("outb %0, %1" : : "a"(val) , "Nd"(port));
}

static inline myxp::uint8_t inb(myxp::uint16_t port)
{
    myxp::uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));

    return ret;
}

static inline void lidt(void *base, myxp::uint16_t size)
{
    struct 
    {
        myxp::uint16_t length;
        void * base;
    } __attribute__((packed)) IDTR = { size, base};
    
    asm ("lidt %0" : : "m"(IDTR));
}

InterruptManager::GateDescriptor InterruptManager::InterruptDescriptorTable[256];
InterruptManager* InterruptManager::ActiveInterruptManager = 0;

InterruptHandler::InterruptHandler(myxp::uint8_t interrupt, InterruptManager* interruptManager)
{
    this->interrupt = interrupt;
    this->interruptManager = interruptManager;
    interruptManager->handlers[interrupt] = this;
}

InterruptHandler::~InterruptHandler()
{
    if(interruptManager->handlers[interrupt] == this)
    {
        interruptManager->handlers[interrupt] = 0;
    }
}

myxp::uint32_t InterruptHandler::HandleInterrupt(myxp::uint32_t esp)
{
    return esp;
}

void InterruptManager::SetInterruptDescriptorTableEntry(myxp::uint8_t interrupt, myxp::uint16_t codeSegmentDescriptorOffset,
                                                        void (*handler)(), myxp::uint8_t DescriptorPrivilegeLevel,
                                                        myxp::uint8_t DescriptorType)
{
    InterruptDescriptorTable[interrupt].handlerAddressLowBits = ((myxp::uint32_t)handler) & 0xFFFF;
    InterruptDescriptorTable[interrupt].handlerAddressHighBits = (((myxp::uint32_t)handler) >> 16) & 0xFFFF;
    InterruptDescriptorTable[interrupt].gdt_codeSegmentDescriptor = codeSegmentDescriptorOffset;
    InterruptDescriptorTable[interrupt].access = IDT_DESC_PRESENT | ((DescriptorPrivilegeLevel & 3) << 5) | DescriptorType;
    InterruptDescriptorTable[interrupt].reserved = 0;
}

void InterruptManager::SetTaskManager(myxp::TaskManager *taskManager)
{
    this->taskManager = taskManager;
}

InterruptManager::InterruptManager(myxp::uint16_t hardwareInterruptOffset, GlobalDescriptorTable* globalDescriptorTable)
    : programmableInterruptControllerMasterCommandPort(0x20),
      programmableInterruptControllerMasterDataPort(0x21),
      programmableInterruptControllerSlaveCommandPort(0xA0),
      programmableInterruptControllerSlaveDataPort(0xA1)
{
    myxp::uint16_t CodeSegment = globalDescriptorTable->CodeSegmentDescriptor();

    this->hardwareInterruptOffset = hardwareInterruptOffset;

    for(myxp::uint16_t i = NUM_OF_IDTS; i > 0; --i)
    {
        SetInterruptDescriptorTableEntry(i, CodeSegment, &InterruptIgnore, KERNEL_MODE, IDT_INTERRUPT_GATE);
        handlers[i] = 0;
    }
    SetInterruptDescriptorTableEntry(0, CodeSegment, &InterruptIgnore, KERNEL_MODE, IDT_INTERRUPT_GATE);
    handlers[0] = 0;

    SetInterruptDescriptorTableEntry(0x00, CodeSegment, &HandleException0x00, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x01, CodeSegment, &HandleException0x01, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x02, CodeSegment, &HandleException0x02, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x03, CodeSegment, &HandleException0x03, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x04, CodeSegment, &HandleException0x04, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x05, CodeSegment, &HandleException0x05, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x06, CodeSegment, &HandleException0x06, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x07, CodeSegment, &HandleException0x07, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x08, CodeSegment, &HandleException0x08, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x09, CodeSegment, &HandleException0x09, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x0A, CodeSegment, &HandleException0x0A, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x0B, CodeSegment, &HandleException0x0B, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x0C, CodeSegment, &HandleException0x0C, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x0D, CodeSegment, &HandleException0x0D, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x0E, CodeSegment, &HandleException0x0E, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x0F, CodeSegment, &HandleException0x0F, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x10, CodeSegment, &HandleException0x10, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x11, CodeSegment, &HandleException0x11, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x12, CodeSegment, &HandleException0x12, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x13, CodeSegment, &HandleException0x13, 0, IDT_INTERRUPT_GATE);

    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x00, CodeSegment, &HandleInterruptRequest0x00, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x01, CodeSegment, &HandleInterruptRequest0x01, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x02, CodeSegment, &HandleInterruptRequest0x02, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x03, CodeSegment, &HandleInterruptRequest0x03, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x04, CodeSegment, &HandleInterruptRequest0x04, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x05, CodeSegment, &HandleInterruptRequest0x05, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x06, CodeSegment, &HandleInterruptRequest0x06, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x07, CodeSegment, &HandleInterruptRequest0x07, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x08, CodeSegment, &HandleInterruptRequest0x08, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x09, CodeSegment, &HandleInterruptRequest0x09, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x0A, CodeSegment, &HandleInterruptRequest0x0A, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x0B, CodeSegment, &HandleInterruptRequest0x0B, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x0C, CodeSegment, &HandleInterruptRequest0x0C, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x0D, CodeSegment, &HandleInterruptRequest0x0D, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x0E, CodeSegment, &HandleInterruptRequest0x0E, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x0F, CodeSegment, &HandleInterruptRequest0x0F, 0, IDT_INTERRUPT_GATE);


    programmableInterruptControllerMasterCommandPort.Write(0x11);
    programmableInterruptControllerSlaveCommandPort.Write(0x11);

    // Tell the PIC that if you recieve an interrupt of 1
    // Add 0x20 to it, or 0x28 for the slave.
    // Since exceptions can use the same interrupt number of 1 etc.
    programmableInterruptControllerMasterDataPort.Write(hardwareInterruptOffset);
    programmableInterruptControllerSlaveDataPort.Write(hardwareInterruptOffset + 8);

    programmableInterruptControllerMasterDataPort.Write(0x04);  // Tell the Master PIC that it is the Master PIC
    programmableInterruptControllerSlaveDataPort.Write(0x02);   // Tell the Slave PIC that it is the Slave PIC

    programmableInterruptControllerMasterDataPort.Write(0x01);
    programmableInterruptControllerSlaveDataPort.Write(0x01);

    programmableInterruptControllerMasterDataPort.Write(0x00);
    programmableInterruptControllerSlaveDataPort.Write(0x00);

    InterruptDescriptorTablePointer idt;
    idt.size = NUM_OF_IDTS * sizeof(GateDescriptor);
    idt.base = (myxp::uint32_t)InterruptDescriptorTable;

    /* printf("\nIDT Size: ");
    printf(idt.size);
    printf("\nIDT Base: 0x");
    printfHex32(idt.base);
    printf("\n"); */

    asm volatile("lidt %0" : : "m" (idt));
}

InterruptManager::~InterruptManager()
{

}

myxp::uint32_t InterruptManager::HandleInterrupt(myxp::uint8_t interrupt, myxp::uint32_t esp)
{
    if(ActiveInterruptManager != 0)
    {
        return ActiveInterruptManager->DoHandleInterrupt(interrupt, esp);
    }

    return esp;
}

myxp::uint32_t InterruptManager::DoHandleInterrupt(myxp::uint8_t interrupt, myxp::uint32_t esp)
{
    /* printf("\nINTERRUPT [");
    printfHex(interrupt);
    printf("]\n"); */

    if(handlers[interrupt] != 0)
    {
        esp = handlers[interrupt]->HandleInterrupt(esp);
    }
    else if(interrupt != hardwareInterruptOffset)
    {
        //printf("UNHANDLED INTERRUPT 0x");
        //printfHex(interrupt);
    }

    if(interrupt == hardwareInterruptOffset)
    {
        //esp = (myxp::uint32_t)taskManager->Schedule((CPUState*)esp);
    }

    if(hardwareInterruptOffset <= interrupt && interrupt < hardwareInterruptOffset + 16)
    {
        if(hardwareInterruptOffset + 8 <= interrupt)
        {
            programmableInterruptControllerSlaveCommandPort.Write(PIC_EOI);
        }

        programmableInterruptControllerMasterCommandPort.Write(PIC_EOI);
    }

    return esp;
}

myxp::uint16_t InterruptManager::HardWareInteruptOffset()
{
    return hardwareInterruptOffset;
}

void InterruptManager::Activate()
{
    if(ActiveInterruptManager != 0)
    {
        ActiveInterruptManager->Deactivate();
    }

    ActiveInterruptManager = this;
    asm("sti");
}

void InterruptManager::Deactivate()
{
    if(ActiveInterruptManager == this)
    {
        ActiveInterruptManager = 0;
        asm("cli");
    }
}

DivByZeroHandler::DivByZeroHandler(myxp::hardware::InterruptManager* interruptManger)
    : InterruptHandler(0x0D, interruptManger)
{

}

DivByZeroHandler::~DivByZeroHandler()
{

}

void DivByZeroHandler::Activate()
{

}

myxp::uint32_t DivByZeroHandler::HandleInterrupt(myxp::uint32_t esp)
{
    errMsg("Division by zero!");
}