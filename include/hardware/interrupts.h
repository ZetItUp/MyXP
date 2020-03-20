/*
 * My Experience Operating System
 * Copyright (C) 2019, Andreas Lindström
 */
#ifndef __MYXP__HARDWARE__INTERRUPTS_H
#define __MYXP__HARDWARE__INTERRUPTS_H

#include <lib/types.h>
#include <gdt.h>
#include <hardware/port.h>
#include <multitasking.h>

#define KERNEL_MODE         0
#define USER_MODE           3
#define NUM_OF_IDTS         256

#define IDT_DESC_PRESENT    0x80
#define IDT_TASK_GATE       0x9
#define IDT_INTERRUPT_GATE  0xE
#define IDT_TRAP_GATE       0xF

#define PIC_EOI             0x20    // End of Interrupt

namespace myxp
{
    namespace hardware
    {
        class InterruptManager;

        static inline void outb(myxp::uint16_t port, myxp::uint8_t val);
        static inline myxp::uint8_t inb(myxp::uint16_t port);
        static inline void lidt(void *base, myxp::uint16_t size);

        struct StackFrame
        {
            uint16_t gs __attribute__((aligned(4)));
            uint16_t fs __attribute__((aligned(4)));
            uint16_t es __attribute__((aligned(4)));
            uint16_t ds __attribute__((aligned(4)));

            uint32_t edi;
            uint32_t esi;
            uint32_t ebp;
            uint32_t esp_;
            uint32_t ebx;
            uint32_t edx;
            uint32_t ecx;
            uint32_t eax;
            uint32_t interruptNum;
            uint32_t errorCode;
            uint32_t eip;
            uint32_t cs;
            uint32_t eflags;
            uint32_t esp;
            uint32_t ss;
        };

        class InterruptHandler
        {
        protected:
            myxp::uint8_t interrupt;
            InterruptManager* interruptManager;

            InterruptHandler(myxp::uint8_t interrupt, InterruptManager* interruptManager);
            ~InterruptHandler();

        public:
            virtual myxp::uint32_t HandleInterrupt(myxp::uint32_t esp);
        };

        class DivByZeroHandler : public InterruptHandler 
        {
        public:
            DivByZeroHandler(InterruptManager *interruptManager);
            ~DivByZeroHandler();

            virtual myxp::uint32_t HandleInterrupt(myxp::uint32_t esp);
            virtual void Activate();
        };

        class InterruptManager
        {
            friend class InterruptHandler;
        private:
            DivByZeroHandler *divByZeroHandler;

        protected:
            static InterruptManager* ActiveInterruptManager;
            InterruptHandler* handlers[256];
            myxp::TaskManager* taskManager;

            struct GateDescriptor
            {
                myxp::uint16_t handlerAddressLowBits;
                myxp::uint16_t gdt_codeSegmentDescriptor;
                myxp::uint8_t reserved;
                myxp::uint8_t access;
                myxp::uint16_t handlerAddressHighBits;
            } __attribute__((packed));

            static GateDescriptor InterruptDescriptorTable[256] __attribute__((aligned(8)));

            struct InterruptDescriptorTablePointer
            {
                myxp::uint16_t size;
                myxp::uint32_t base;
            } __attribute__((packed));

            myxp::uint16_t hardwareInterruptOffset;

            static void SetInterruptDescriptorTableEntry(myxp::uint8_t interrupt, myxp::uint16_t codeSegmentDescriptorOffset,
                                                        void (*handler)(), myxp::uint8_t DescriptorPrivilegeLevel,
                                                        myxp::uint8_t DescriptorType);

        public:
            static myxp::uint32_t HandleInterrupt(myxp::uint8_t interrupt, myxp::uint32_t esp);
            
        protected:
            myxp::uint32_t DoHandleInterrupt(myxp::uint8_t interrupt, myxp::uint32_t esp);

            static void InterruptIgnore();              // Ignore Interrupt, to prevent crash on any unhandled interrupts.

            static void HandleInterruptRequest0x00();   // Timer Interrupt
            static void HandleInterruptRequest0x01();   // Keyboard Interrupt 
            static void HandleInterruptRequest0x02();
            static void HandleInterruptRequest0x03();
            static void HandleInterruptRequest0x04();
            static void HandleInterruptRequest0x05();
            static void HandleInterruptRequest0x06();
            static void HandleInterruptRequest0x07();
            static void HandleInterruptRequest0x08();
            static void HandleInterruptRequest0x09();
            static void HandleInterruptRequest0x0A();
            static void HandleInterruptRequest0x0B();
            static void HandleInterruptRequest0x0C();   // Mouse Interrupt
            static void HandleInterruptRequest0x0D();
            static void HandleInterruptRequest0x0E();   // Page Fault Interrupt
            static void HandleInterruptRequest0x0F();
            static void HandleInterruptRequest0x31();

            static void HandleException0x00();
            static void HandleException0x01();
            static void HandleException0x02();
            static void HandleException0x03();
            static void HandleException0x04();
            static void HandleException0x05();
            static void HandleException0x06();
            static void HandleException0x07();
            static void HandleException0x08();
            static void HandleException0x09();
            static void HandleException0x0A();
            static void HandleException0x0B();
            static void HandleException0x0C();
            static void HandleException0x0D();
            static void HandleException0x0E();
            static void HandleException0x0F();
            static void HandleException0x10();
            static void HandleException0x11();
            static void HandleException0x12();
            static void HandleException0x13();

            Port8BitSlow programmableInterruptControllerMasterCommandPort;
            Port8BitSlow programmableInterruptControllerMasterDataPort;
            Port8BitSlow programmableInterruptControllerSlaveCommandPort;
            Port8BitSlow programmableInterruptControllerSlaveDataPort;

        public:
            InterruptManager(myxp::uint16_t hardwareInterruptOffset, GlobalDescriptorTable* globalDescriptorTable);
            ~InterruptManager();

            myxp::uint16_t HardWareInteruptOffset();
            void SetTaskManager(myxp::TaskManager *taskManager);
            void Activate();
            void Deactivate();
        };
    }
}

#endif