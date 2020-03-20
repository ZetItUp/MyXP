/*
 * My Experience Operating System
 * Copyright (C) 2019, Andreas Lindström
 */
#ifndef __MYXP__PAGING_H
#define __MYXP__PAGING_H

#include <lib/types.h>

#define PAGE_SHIFT  12
// #define PAGE_SIZE       4096
#define PAGE_SIZE  (1 << PAGE_SHIFT)
#define PAGE_ENTRIES    1024
#define PAGE_MASK   (~(PAGE_SIZE - 1))

namespace myxp
{
    namespace paging
    {
        static inline void invlpg(void *m);

        typedef struct page
        {
            uint32_t Present        : 1;    // Page present in memory.
            uint32_t RW             : 1;    // Read/Write if set, Read Only if not set.
            uint32_t User           : 1;    // Supervisor if cleared.
            uint32_t WriteThrough   : 1;
            uint32_t CacheDisable   : 1;
            uint32_t Accessed       : 1;    // Has the page been accessed since last refresh?
            uint32_t Dirty          : 1;    // Has the page been written to since last refresh?
            uint32_t PAT            : 1;    // Physical Address Table
            uint32_t Global         : 1;
            uint32_t Unused         : 3;    // Amalgamation of unused and reserved bits
            uint32_t Frame          : 20;   // Frame address (Shifted right by 12 bits)
        } __attribute__((packed)) Page;

        typedef struct pageTable
        {
            Page Pages[PAGE_ENTRIES];
        } PageTable;
        
        typedef struct pageDirectory
        {
            // Array of pointers to pagetables.
            PageTable *Tables[PAGE_ENTRIES];

            /*
            *  Array of pointers to the pagetables above, but gives their _PHYSICAL_
            *  location, for loading into the CR3 register.
            */
            PageTable *PhysicalTables[PAGE_ENTRIES];

            /*
            * The physical address of PhysicalTables. This comes into play
            * when we get out kernel heap allocated and the directory may
            * be in a different location in virtual memory.
            */
            uintptr_t PhysicalAddr;
        } PageDirectory;
    }
}

#endif