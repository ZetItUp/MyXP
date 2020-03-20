/*
 * My Experience Operating System
 * Copyright (C) 2019, Andreas Lindström
 */
#ifndef __MYXP__HARDWARE__PCI__H
#define __MYXP__HARDWARE__PCI__H

#include <hardware/port.h>
#include <hardware/interrupts.h>
#include <lib/types.h>
#include <drivers/driver.h>

namespace myxp
{
    namespace hardware
    {
        enum BaseAddressRegisterType
        {
            MemoryMapping = 0,
            InputOutput = 1
        };

        class BaseAddressRegister
        {
        public:
            bool prefetchable;
            myxp::uint8_t *address;
            myxp::uint32_t size;
            BaseAddressRegisterType type;
        };

        class PeripheralComponentInterconnectDeviceDescriptor
        {
        public:
            myxp::uint32_t portBase;
            myxp::uint32_t interrupt;

            myxp::uint16_t bus;
            myxp::uint16_t device;
            myxp::uint16_t function;

            myxp::uint16_t vendor_id;
            myxp::uint16_t device_id;

            myxp::uint8_t class_id;
            myxp::uint8_t subclass_id;
            myxp::uint8_t interface_id;

            myxp::uint8_t revision;

            PeripheralComponentInterconnectDeviceDescriptor();
            ~PeripheralComponentInterconnectDeviceDescriptor();
        };

        class PeripheralComponentInterconnectController
        {
            Port32Bit dataPort;
            Port32Bit commandPort;

        public:
            PeripheralComponentInterconnectController();
            ~PeripheralComponentInterconnectController();

            myxp::uint32_t Read(myxp::uint16_t bus, myxp::uint16_t device, myxp::uint16_t function, myxp::uint32_t registerOffset);
            void Write(myxp::uint16_t bus, myxp::uint16_t device, myxp::uint16_t function, myxp::uint32_t registerOffset, myxp::uint32_t value);
            bool DeviceHasFunctions(myxp::uint16_t bus, myxp::uint16_t device);

            void SelectDrivers(myxp::drivers::DriverManager *driverManager, myxp::hardware::InterruptManager *interruptManager);
            drivers::Driver* GetDriver(PeripheralComponentInterconnectDeviceDescriptor dev, myxp::hardware::InterruptManager *interruptManager);
            PeripheralComponentInterconnectDeviceDescriptor GetDeviceDescriptor(myxp::uint16_t bus, myxp::uint16_t device, myxp::uint16_t function);
            BaseAddressRegister GetBaseAdressRegister(myxp::uint16_t bus, myxp::uint16_t device, myxp::uint16_t function, myxp::uint16_t bar);
        };
    }
}

#endif