/*
 * My Experience Operating System
 * Copyright (C) 2019, Andreas Lindström
 */
#include <hardware/port.h>

using namespace myxp::hardware;

Port::Port(myxp::uint16_t portNumber)
{
    this->portNumber = portNumber;
}

Port::~Port()
{

}

/*
 *  8-bit Section
 */

Port8Bit::Port8Bit(myxp::uint16_t portNumber)
    : Port(portNumber)
{

}

Port8Bit::~Port8Bit()
{

}

void Port8Bit::Write(myxp::uint8_t data)
{
    Write8(portNumber, data);
}

myxp::uint8_t Port8Bit::Read()
{
    return Read8(portNumber);
}

/*
 * 8-bit Section (Slow Mode)
 */
Port8BitSlow::Port8BitSlow(myxp::uint16_t portNumber)
    : Port8Bit(portNumber)
{

}

Port8BitSlow::~Port8BitSlow()
{

}

void Port8BitSlow::Write(myxp::uint8_t data)
{
    Write8Slow(portNumber, data);
}

/*
 *  16-bit Section
 */
Port16Bit::Port16Bit(myxp::uint16_t portNumber)
    : Port(portNumber)
{

}

Port16Bit::~Port16Bit()
{

}

void Port16Bit::Write(myxp::uint16_t data)
{
    Write16(portNumber, data);
}

myxp::uint16_t Port16Bit::Read()
{
    return Read16(portNumber);
}

/*
 *  32-bit Section
 */
Port32Bit::Port32Bit(myxp::uint32_t portNumber)
    : Port(portNumber)
{

}

Port32Bit::~Port32Bit()
{

}

void Port32Bit::Write(myxp::uint32_t data)
{
    Write32(portNumber, data);
}

myxp::uint32_t Port32Bit::Read()
{
    return Read32(portNumber);
}