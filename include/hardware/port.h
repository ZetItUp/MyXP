/*
 * My Experience Operating System
 * Copyright (C) 2019, Andreas Lindström
 */
#ifndef __MYXP__HARDWARE__PORT_H
#define __MYXP__HARDWARE__PORT_H

#include <lib/types.h>

namespace myxp
{
    namespace hardware
    {
        class Port
        {
        protected:
            // FIXME: Must be virtual (currently got no memory management)
            Port(myxp::uint16_t portNumber);
            ~Port();

            myxp::uint16_t portNumber;
        };


        class Port8Bit : public Port
        {
            public:
                Port8Bit(myxp::uint16_t portNumber);
                ~Port8Bit();

                virtual myxp::uint8_t Read();
                virtual void Write(myxp::uint8_t data);

            protected:
                static inline myxp::uint8_t Read8(myxp::uint16_t _port)
                {
                    myxp::uint8_t result;
                    __asm__ volatile("inb %1, %0" : "=a" (result) : "Nd" (_port));

                    return result;
                }

                static inline void Write8(myxp::uint16_t _port, myxp::uint8_t _data)
                {
                    __asm__ volatile("outb %0, %1" : : "a" (_data), "Nd" (_port));
                }

        };

        class Port8BitSlow : public Port8Bit
        {
            public:
                Port8BitSlow(myxp::uint16_t portNumber);
                ~Port8BitSlow();

                virtual void Write(myxp::uint8_t data);

            protected:
                static inline void Write8Slow(uint16_t _port, uint8_t _data)
                {
                    __asm__ volatile("outb %0, %1\njmp 1f\n1: jmp 1f\n1:" : : "a" (_data), "Nd" (_port));
                }

        };

        class Port16Bit : public Port
        {
            public:
                Port16Bit(myxp::uint16_t portNumber);
                ~Port16Bit();

                virtual myxp::uint16_t Read();
                virtual void Write(myxp::uint16_t data);

            protected:
                static inline uint16_t Read16(uint16_t _port)
                {
                    uint16_t result;
                    __asm__ volatile("inw %1, %0" : "=a" (result) : "Nd" (_port));
                    return result;
                }

                static inline void Write16(uint16_t _port, uint16_t _data)
                {
                    __asm__ volatile("outw %0, %1" : : "a" (_data), "Nd" (_port));
                }

        };

        class Port32Bit : public Port
        {
            public:
                Port32Bit(myxp::uint32_t portNumber);
                ~Port32Bit();

                virtual myxp::uint32_t Read();
                virtual void Write(myxp::uint32_t data);

            protected:
                static inline uint32_t Read32(uint16_t _port)
                {
                    uint32_t result;
                    __asm__ volatile("inl %1, %0" : "=a" (result) : "Nd" (_port));
                    return result;
                }

                static inline void Write32(uint16_t _port, uint32_t _data)
                {
                    __asm__ volatile("outl %0, %1" : : "a"(_data), "Nd" (_port));
                }

        };
    }
}

#endif