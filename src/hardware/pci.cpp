/*
 * My Experience Operating System
 * Copyright (C) 2019, Andreas Lindström
 */
#include <hardware/pci.h>
#include <console.h>

/*
 *  BaseAdressRegister (BAR)
 */


/*
 *  PCI Descriptor
 */
myxp::hardware::PeripheralComponentInterconnectDeviceDescriptor::PeripheralComponentInterconnectDeviceDescriptor()
{

}

myxp::hardware::PeripheralComponentInterconnectDeviceDescriptor::~PeripheralComponentInterconnectDeviceDescriptor()
{

}

/*
 *  PCI Controller
 */
myxp::hardware::PeripheralComponentInterconnectController::PeripheralComponentInterconnectController()
    : dataPort(0xCFC),
      commandPort(0xCF8)
{

}

myxp::hardware::PeripheralComponentInterconnectController::~PeripheralComponentInterconnectController()
{

}

myxp::uint32_t myxp::hardware::PeripheralComponentInterconnectController::Read(myxp::uint16_t bus, myxp::uint16_t device, myxp::uint16_t function, myxp::uint32_t registerOffset)
{
    myxp::uint32_t id = 
        0x1 << 31 | 
        ((bus & 0xFF) << 16) |
        ((device & 0x1F) << 11) | 
        ((function & 0x07) << 8) |
        (registerOffset & 0xFC);
    
    commandPort.Write(id);
    myxp::uint32_t result = dataPort.Read();

    return result >> (8 * (registerOffset % 4));
}

void myxp::hardware::PeripheralComponentInterconnectController::Write(myxp::uint16_t bus, myxp::uint16_t device, myxp::uint16_t function, myxp::uint32_t registerOffset, myxp::uint32_t value)
{
    myxp::uint32_t id = 
        0x1 << 31 | 
        ((bus & 0xFF) << 16) |
        ((device & 0x1F) << 11) | 
        ((function & 0x07) << 8) |
        (registerOffset & 0xFC);

    commandPort.Write(id);
    dataPort.Write(value);
}

bool myxp::hardware::PeripheralComponentInterconnectController::DeviceHasFunctions(myxp::uint16_t bus, myxp::uint16_t device)
{
    Read(bus, device, 0, 0x0E) & (1 << 7);

    return true;
}

void myxp::hardware::PeripheralComponentInterconnectController::SelectDrivers(myxp::drivers::DriverManager *driverManager, myxp::hardware::InterruptManager *interruptManager)
{
    for(int bus = 0; bus < 8; bus++)
    {
        for(int device = 0; device < 32; device++)
        {
            int numFunctions = DeviceHasFunctions(bus, device) ? 8 : 1;

            for(int function = 0; function < numFunctions; function++)
            {
                PeripheralComponentInterconnectDeviceDescriptor dev = GetDeviceDescriptor(bus, device, function);

                if(dev.vendor_id == 0x0000 || dev.vendor_id == 0xFFFF)
                {
                    continue;
                }

                for(int barNum = 0; barNum < 6; barNum++)
                {
                    BaseAddressRegister bar = GetBaseAdressRegister(bus, device, function, barNum);

                    if(bar.address && (bar.type == InputOutput))
                    {
                        dev.portBase = (myxp::uint32_t)bar.address;
                    }

                    myxp::drivers::Driver *driver = GetDriver(dev, interruptManager);

                    if(driver != 0)
                    {
                        driverManager->AddDriver(driver);
                    }
                }

                printf("[");
                SetConsoleColor(14, 0);
                printf("PCI");
                SetConsoleColor(7, 0);
                printf("] ");
                printf("BUS=0x");
                printfHex(bus & 0xFF);

                printf(", DEVICE=0x");
                printfHex(device & 0xFF);

                printf(", FUNCTION=0x");
                printfHex(function & 0xFF);

                printf(", VENDOR=0x");
                printfHex((dev.vendor_id & 0xFF00) >> 8);
                printfHex(dev.vendor_id & 0xFF);
                
                printf(", DEVICE_ID=0x");
                printfHex((dev.device_id & 0xFF00) >> 8);
                printfHex(dev.device_id & 0xFF);
                printf("\n");
            }
        }
    }
}

myxp::drivers::Driver* myxp::hardware::PeripheralComponentInterconnectController::GetDriver(myxp::hardware::PeripheralComponentInterconnectDeviceDescriptor dev, myxp::hardware::InterruptManager *interruptManager)
{
    switch(dev.vendor_id)
    {
        case 0x1022: // AMD
        {
            switch (dev.device_id)
            {
                case 0x2000:    // AM79C973 Network Card
                    //printf("AMD am79c973 ");
                    break;
            
                default:
                    break;
            }
        }
        case 0x8086: // Intel
        {
            break;
        }
    }

    switch(dev.class_id)
    {
        case 0x03: // Graphics
            switch(dev.subclass_id)
            {
                case 0x00: // VGA
                    //printf("VGA ");
                    break;
            }   
            break;
    }

    return 0;
}
            
myxp::hardware::BaseAddressRegister myxp::hardware::PeripheralComponentInterconnectController::GetBaseAdressRegister(myxp::uint16_t bus, myxp::uint16_t device, myxp::uint16_t function, myxp::uint16_t bar)
{
    myxp::hardware::BaseAddressRegister result;

    myxp::uint32_t headerType = Read(bus, device, function, 0x0E) % 0x7F;
    int maxBARs = 6 - (4*headerType);

    if(bar >= maxBARs)
    {
        return result;
    }

    myxp::uint32_t barValue = Read(bus, device, function, 0x10 + 4 * bar);
    result.type = (barValue & 0x1) ? InputOutput : MemoryMapping;

    if(result.type == MemoryMapping)
    {
        switch((barValue >> 1) & 0x3)
        {
            case 0: //  32-bit Mode
            case 1: //  20-bit Mode
            case 2: //  64-bit Mode
                break;
        }
    }
    else
    {
        result.address = (myxp::uint8_t*)(barValue & ~0x3);
        result.prefetchable = false;
    }
    
    return result;
}

myxp::hardware::PeripheralComponentInterconnectDeviceDescriptor myxp::hardware::PeripheralComponentInterconnectController::GetDeviceDescriptor(myxp::uint16_t bus, myxp::uint16_t device, myxp::uint16_t function)
{
    PeripheralComponentInterconnectDeviceDescriptor result;

    result.bus = bus;
    result.device = device;
    result.function = function;
    
    result.vendor_id = Read(bus, device, function, 0x00);
    result.device_id = Read(bus, device, function, 0x02);

    result.class_id = Read(bus, device, function, 0x0B);
    result.subclass_id = Read(bus, device, function, 0x0A);
    result.interface_id = Read(bus, device, function, 0x09);

    result.revision = Read(bus, device, function, 0x08);
    result.interrupt = Read(bus, device, function, 0x3C);

    return result;
}