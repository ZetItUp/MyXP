/*
 * My Experience Operating System
 * Copyright (C) 2019, Andreas Lindström
 */
#ifndef __MYXP__GDT_H
#define __MYXP__GDT_H

#include <lib/types.h>

namespace myxp
{
    class GlobalDescriptorTable
    {
        public:
            class SegmentDescriptor
            {
                private:
                    myxp::uint16_t limit_low;
                    myxp::uint16_t base_low;
                    myxp::uint8_t base_high;
                    myxp::uint8_t type;
                    myxp::uint8_t limit_high;
                    myxp::uint8_t base_vhigh;

                public:
                    SegmentDescriptor(myxp::uint32_t base, myxp::uint32_t limit, myxp::uint8_t type);
                    myxp::uint32_t Base();
                    myxp::uint32_t Limit();
            } __attribute__((packed));

        private:
            SegmentDescriptor nullSegmentDescriptor;    // The null descriptor which is never referenced by the processor. 
                                                        // Certain emulators, like Bochs, will complain about limit exceptions if you do not have one present. 
                                                        // Some use this descriptor to store a pointer to the GDT itself (to use with the LGDT instruction). 
                                                        // The null descriptor is 8 bytes wide and the pointer is 6 bytes wide so it might just be the perfect place for this.
            
            //SegmentDescriptor unusedSegmentDescriptor;  // Unused for forward compatibility stuff? Not sure yet.

            SegmentDescriptor codeSegmentDescriptor;
            SegmentDescriptor dataSegmentDescriptor;
            SegmentDescriptor userCodeSegementDescriptor;
            SegmentDescriptor userDataSegementDescriptor;

        public:
            GlobalDescriptorTable();
            ~GlobalDescriptorTable();

            myxp::uint16_t CodeSegmentDescriptor();
            myxp::uint16_t DataSegmentDescriptor();

            void SetDescriptorByteGranularity(myxp::int32_t segment, myxp::uint32_t size, myxp::uint32_t base, myxp::int8_t access, myxp::int32_t dpl);
    };
}

#endif