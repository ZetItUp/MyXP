/*
 * My Experience Operating System
 * Copyright (C) 2019, Andreas Lindström
 */
#ifndef __MYXP__DRIVER__MOUSE_H
#define __MYXP__DRIVER__MOUSE_H

#include <lib/types.h>
#include <hardware/port.h>
#include <hardware/interrupts.h>
#include <drivers/driver.h>

namespace myxp
{
    namespace drivers
    {
        class MouseEventHandler
        {
        protected:
            myxp::int8_t xPosition;
            myxp::int8_t yPosition;

        public:
            MouseEventHandler();

            virtual void OnActivate();
            virtual void OnMouseDown(myxp::uint8_t button);
            virtual void OnMouseUp(myxp::uint8_t button);
            virtual void OnMouseMove(myxp::int8_t x, myxp::int8_t y);
        };

        class MouseDriver : public hardware::InterruptHandler, public Driver
        {
            hardware::Port8Bit dataPort;
            hardware::Port8Bit commandPort;

            // We use 3 buffers:
            // 0: Will be the movement on the X-axis
            // 1: Will be the movement on the Y-axis (reversed)
            // 2: 
            myxp::uint8_t buffer[3];
            myxp::uint8_t offset;
            myxp::uint8_t buttons;

            MouseEventHandler *handler;

        public:
            MouseDriver(hardware::InterruptManager *manager, MouseEventHandler *handler);
            ~MouseDriver();

            virtual myxp::uint32_t HandleInterrupt(myxp::uint32_t esp);
            virtual void Activate();
        };
    }  
} 


#endif