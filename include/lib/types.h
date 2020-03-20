/*
 * My Experience Operating System
 * Copyright (C) 2019, Andreas Lindström
 */
#ifndef __MYXP__LIB__TYPES_H
#define __MYXP__LIB__TYPES_H

namespace myxp
{
    typedef char int8_t;
    typedef unsigned char uint8_t;
    typedef short int16_t;
    typedef unsigned short uint16_t;
    typedef int int32_t;
    typedef unsigned int uint32_t;
    typedef long long int int64_t;
    typedef unsigned long long int uint64_t;
    typedef uint32_t size_t;

    typedef uint32_t uintptr_t;
    typedef int32_t intptr_t;
    typedef uint32_t physaddr_t;
    typedef uint32_t physpagenum_t;
}

#endif