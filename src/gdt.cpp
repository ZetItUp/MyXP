/*
 * My Experience Operating System
 * Copyright (C) 2019, Andreas Lindström
 */
#include <gdt.h>
#include <console.h>

myxp::GlobalDescriptorTable::GlobalDescriptorTable()
    : nullSegmentDescriptor(0, 0, 0),
      codeSegmentDescriptor(0, 0xFFFFFFFF, 0x9A),   // Code Segment Descriptor should have the value 0x9A
      dataSegmentDescriptor(0, 0xFFFFFFFF, 0x92),    // Data Segement Descriptor should have the value 0x92, We cannot write to this!
      userCodeSegementDescriptor(0, 0xFFFFFFFF, 0xFA),
      userDataSegementDescriptor(0, 0xFFFFFFFF, 0xF2)
{
    // The processor requires 6 bytes to be told to use this GDT.
    myxp::uint32_t i[2];                            // We use this to get 8 bytes

    i[1] = (myxp::uint32_t)this;                    // Set the adress of the GDT it self.
    i[0] = sizeof(GlobalDescriptorTable) << 16;     // Set the last 4 bytes to the size of the High bytes of the GDT size.
                                                    // So we shift it to the left to get the high bytes

/*     printf("\nGDT Address: 0x");
    printfHex32(i[1]);
    printf("\nGDT Size: ");
    printf(i[0]);
    printf("\n");
    
    printf("\n"); */

    // Tell the processor to use this GDT using the lgdt(load global descriptor table)
    //uint32_t eax; 

    //asm volatile("mov %0, %%eax" : "=r"(eax));
    /* printf("\nEAX: 0x");
    printfHex32(eax); */
    
    asm volatile("lgdt (%0)\n\t"
                "ljmp $0x08, $1f\n\t"
                "1:\n\t"
                "mov $0x10, %%eax\n\t"
                "mov %%eax, %%ds\n\t"
                "mov %%eax, %%es\n\t"
                "mov %%eax, %%fs\n\t"
                "mov %%eax, %%gs\n\t"
                "mov %%eax, %%ss\n\t": :"p" (((myxp::uint8_t*)i) + 2) : "eax");

    //asm volatile("mov %0, %%eax" : "=r"(eax));
    /* printf("\nEAX: 0x");
    printfHex32(eax); */
}

myxp::GlobalDescriptorTable::~GlobalDescriptorTable()
{
    // Unload the GDT here.
}

void myxp::GlobalDescriptorTable::SetDescriptorByteGranularity(myxp::int32_t segment, myxp::uint32_t size, myxp::uint32_t base, myxp::int8_t access, myxp::int32_t dpl)
{

}

myxp::GlobalDescriptorTable::SegmentDescriptor::SegmentDescriptor(myxp::uint32_t base, myxp::uint32_t limit, myxp::uint8_t type)
{
    // Since the GDT is a big mess we set the bytes very explicit in an array.
    myxp::uint8_t* target = (myxp::uint8_t*)this;

    // The limit has to be a multiply of 2 to the 12
    // If the limit is less or equal to 16-bit
    if(limit <= 65536)
    {   
        // Set the 6th bit to 16-bit, so the processor knows that it is a 16-bit
        target[6] = 0x40;
    }
    else
    {
        // Else we check if the last bits are not all 1's and set it appropriatly
        if((limit & 0xFFF) != 0xFFF)
        {
            // Shift the limit by 12 - 1
            limit = (limit >> 12) - 1;
        }
        else
        {
            // Shift the limit by just 12
            limit = (limit >> 12);
        } 

        //limit = (limit >> 12);
        
        target[6] = 0xC0;
    }
    
    // Set the limit bits
    target[0] = limit & 0xFF;
    target[1] = (limit >> 8) & 0xFF;
    target[6] |= (limit >> 16) & 0xF;

    // Distribute the pointer in memory
    target[2] = base & 0xFF;
    target[3] = (base >> 8) & 0xFF;
    target[4] = (base >> 16) & 0xFF;
    target[7] = (base >> 24) & 0xFF;

    // Set the access flags
    target[5] = type;
}

myxp::uint16_t myxp::GlobalDescriptorTable::DataSegmentDescriptor()
{
    // Give us the offset of the Data Segment
    // To do that we get the adress of the descriptor and subtract the adress of the GDT.
    return (myxp::uint8_t*)&dataSegmentDescriptor - (uint8_t*)this;
}

myxp::uint16_t myxp::GlobalDescriptorTable::CodeSegmentDescriptor()
{
    // Give us the offset of the Code Segment
    // We do that the same way as with the Data Segment
    return (myxp::uint8_t*)&codeSegmentDescriptor - (uint8_t*)this;
}

myxp::uint32_t myxp::GlobalDescriptorTable::SegmentDescriptor::Base()
{
    // Calculate the base pointer in memory and return it
    myxp::uint8_t* target = (myxp::uint8_t*)this;
    myxp::uint32_t result = target[7];

    result = (result << 8) + target[4];
    result = (result << 8) + target[3];
    result = (result << 8) + target[2];

    return result;
}

myxp::uint32_t myxp::GlobalDescriptorTable::SegmentDescriptor::Limit()
{
    // Calculate the memory limit and return it
    myxp::uint8_t* target = (myxp::uint8_t*)this;
    myxp::uint32_t result = target[6] & 0xF;    // Grab the 4 bits of the shared 6th byte

    result = (result << 8) + target[1];
    result = (result << 8) + target[0];

    if((target[6] & 0xC0) == 0xC0)
    {
        result = (result << 12) | 0xFFF;
    }

    return result;
}