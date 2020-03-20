/*
 * My Experience Operating System
 * Copyright (C) 2019, Andreas Lindström
 */
#include <drivers/keyboard.h>

void printf(char *str);
void printfHex(myxp::uint8_t key);

/*
 *  Keyboard Event Handler
 */
myxp::drivers::KeyboardEventHandler::KeyboardEventHandler()
{

}

void myxp::drivers::KeyboardEventHandler::OnKeyUp(char)
{

}

void myxp::drivers::KeyboardEventHandler::OnKeyDown(char)
{

}

/*
 * Keyboard Driver
 */
myxp::drivers::KeyboardDriver::KeyboardDriver(hardware::InterruptManager *interruptManager, drivers::KeyboardEventHandler *handler)
    : InterruptHandler(0x21, interruptManager),
    dataPort(0x60),
    commandPort(0x64)
{
    driverName = "PS/2 Keyboard Driver";
    this->handler = handler;
}

myxp::drivers::KeyboardDriver::~KeyboardDriver()
{

}

void myxp::drivers::KeyboardDriver::Activate()
{
    while(commandPort.Read() & 0x1)
    {
        dataPort.Read();
    }

    commandPort.Write(0xAE);    // Tell PIC to start sending keyboard interrupts
    commandPort.Write(0x20);    // Send command to wait for PICs current state

    // Get the state and clear the 5th bit
    myxp::uint8_t status = (dataPort.Read() | 1) & ~0x10;
    commandPort.Write(0x60);    // Set state

    dataPort.Write(status);     // Reply with the new state
    dataPort.Write(0xF4);       // Activate the keyboard
}

myxp::uint32_t myxp::drivers::KeyboardDriver::HandleInterrupt(myxp::uint32_t esp)
{
    myxp::uint8_t key = dataPort.Read();

    printf("Keyboard 0x");
    printfHex(key);

    return esp;
}