/*
 * My Experience Operating System
 * Copyright (C) 2019, Andreas Lindström
 * 
 * 
 * Mouse Driver Notice(s):
 * + When the mouse is reset, either by applying power or with a reset command (0xFF), it always goes to the following default state:
 * 
 *      - Packets Disabled
 *      - Emulate 3 Button Mouse (buttons 4, 5 and scroll wheels disabled)
 *      - 3 byte packets
 *      - 4 pixel/mm resolution
 *      - 100 packets per second sample rate     
 * 
 */
#include <drivers/mouse.h>

void printf(char *str);

myxp::drivers::MouseEventHandler::MouseEventHandler()
{

}

void myxp::drivers::MouseEventHandler::OnActivate()
{

}

void myxp::drivers::MouseEventHandler::OnMouseDown(myxp::uint8_t button)
{

}

void myxp::drivers::MouseEventHandler::OnMouseUp(myxp::uint8_t button)
{

}

void myxp::drivers::MouseEventHandler::OnMouseMove(myxp::int8_t x, myxp::int8_t y)
{

}

myxp::drivers::MouseDriver::MouseDriver(hardware::InterruptManager *manager, drivers::MouseEventHandler *handler)
    : InterruptHandler(0x2C, manager),
    dataPort(0x60),
    commandPort(0x64)
{
    this->handler = handler;
    driverName = "PS/2 Mouse Driver";
}

myxp::drivers::MouseDriver::~MouseDriver()
{

}

void myxp::drivers::MouseDriver::Activate()
{
    offset = 0;
    buttons = 0;

    // Send an ACK command (Enable Auxiliary Device) to port 0x64
    commandPort.Write(0xA8);
    commandPort.Write(0x20);

    // Must wait for port 0x60 to have bit 1 (value=2) to become clear.
    myxp::uint8_t status = dataPort.Read() | 2;
    commandPort.Write(0x60);    // Command 0x60 sets the controller command byte
    dataPort.Write(status);

    // Sending bytes to the mouse must preced by sending a 0xD4 byte to port 0x64
    commandPort.Write(0xD4);
    // Enable Packet Streaming by sending 0xF4 so we can get input like movement and clicks.
    dataPort.Write(0xF4);
    dataPort.Read();
}

myxp::uint32_t myxp::drivers::MouseDriver::HandleInterrupt(myxp::uint32_t esp)
{
        myxp::uint8_t status = commandPort.Read();

    // Wait for 0x20 (Compaq Status Byte)
    if(!(status & 0x20))
    {
        return esp;
    }

    buffer[offset] = dataPort.Read();
    offset = (offset + 1) % 3;

    if(offset == 0)
    {
        if(buffer[1] != 0 || buffer[2] != 0)
        {
            handler->OnMouseMove(buffer[1], -buffer[2]);
        }

        for(myxp::uint8_t i = 0; i < 3; i++)
        {
            if((buffer[0] & 0x1 << i) != (buttons & (0x1 << i)))
            {
                if(buttons & (0x1 << i))
                {
                    handler->OnMouseUp(i + 1);
                }
                else
                {
                    handler->OnMouseDown(i + 1);
                }
            }
        }

        buttons = buffer[0];
    }

    return esp;
}
