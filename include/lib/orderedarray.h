/*
 * My Experience Operating System
 * Copyright (C) 2019, Andreas Lindström
 */
#ifndef __MYXP__ORDERED__ARRAY_H
#define __MYXP__ORDERED__ARRAY_H

#include <lib/types.h>
#include <system.h>

namespace myxp
{

    namespace ordered_array
    {
        /*
        *  This array is insertion sorted, it always remains in a sorted state (between calls).
        *  It can store anything that can be cast to a void*, so a uint32_t or any pointer.
        */
        typedef void* Type;

        /*
        *  A predicate should return non-zero if the first argument is less than the second.
        *  Else it should return zero.
        */
        typedef int8_t (*LessThanPredicate)(Type, Type);
        
        class OrderedArray
        {
        public:
            Type *array;
            uint32_t size;
            uint32_t maxSize;

            // A standard less than predicate
            LessThanPredicate LessThan;
            int8_t StandardLessThanPredicate(Type a, Type b);

            // Create an ordered array
            static OrderedArray CreateOrderedArray(uint32_t maxSize, LessThanPredicate lessThan);
            static OrderedArray PlaceOrderedArray(void *address, uint32_t maxSize, LessThanPredicate lessThan);

            OrderedArray();
            ~OrderedArray();

            // Destroy an ordered array
            void DestroyOrderedArray();
            uint32_t GetSize();
            uint32_t GetMaxSize();

            // Add an item into the array
            void Insert(Type item);
            Type LookupAt(uint32_t i);
            void RemoveAt(uint32_t i);
        };
    }

    typedef struct _heap
    {
        myxp::ordered_array::OrderedArray Index;
        myxp::uint32_t StartAddress;  // The start of our allocated space
        myxp::uint32_t EndAddress;    // The end of our allocated space. May be expanded up to MaxAddress.
        myxp::uint32_t MaxAddress;    // The maximum address the heap can be expanded to.
        myxp::uint8_t SuperUser;      // Should extra pages requested by us be mapped as SuperUser?
        myxp::uint8_t ReadOnly;       // Should extra pages requested by us be mapped as Read Only?
    } Heap;
}


#endif