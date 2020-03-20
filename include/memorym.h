/*
 * My Experience Operating System
 * Copyright (C) 2019, Andreas Lindström
 */
#ifndef __MYXP__MEMORY__MANAGEMENT_H
#define __MYXP__MEMORY__MANAGEMENT_H

//#pragma once

#include <lib/types.h>
#include <paging.h>
#include <hardware/interrupts.h>
#include <lib/orderedarray.h>

#define ASSUME(cond) __extension__ ({ if (!(cond)) { __builtin_unreachable(); } })

#define INDEX_FROM_BIT(a)   (a / 0x20)
#define OFFSET_FROM_BIT(a)  (a % 0x20)

#define KERNEL_HEAP_END     0x20000000
#define KERNEL_HEAP_INIT    0x00800000

#define HEAP_INDEX_SIZE 0x20000
#define HEAP_MAGIC 0x123890AB
#define HEAP_MIN_SIZE 0x70000

extern void *end;

typedef struct _heapHeader
{
    myxp::uint32_t Magic; // Magic number, used for error checking and identification
    myxp::uint8_t isHole; // 1 if this is a hole. 0 if this is a block.
    myxp::uint32_t Size;  // Size of the block, including the end footer.
} HeapHeader;

typedef struct _heapFooter
{
    myxp::uint32_t Magic;     // Magic number, same as in HeapHeader
    HeapHeader *Header; // Pointer to the block header.
} HeapFooter;

void *memset(void *dest, int c, myxp::size_t n);

namespace myxp
{
    using namespace myxp::paging;
    
    /*
    struct MemoryChunk
    {
        MemoryChunk *next;
        MemoryChunk *prev;
        bool allocated;
        myxp::size_t size;
    };
     */    

    class MemoryManager : public hardware::InterruptHandler
    {
    protected:
        PageDirectory *kernelDirectory;
        PageDirectory *currentDirectory;

        uint32_t *frames;
        uint32_t nframes;

        // Memory Managers Heap
        myxp::Heap *mmHeap;
        myxp::uintptr_t heapEnd = 0;
        myxp::uintptr_t kernelHeapAllocPoint = KERNEL_HEAP_INIT;
        
        myxp::uintptr_t kheap_end = (myxp::uintptr_t)0;
        //uintptr_t kernel_heap_allocation_point = KERNEL_HEAP_INIT;
        myxp::uintptr_t placementAddress = (myxp::uintptr_t)&end;

        //MemoryChunk *first;
    protected:
        static int32_t FindSmallestHole(uint32_t size, uint8_t pageAlign, Heap *heap);
        static int8_t HeapHeaderLessThan(void *a, void *b);
        
    public:
        void *kmemset(void * dest, int c, size_t n);
        void *kmemcpy(void * __restrict dest, const void * __restrict src, size_t n);
        static MemoryManager *activeMemoryManager;

        MemoryManager(hardware::InterruptManager *intManager);

        void ExpandHeap(uint32_t newSize, Heap *heap);
        uint32_t ContractHeap(uint32_t newSize, Heap *heap);

        uint32_t NumOfFrames(int32_t num);
        void SetFrame(uint32_t frameAddr);
        void ClearFrame(uint32_t frameAddr);
        void DMAFrame(Page *page, int isKernel, int isWriteable, uintptr_t address);
        uint32_t TestFrame(uint32_t frameAddr);
        uint32_t FirstFrame();
        void AllocFrame(Page *page, int isKernel, int isWritable);
        void FreeFrame(Page *page);
        uintptr_t MapToPhysical(uintptr_t virtualAddress);

        // Sets up the environment, page directories, etc and enables paging.
        void InitializePaging(uint32_t memorySize);
        Heap *CreateHeap(uint32_t start, uint32_t end, uint32_t max, uint8_t superUser, uint8_t readOnly);
        void MemoryManager::MarkPagingSystem(uint64_t address);
        void CompletePaging();

        Heap *GetHeap();
        uintptr_t *GetHeapEnd();

        // Causes the specified page directory to be loaded into the CR3 register.
        void SwitchPageDirectory(PageDirectory *newDir);
        PageDirectory *CloneDirectory(PageDirectory *dir);

        Page *GetPage(uint32_t address, int make, PageDirectory *dir);
        PageDirectory *GetKernelDirectory();
        //void* malloc(myxp::size_t size);
        //void free(void *ptr);

        void *alloc(uint32_t size, uint8_t pageAlign, Heap *heap);
        void free(void *p, Heap *heap);

        virtual uint32_t HandleInterrupt(uint32_t esp);
        virtual void Activate();

        myxp::uintptr_t kmallocReal(myxp::size_t size, myxp::int32_t align, uintptr_t *phys);
        myxp::uint32_t kmalloc_a(uint32_t size);
        myxp::uint32_t kmalloc_int(uint32_t size, int align, uint32_t *phys);
        myxp::uint32_t kmalloc_p(uint32_t size, uint32_t *phys);
        myxp::uint32_t kmalloc_ap(uint32_t size, uint32_t *phys);
        myxp::uintptr_t kmalloc(uint32_t size);

        void kmalloc_startAt(uintptr_t address);
        void kfree(void *ptr);
    };
}

#endif

/*

    OLD STUFF

// Kernel Stack and Base
#define KERNEL_BASE         0xF0000000
#define KERNEL_STACK_TOP    KERNBASE
#define KERNEL_STACK_SIZE   (8 * PAGE_SIZE)     // Size of a kernel stack
#define KERNEL_STACK_GAP    (8 * PAGE_SIZE)     // Size of a kernel stack guard

// At IOPHYSMEM (640K) there is a 384K hole for I/O.  From the kernel,
// IOPHYSMEM can be addressed at KERNBASE + IOPHYSMEM.  The hole ends
// at physical address EXTPHYSMEM.
#define IOPHYSMEM           0x0A0000            // 640 kB
#define EXTPHYSMEM          0x100000            // 1048 kB

// Memory-mapped IO.
#define MMIOLIM		(KERNEL_STACK_TOP - PAGE_TABLE_SIZE)
#define MMIOBASE	(MMIOLIM - PTSIZE)

#define ULIM		(MMIOBASE)

 */

//#endif