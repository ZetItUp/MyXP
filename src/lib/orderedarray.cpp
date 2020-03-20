/*
 * My Experience Operating System
 * Copyright (C) 2019, Andreas Lindström
 */
#include <memorym.h>

myxp::ordered_array::OrderedArray::OrderedArray()
{

}

myxp::ordered_array::OrderedArray::~OrderedArray()
{

}

myxp::int8_t myxp::ordered_array::OrderedArray::StandardLessThanPredicate(myxp::ordered_array::Type a, myxp::ordered_array::Type b)
{
    return (a < b) ? 1 : 0;
}

myxp::ordered_array::OrderedArray myxp::ordered_array::OrderedArray::CreateOrderedArray(myxp::uint32_t maxSize, myxp::ordered_array::LessThanPredicate lessThan)
{
    myxp::ordered_array::OrderedArray arr;

    arr.array = (myxp::ordered_array::Type*)MemoryManager::activeMemoryManager->kmalloc(maxSize * sizeof(myxp::ordered_array::Type));
    MemoryManager::activeMemoryManager->kmemset(arr.array, 0, maxSize * sizeof(myxp::ordered_array::Type));
    arr.size = 0;
    arr.maxSize = maxSize;
    arr.LessThan = lessThan;

    return arr;
}

myxp::ordered_array::OrderedArray myxp::ordered_array::OrderedArray::PlaceOrderedArray(void *address, myxp::uint32_t maxSize, myxp::ordered_array::LessThanPredicate lessThan)
{
    myxp::ordered_array::OrderedArray arr;

    arr.array = (myxp::ordered_array::Type*)address;
    MemoryManager::activeMemoryManager->kmemset(arr.array, 0, maxSize * sizeof(myxp::ordered_array::Type));
    arr.size = 0;
    arr.maxSize = maxSize;
    arr.LessThan = lessThan;

    return arr;
}

void myxp::ordered_array::OrderedArray::DestroyOrderedArray()
{
    // kfree(array->array)
}

void myxp::ordered_array::OrderedArray::Insert(myxp::ordered_array::Type item)
{
    ASSERT(LessThan);
    uint32_t iter = 0; // Iterator

    while(iter < size && LessThan(array[iter], item))
    {
        iter++;
    }

    // Just add to the end of the array
    if(iter == size)
    {
        array[size++] = item;
    }
    else
    {
        myxp::ordered_array::Type tmp = array[iter];
        array[iter] = item;

        while(iter < size)
        {
            iter++;
            myxp::ordered_array::Type tmp2 = array[iter];
            array[iter] = tmp;
            tmp = tmp2;
        }

        size++;
    }
}

myxp::uint32_t myxp::ordered_array::OrderedArray::GetSize()
{
    return size;
}

myxp::uint32_t myxp::ordered_array::OrderedArray::GetMaxSize()
{
    return maxSize;
}

myxp::ordered_array::Type myxp::ordered_array::OrderedArray::LookupAt(myxp::uint32_t i)
{
    ASSERT(i < size);

    return array[i];
}

void myxp::ordered_array::OrderedArray::RemoveAt(myxp::uint32_t i)
{
    while(i < size)
    {
        array[i] = array[i + 1];
        i++;
    }

    size--;
}