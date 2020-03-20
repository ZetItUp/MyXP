/*
 * My Experience Operating System
 * Copyright (C) 2019, Andreas Lindström
 */
#ifndef __MYXP__DRIVER__KEYBOARD_H
#define __MYXP__DRIVER__KEYBOARD_H

#include <lib/types.h>
#include <hardware/interrupts.h>
#include <hardware/port.h>
#include <drivers/driver.h>

namespace myxp
{
    namespace drivers
    {
        class KeyboardEventHandler 
        {
        public:
            KeyboardEventHandler();

            virtual void OnKeyDown(char);
            virtual void OnKeyUp(char);
        };

        class KeyboardDriver : public hardware::InterruptHandler, public Driver
        {
            hardware::Port8Bit dataPort;
            hardware::Port8Bit commandPort;

            KeyboardEventHandler *handler;
        public:
            KeyboardDriver(hardware::InterruptManager *interruptManager, KeyboardEventHandler *handler);
            ~KeyboardDriver();

            virtual myxp::uint32_t HandleInterrupt(myxp::uint32_t esp);
            virtual void Activate();
        };
    }
}

#endif