/*
 * My Experience Operating System
 * Copyright (C) 2019, Andreas Lindström
 */
#include <memorym.h>
#include <console.h>

using namespace myxp;
using namespace myxp::paging;

void MemoryManager::kmalloc_startAt(uintptr_t address)
{
	placementAddress = address;
}

void MemoryManager::kfree(void *ptr)
{
	free(ptr, GetHeap());
}

uintptr_t MemoryManager::kmallocReal(myxp::size_t size, myxp::int32_t align, uintptr_t *phys)
{
	if(MemoryManager::activeMemoryManager->GetHeapEnd())
	{		
		void *addr = alloc(size, (uint8_t)align, MemoryManager::activeMemoryManager->GetHeap());

		if(phys != 0)
		{
			Page *p = MemoryManager::activeMemoryManager->GetPage((uint32_t)addr, 0, kernelDirectory);
			*phys = p->Frame * 0x1000 + (uint32_t)addr & 0xFFF;
		}

		return (uint32_t)addr;
	}
	else
	{
		/* 0xC0000000 - 0xC0409004
			3221225472	3225456644

		* This will eventually call malloc() on the kernel heap.
		* For now we just assign memory at placementAddress and
		* increment it by size.
		* 
		* Even when we've coded our kernel heap, this will be useful
		* for use before the heap is initialized.
		*/
		if(align == 1 && (MemoryManager::activeMemoryManager->placementAddress & 0x00000FFF))
		{
			MemoryManager::activeMemoryManager->placementAddress &= 0xFFFFF000;
			MemoryManager::activeMemoryManager->placementAddress += 0x1000;
		}

		if(phys)
		{
			*phys = MemoryManager::activeMemoryManager->placementAddress;
		}

		uintptr_t tmp = MemoryManager::activeMemoryManager->placementAddress;
		MemoryManager::activeMemoryManager->placementAddress += size;

		return (uintptr_t)tmp;
	}
}

uintptr_t MemoryManager::kmalloc(size_t size)
{
	return kmallocReal(size, 0, 0);
}

uint32_t MemoryManager::kmalloc_a(uint32_t size)
{
	return kmallocReal(size, 1, 0);
}

uint32_t MemoryManager::kmalloc_p(uint32_t size, uint32_t *phys)
{
	return kmallocReal(size, 0, phys);
}

uint32_t MemoryManager::kmalloc_ap(uint32_t size, uint32_t *phys)
{
	return kmallocReal(size, 1, phys);
}

/* // Copy len bytes from src to dest.
void *kkmemcpy(void* dest, const uint32_t *src, uint32_t len)
{
    const uint8_t *sp = (const uint8_t *)src;
    uint8_t *dp = (uint8_t *)dest;

    for(size_t i = 0; i < len; i++) 
	{
		dp[i] = sp[i];
	}

	return dp;
}

// Write len copies of val into dest.
void *kmemset(void *dest, uint32_t val, uint32_t len)
{
    uint8_t *temp = (uint8_t *)dest;

    for (size_t i = 0; i < len; i++) 
	{
		temp[i] = (uint8_t)val;
	}

	return temp;
} */

void *MemoryManager::kmemcpy(void * __restrict dest, const void * __restrict src, size_t n) 
{
	asm volatile("cld; rep movsb": "=c"((int){0}) : "D"(dest), "S"(src), "c"(n) : "flags", "memory");
	
	return dest;
}

void *MemoryManager::kmemset(void * dest, int c, size_t n) 
{
	//asm volatile("cld; rep stosb": "=c"((int){0}) : "D"(dest), "a"(c), "c"(n) : "flags", "memory");
	
	myxp::uint8_t val = (myxp::uint8_t)(c & 0xFF);
	myxp::uint8_t *dest2 = (uint8_t*)(dest);

	myxp::size_t i = 0;

	while(i < n)
	{
		dest2[i] = val;
		i++;
	}

	return dest;
}

MemoryManager *myxp::MemoryManager::activeMemoryManager = 0;

MemoryManager::MemoryManager(hardware::InterruptManager *intManager)
	: InterruptHandler(0x0E, intManager)
{
	if(activeMemoryManager != 0)
	{
		// TODO: Add debug message, only allow one active memorymanager...

		return;
	}

	activeMemoryManager = this;
}

void *memset(void *dest, int c, myxp::size_t n)
{
	
}

Heap *MemoryManager::GetHeap()
{
	return mmHeap;
}

uintptr_t *MemoryManager::GetHeapEnd()
{
	return heapEnd;
}

// Set a bit in the frames bitset
void MemoryManager::SetFrame(uint32_t frameAddr)
{
	if(frameAddr < nframes * 0x1000)
	{
		uint32_t frame = frameAddr / 0x1000;
		uint32_t index = INDEX_FROM_BIT(frame);
		uint32_t off = OFFSET_FROM_BIT(frame);

		frames[index] |= ((uint32_t)0x1 << off);
	}
}

uint32_t MemoryManager::NumOfFrames(int32_t num)
{
	for(uint32_t i = 0; i < nframes * 0x1000; i+= 0x1000)
	{
		int isBad = 0;

		for(int j = 0; j < num; ++j)
		{
			if(TestFrame(i + 0x1000 * j))
			{
				isBad = j + 1;
			}
		}

		if(!isBad)
		{
			return i / 0x1000;
		}
	}

	return 0xFFFFFFFF;
}

// Clear a bit in the frames bitset
void MemoryManager::ClearFrame(uint32_t frameAddr)
{
	uint32_t frame = frameAddr / 0x1000;
	uint32_t index = INDEX_FROM_BIT(frame);
	uint32_t off = OFFSET_FROM_BIT(frame);

	frames[index] &= ~((uint32_t)0x1 << off);
}

uint32_t MemoryManager::TestFrame(uint32_t frameAddr)
{
	uint32_t frame = frameAddr / 0x1000;
	uint32_t index = INDEX_FROM_BIT(frame);
	uint32_t off = OFFSET_FROM_BIT(frame);

	return (frames[index] & ((uint32_t)0x1 << off));
}

uint32_t MemoryManager::FirstFrame()
{
	uint32_t i;
	uint32_t j;

	for(i = 0; i < INDEX_FROM_BIT(nframes); i++)
	{
		// Nothing free, exit
		if(frames[i] != 0xFFFFFFFF)
		{
			// At least one bit is free here.
			for(j = 0; j < 32; j++)
			{
				uint32_t test = (uint32_t)0x1 << j;

				if(!(frames[i] & test))
				{
					return i * 0x20 + j;
				}
			}
		}
	}
}

// Allocate a frame
void MemoryManager::AllocFrame(Page *page, int isKernel, int isWritable)
{
	/* if(page->Frame == 0)
	{
		printf("\nPage: 0x");
		printfHex32(page);
		printf("\n");
	} */

	ASSUME(page != nullptr);

	if(page->Frame != 0)
	{
		// Frame is already allocated, return
		page->Present = 1;
		page->RW = (isWritable == 1) ? 1 : 0;
		page->User = (isKernel == 1) ? 0 : 1;
		return;
	}
	else
	{
		uint32_t index = FirstFrame();	// Get the first free frame

		ASSERT(index != (uint32_t)-1);

		SetFrame(index * 0x1000);			// Claim the frame.
		page->Frame = index;
		page->Present = 1;					// Mark it as present
		page->RW = (isWritable == 1) ? 1 : 0;	// Should the page be writeable?
		page->User = (isKernel == 1) ? 0 : 1;	// Should the page be in User Mode?
	}
}

uintptr_t MemoryManager::MapToPhysical(uintptr_t virtualAddress)
{
	uintptr_t remaining = virtualAddress % 0x1000;
	uintptr_t frame = virtualAddress / 0x1000;
	uintptr_t table = frame / 1024;
	uintptr_t subframe = frame % 1024;

	if(currentDirectory->Tables[table])
	{
		Page *p = &currentDirectory->Tables[table]->Pages[subframe];

		return p->Frame * 0x1000 + remaining;
	}
	else
	{
		return 0;
	}	
}

// Deallocate a frame
void MemoryManager::FreeFrame(Page *page)
{
	uint32_t frame;

	if(!(frame = page->Frame))
	{
		// The page did not hae an allocated frame.
		return;
	}
	else
	{
		ClearFrame(frame);  // Clear the frame
		page->Frame = 0x0;	// Remove the page's frame
	}
}

void MemoryManager::DMAFrame(Page *page, int isKernel, int isWriteable, uintptr_t address)
{
	ASSUME(page != nullptr);

	page->Present = 1;
	page->RW = (isWriteable) ? 1 : 0;
	page->User = (isKernel) ? 0 : 1;
	page->Frame = address / 0x1000;
	SetFrame(address);
}

void MemoryManager::InitializePaging(uint32_t memorySize)
{
	nframes = memorySize / 4;
	frames = (uint32_t *)kmalloc(INDEX_FROM_BIT(nframes * 8));
	kmemset(frames, 0, INDEX_FROM_BIT(nframes * 8));

	uintptr_t phys;
	kernelDirectory = (PageDirectory*)kmalloc_ap(sizeof(PageDirectory), &phys);
	kmemset(kernelDirectory, 0, sizeof(PageDirectory));

	// Set PAT 111b to Write-Combining
	/* asm volatile (
		"mov $0x277, %%ecx\n" // IA32_MSR_PAT 
		"rdmsr\n"
		"or $0x1000000, %%edx\n" // set bit 56 
		"and $0xf9ffffff, %%edx\n" // unset bits 57, 58 
		"wrmsr\n"
		: : : "ecx", "edx", "eax"
	);   */
}

void MemoryManager::CompletePaging()
{
	/* printf("Placement Address is at: 0x");
	printfHex32(placementAddress);
	printf("\n"); */

	GetPage(0, 1, kernelDirectory)->Present = 0;
	SetFrame(0);
	/*
	 * Map some pages in the kernel heap area.
	 * Here we call GetPage() but not AllocFrame(). This causes PageTable's
	 * to be created where necessary. We can't allocate frames yet because they
	 * need to be identity mapped first below, and yet we can't increase
	 * placementAddress between identity mapping and enabling the heap!
	 */
	for(uintptr_t i = 0x1000; i < 0x80000; i += 0x1000)
	{
		DMAFrame(GetPage(i, 1, kernelDirectory), 1, 0, i);
	}

	for (uintptr_t i = 0x80000; i < 0x100000; i += 0x1000) 
	{
		DMAFrame(GetPage(i, 1, kernelDirectory), 1, 0, i);
	}

	/* printf("\nPlacement: 0x");
	printfHex32(placementAddress);
	printf("\n"); */

	for (uintptr_t i = 0x100000; i < placementAddress + 0x3000; i += 0x1000) 
	{
		DMAFrame(GetPage(i, 1, kernelDirectory), 1, 0, i);
	}

	// VGA Map
	for (uintptr_t i = 0xb8000; i < 0xc0000; i += 0x1000) 
	{
		DMAFrame(GetPage(i, 0, kernelDirectory), 0, 1, i);
	}

	/*
	 * We need to identity map (physical address = virtual address) from
	 * 0x0 to the end of used memory, so we can access this transparently,
	 * as if paging wasn't enabled.
	 * 
	 * NOTE:
	 * We use a while loop here deliberately.
	 * Inside the loop body we actually change placementAddress by calling
	 * kmalloc(). A while loop causes this to be computed on-the-fly rather
	 * than once at the start.
	 * 
	 * Allocate a little bit extra so the kernel heap can be initialized properly.
	 */
	//ClearScreen();

	/* printf("\nKernel Directory: 0x");
	printfHex32(kernelDirectory->PhysicalAddr);
	printf("\n"); */
	kernelDirectory->PhysicalAddr = (uintptr_t)kernelDirectory->PhysicalTables;

	uintptr_t tmpHeapStart = KERNEL_HEAP_INIT;

	if(tmpHeapStart >= placementAddress + 0x3000)
	{
		/* errMsg("Bad heap address!");
		printf("Temp Heap Start: 0x");
		printfHex32(tmpHeapStart);
		printf("\nPlacement Address Offset: 0x");
		printfHex32(placementAddress + 0x3000); */
		tmpHeapStart = placementAddress + 0x100000;
		kernelHeapAllocPoint = tmpHeapStart;
		/* printf("\nNew Temp Heap Start: 0x");
		printfHex32(tmpHeapStart);
		printf("\n"); */
	}

	// Kernel Heap Space
	for(uintptr_t i = placementAddress + 0x3000; i < tmpHeapStart; i += 0x1000)
	{
		AllocFrame(GetPage(i, 1, kernelDirectory), 1, 0);
	}

	// Preallocate the page entries for all the rest of the kernel heap aswell
	for(uintptr_t i = tmpHeapStart; i <= KERNEL_HEAP_END; i += 0x1000)
	//for(uintptr_t i = 0x100000; i < KERNEL_HEAP_END; i += 0x1000)
	{
		GetPage(i, 1, kernelDirectory);
	}

	/* for(uintptr_t i = 0xE000; i <= 0xFFF0; i += 0x40)
	{
		GetPage(i << 16UL, 1, kernelDirectory);
	}   */

	// Enable paging!
	//printf("\nPlacement: 0x");
	//printfHex32((placementAddress + 0x1000) & ~0xFFF);
	/* printf("\nKernel: 0x");
	printfHex32(kernelDirectory);
	printf("\nKernel Phys Addr: 0x");
	printfHex32(kernelDirectory->PhysicalAddr);
	printf("\nKernel Phys Tables: 0x");
	printfHex32(kernelDirectory->PhysicalTables); */
	SwitchPageDirectory(kernelDirectory);

	heapEnd = (placementAddress + 0x1000) & ~0xFFF;

	//mmHeap = CreateHeap(KERNEL_HEAP_INIT, KERNEL_HEAP_INIT + KERNEL_HEAP_END, 0xCFFFF000, 0, 0);
	//mmHeap = CreateHeap(0x100000, KERNEL_HEAP_END, 0xCFFFF000, 0, 0);

	/* printf("\nHeap End: 0x");
	printfHex32(heapEnd);
	printf("\n"); */

	/* uintptr_t phys;
	kernelDirectory = (PageDirectory *)kmalloc_ap(sizeof(PageDirectory), &phys);
	kmemset(kernelDirectory, 0, sizeof(PageDirectory)); */

	/* 3F209004
	 * We need to identity map (phys addr = virt addr) from 0x0 to
	 * the end of the used memory, so we can access this transparently.
	 * 
	 * NOTE:
	 * We use a while loop here deliberatley.
	 * Inside the loop body we actually change placementAddress by 
	 * calling kmalloc(). A while loop causes this to be computed
	 * on-the-fly rather than once at the start.
	 */
/* 	int i = 0;

	while(i < placementAddress)
	{
		// Kernel code is readable but not writeable from userspace.
		this->AllocFrame(GetPage(i, 1, kernelDirectory), 0, 0);
		i += 0x1000;
	} */

	// Enable paging

	/* kernelDirectory->PhysicalAddr = *kernelDirectory->PhysicalTables;
	printf("Kernel Directory: 0x");
	printfHex32(kernelDirectory->PhysicalAddr);
	printf("\n");
	SwitchPageDirectory(kernelDirectory);  */
}

void MemoryManager::SwitchPageDirectory(PageDirectory *newDir)
{
	//printf("SPD");
	currentDirectory = newDir;

	//printfHex32(&newDir->PhysicalTables);

	//uint32_t a = 2 / 0;

	/* asm volatile("mov %0, %%cr3":: "r"(&newDir->PhysicalTables));
    uint32_t cr0;
	//printf("Hai");
	asm volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000; // Enable paging!
	// interruptManager->HandleInterrupt(0x0E, 0x0E);
	// printf("Wut?");
    asm volatile("mov %0, %%cr0;":: "r"(cr0)); */

	/* printf("\n*newDir PhysAddr: 0x");
	printfHex32(newDir->PhysicalAddr);
	printf("\n");

	printf("&newDir PhysAddr: 0x");
	printfHex32(&newDir->PhysicalAddr);
	printf("\n");

	printf("\n*newDir PhysTables: 0x");
	printfHex32(newDir->PhysicalTables);
	printf("\n");

	printf("&newDir PhysTables: 0x");
	printfHex32(&newDir->PhysicalTables);
	printf("\n");

	printf("\n*newDir Tables: 0x");
	printfHex32(newDir->Tables);
	printf("\n");

	printf("&newDir Tables: 0x");
	printfHex32(&newDir->Tables);
	printf("\n"); */

 	asm volatile (
			"mov %0, %%cr3\n"
			"mov %%cr0, %%eax;\n"
			"orl $0x80000000, %%eax;\n"
			"mov %%eax, %%cr0\n"
			:: "r"(newDir->PhysicalAddr)
			: "%eax"); 

	//printf("DONE");
}

PageDirectory *MemoryManager::CloneDirectory(PageDirectory *dir)
{
	// Allocate a new page directory and clear it
	uintptr_t phys;
	PageDirectory *newDir = (PageDirectory*)kmalloc_ap(sizeof(PageDirectory), &phys);
	kmemset(newDir, 0, sizeof(PageDirectory));

	newDir->PhysicalAddr = phys;

	for(uint32_t i = 0; i < 1024; i++)
	{
		if(!dir->Tables[i] || (uintptr_t)dir->Tables[i] == (uintptr_t)0xFFFFFFF)
		{
			continue;
		}

		if(kernelDirectory->Tables[i] == dir->Tables[i])
		{
			// Kernel tables are linked
			newDir->Tables[i] = dir->Tables[i];
			newDir->PhysicalTables[i] = dir->PhysicalTables[i];
		}
		else
		{
			// TODO: Clone table
		}
	}

	return newDir;
}

void MemoryManager::free(void *p, Heap *heap)
{
	// Exit gracefully for null pointers
	if(p == 0)
	{
		return;
	}

	// Get the header and footer for this pointer
	HeapHeader *header = (HeapHeader*)((uint32_t)p - sizeof(HeapHeader));
	HeapFooter *footer = (HeapFooter*)((uint32_t)header + header->Size - sizeof(HeapFooter));

	// Sanity checks
	ASSERT(header->Magic == HEAP_MAGIC);
	ASSERT(footer->Magic == HEAP_MAGIC);

	// Make us a hole
	header->isHole = 1;

	// Do we want to add this header into the "Free Holes" index?
	char doAdd = 1;

	// Unify left. If the thing to the left of us is a footer...
	HeapFooter *testFooter = (HeapFooter*)((uint32_t)header - sizeof(HeapFooter));

	if(testFooter->Magic == HEAP_MAGIC && testFooter->Header->isHole == 1)
	{
		uint32_t cacheSize = header->Size;	// Cache our current size.
		header = testFooter->Header;		// Rewrite our header with the new one.
		footer->Header = header;			// Rewrite our footer to point to, to the new header.
		header->Size += cacheSize;			// Change the size.
		doAdd = 0;							// Since this header is already in the index, we do not
											// want to add it again.
	}

	// Unify right. If the thing to the right of us is a header...
	HeapHeader *testHeader = (HeapHeader*)((uint32_t)footer + sizeof(HeapFooter));

	if(testHeader->Magic == HEAP_MAGIC && testHeader->isHole == 1)
	{
		header->Size += testHeader->Size;	// Increase our size.
		// Rewrite it's footer to point to our header.
		testFooter = (HeapFooter*)((uint32_t)testHeader + testHeader->Size - sizeof(HeapFooter));
		footer = testFooter;

		// Find and remove this header from the index.
		uint32_t iter = 0;

		while((iter < heap->Index.GetSize()) && 
			(heap->Index.LookupAt(iter) != (void*)testHeader))
		{
			iter++;
		}

		// Make sure we actually found the item.
		ASSERT(iter < heap->Index.GetSize());

		// Remove it.
		heap->Index.RemoveAt(iter);
	}

	// If the footer location is the end address, we can contract?
	if((uint32_t)footer + sizeof(HeapFooter) == heap->EndAddress)
	{
		uint32_t oldLength = heap->EndAddress - heap->StartAddress;
		uint32_t newLength = ContractHeap((uint32_t)header - heap->StartAddress, heap);

		// Check how big we will be after resizing.
		if(header->Size - (oldLength - newLength) > 0)
		{
			// We will still exist, so resize us.
			header->Size -= oldLength - newLength;

			footer = (HeapFooter*)((uint32_t)header + header->Size - sizeof(HeapFooter));
			footer->Magic = HEAP_MAGIC;
			footer->Header = header;
		}
		else
		{
			// We will no longer exist. Remove us from the index.
			uint32_t iter = 0;

			while((iter < heap->Index.GetSize() &&
				heap->Index.LookupAt(iter) != (void*)testHeader))
			{
				iter++;	
			}

			// If we didn't find ourselves, we have nothing to remove.
			if(iter < heap->Index.GetSize())
			{
				heap->Index.RemoveAt(iter);
			}
		}
	}

	// If required, add us ot the index.
	if(doAdd == 1)
	{
		heap->Index.Insert((void*)header);
	}
}

void *MemoryManager::alloc(uint32_t size, uint8_t pageAlign, Heap *heap)
{
	// Make sure we take the size of header and footer into account
	uint32_t newSize = size + sizeof(HeapHeader) + sizeof(HeapFooter);

	// Find the smallest hole that will fit
	int32_t iter = FindSmallestHole(newSize, pageAlign, heap);

	// If we don't find a suitable hole...
	if(iter == -1)
	{
		uint32_t oldLength = heap->EndAddress - heap->StartAddress;
		uint32_t oldEndAddress = heap->EndAddress;

		// We need to allocate more space!
		ExpandHeap(oldLength + newSize, heap);
		uint32_t newLength = heap->EndAddress - heap->StartAddress;

		// Fin the header closest to the end (Location wise, not size!)
		iter = 0;
		uint32_t index = -1;
		uint32_t value = 0x0;

		while(iter < heap->Index.GetSize())
		{
			uint32_t tmp = (uint32_t)heap->Index.LookupAt(iter);

			if(tmp > value)
			{
				value = tmp;
				index = iter;
			}

			iter++;
		}

		// If we don't find ANY headers, add one!
		if(index == -1)
		{
			HeapHeader *header = (HeapHeader*)oldEndAddress;
			header->Magic = HEAP_MAGIC;
			header->Size = newLength - oldLength;
			header->isHole = 1;

			HeapFooter *footer = (HeapFooter*)(oldEndAddress + header->Size - sizeof(HeapFooter));
			footer->Magic = HEAP_MAGIC;
			footer->Header = header;
			heap->Index.Insert((void*)header);
		}
		else
		{
			// The last header needs adjusting
			HeapHeader *header = (HeapHeader*)heap->Index.LookupAt(index);
			header->Size += newLength - oldLength;

			// Rewrite the footer
			HeapFooter *footer = (HeapFooter*)(uint32_t)header + header->Size - sizeof(HeapFooter);
			footer->Header = header;
			footer->Magic = HEAP_MAGIC;
		}

		// We should have enough space, call ourselves recursive.
		return alloc(size, pageAlign, heap);
	}

	HeapHeader *origHoleHeader = (HeapHeader *)heap->Index.LookupAt(iter);
	uint32_t origHolePos = (uint32_t)origHoleHeader;
	uint32_t origHoleSize = origHoleHeader->Size;

	// Should we split the hole in two parts?
	// Is the original hole sie - requested hole ssize less than the overhead for
	// adding a new hole?
	if(origHoleSize - newSize < sizeof(HeapHeader) + sizeof(HeapFooter))
	{
		// If so, increase the requested size to the size of the hole we found
		size += origHoleSize - newSize;
		newSize = origHoleSize;
	}

	// Page align and make a new hole in front of our block.
	// Save address and size of the header if we need to overwrite it later.
	if(pageAlign && (origHolePos & 0xFFFFF000))
	{
		// 0x1000 = 4096 (Page Aligned)
		uint32_t newLocation = origHolePos + 0x1000 - (origHolePos & 0xFFF) - sizeof(HeapHeader);
		
		HeapHeader *holeHeader = (HeapHeader *)origHolePos;
		holeHeader->Size = 0x1000 - (origHolePos & 0xFFF) - sizeof(HeapHeader);
		holeHeader->Magic = HEAP_MAGIC;
		holeHeader->isHole = 1;

		HeapFooter *holeFooter = (HeapFooter*)((uint32_t)newLocation - sizeof(HeapFooter));
		holeFooter->Magic = HEAP_MAGIC;
		holeFooter->Header = holeHeader;
		origHolePos = newLocation;
		origHoleSize = origHoleSize - holeHeader->Size;
	}
	else
	{
		// Else we don't need this anymore, delete it!
		heap->Index.RemoveAt(iter);
	}
	
	// Overwrite the original header and footer
	HeapHeader *blockHeader = (HeapHeader*)origHolePos;
	blockHeader->Magic = HEAP_MAGIC;
	blockHeader->isHole = 0;
	blockHeader->Size = newSize;

	HeapFooter *blockFooter = (HeapFooter*)(origHolePos + sizeof(HeapHeader) + size);
	blockFooter->Magic = HEAP_MAGIC;
	blockFooter->Header = blockHeader;

	// We may need to write a new hole after the allocated block
	// We do this only if the new hole would have positive size!
	if(origHoleSize - newSize > 0)
	{
		HeapHeader *holeHeader = (HeapHeader*)(origHolePos + sizeof(HeapHeader) + size + sizeof(HeapFooter));
		holeHeader->Magic = HEAP_MAGIC;
		holeHeader->isHole = 1;
		holeHeader->Size = origHoleSize - newSize;

		HeapFooter *holeFooter = (HeapFooter*)((uint32_t)holeHeader + origHoleSize - newSize - sizeof(HeapFooter));
		if((uint32_t)holeFooter < heap->EndAddress)
		{
			holeFooter->Magic = HEAP_MAGIC;
			holeFooter->Header = holeHeader;
		}

		// Put the new hole in the index!
		heap->Index.Insert((void*)holeHeader);
	}

	// Done!
	return (void *)((uint32_t)blockHeader + sizeof(HeapHeader));
}

int8_t MemoryManager::HeapHeaderLessThan(void *a, void *b)
{
	return (((HeapHeader*)a)->Size < ((HeapHeader*)b)->Size) ? 1 : 0;
}

int32_t MemoryManager::FindSmallestHole(uint32_t size, uint8_t pageAlign, Heap *heap)
{
	// Iterator
	uint32_t iter = 0;

	while(iter < heap->Index.GetSize())
	{
		HeapHeader *header = (HeapHeader *)heap->Index.LookupAt(iter);

		if(pageAlign > 0)
		{
			// Page align the starting point of this header
			uint32_t location = (uint32_t)header;
			int32_t offset = 0;

			if((location + sizeof(HeapHeader) & 0xFFFFF000) != 0)
			{
				offset = 0x1000 - (location + sizeof(HeapHeader)) % 0x1000;
			}

			int32_t holeSize = (int32_t)header->Size - offset;

			// Can we fit now?
			if(holeSize >= (int32_t)size)
			{
				break;
			}
		}
		else if(header->Size >= size)
		{
			break;
		}

		iter++;
	}

	// Why did the loop exit?
	if(iter == heap->Index.GetSize())
	{
		return -1;	// We got to the end and didn't find anything.
	}
	else
	{
		return iter;
	}
	
}

void MemoryManager::MarkPagingSystem(uint64_t address)
{
	SetFrame(address);
}

void MemoryManager::ExpandHeap(uint32_t newSize, Heap *heap)
{
	// Sanity check!
	ASSERT(newSize > heap->EndAddress - heap->StartAddress);

	if(newSize & 0xFFFFF000 != 0)
	{
		newSize &= 0xFFFFF000;
		newSize += 0x1000;
	}

	// Make sure we don't go beyond our scope!
	ASSERT(heap->StartAddress + newSize <= heap->MaxAddress);

	// This should always be on page boundary
	uint32_t oldSize = heap->EndAddress - heap->StartAddress;
	uint32_t i = oldSize;

	while(i < newSize)
	{
		AllocFrame(GetPage(heap->StartAddress + i, 1, kernelDirectory), (heap->SuperUser) ? 1 : 0, (heap->ReadOnly) ? 0 : 1);
		i += 0x1000;	// Page Size
	}

	heap->EndAddress = heap->StartAddress + newSize;
}

uint32_t MemoryManager::ContractHeap(uint32_t newSize, Heap *heap)
{
	// Sanity Check!
	ASSERT(newSize < heap->EndAddress - heap->StartAddress);

	// Get the nearest following page boundary
	if(newSize & 0x1000)
	{
		newSize &= 0x1000;
		newSize += 0x1000;
	}

	// Don't contract too far!
	if(newSize < HEAP_MIN_SIZE)
	{
		newSize = HEAP_MIN_SIZE;
	}

	uint32_t oldSize = heap->EndAddress - heap->StartAddress;
	uint32_t i = oldSize - 0x1000;

	while(newSize < i)
	{
		FreeFrame(GetPage(heap->StartAddress + i, 0, kernelDirectory));
		i -= 0x1000;
	}

	heap->EndAddress = heap->StartAddress + newSize;

	return newSize;
}

Heap *MemoryManager::CreateHeap(uint32_t start, uint32_t end, uint32_t max, uint8_t superUser, uint8_t readOnly)
{
	Heap *heap = (Heap*)kmalloc(sizeof(Heap));

	// Make sure everything is Page aligned
	ASSERT(start % 0x1000 == 0);
	ASSERT(end % 0x1000 == 0);

	heap->Index = ordered_array::OrderedArray::PlaceOrderedArray((void*)start, HEAP_INDEX_SIZE, &HeapHeaderLessThan);
	for(;;);
	// Shift the start address forward to resemble where we can start putting data
	start += sizeof(ordered_array::Type)*HEAP_INDEX_SIZE;

	// Make sure the start address is Page Aligned
	if(start % 0xFFFFF000 != 0)
	{
		start &= 0xFFFFF000;
		start += 0x1000;
	}

	// Write the Heap into it's structure
	heap->StartAddress = start;
	heap->EndAddress = end;
	heap->MaxAddress = max;
	heap->SuperUser = superUser;
	heap->ReadOnly = readOnly;

	// We start off with one large hole in the index
	HeapHeader *hole = (HeapHeader*)start;
	hole->Size = end - start;
	hole->Magic = HEAP_MAGIC;
	hole->isHole = 1;

	mmHeap->Index.Insert((void*)hole);

	return heap;
}

uint32_t MemoryManager::HandleInterrupt(uint32_t esp)
{
	// Handle Page Fault!
	// The faulting address is stored in the CR2 register
	uint32_t faultingAddress;

	asm volatile("mov %%cr2, %0" : "=r" (faultingAddress));

	struct hardware::StackFrame *sframe = *((struct hardware::StackFrame **)esp);

	printf("\nPage Address: 0x");
	printfHex32(faultingAddress >> PAGE_SHIFT);
	printf("\nStack Pointer: 0x");
	printfHex32((uint32_t)&sframe->esp >> PAGE_SHIFT);
	printf("\n");

	printf("Shift: 0x");
	printfHex32(PAGE_SHIFT);
	printf("\nSize: 0x");
	printfHex32(PAGE_SIZE);
	printf("\nMask: 0x");
	printfHex32(PAGE_MASK);
    
	//printfHex32(eax);
	panic("Page Fault!");

	return esp;
}

void MemoryManager::Activate()
{
	
}

Page *MemoryManager::GetPage(uint32_t address, int make, PageDirectory *dir)
{
	// Turn the address into an index
	address /= 0x1000;

	// Find the page table containing this address.
	uint32_t tableIndex = address / 1024;

	// If this table is already assigned
	if(dir->Tables[tableIndex])
	{
		return &dir->Tables[tableIndex]->Pages[address % 1024];
	}
	else if(make)
	{
		uint32_t tmp;
		dir->Tables[tableIndex] = (PageTable*)kmalloc_ap(sizeof(PageTable), (uintptr_t *)(&tmp));
		ASSUME(dir->Tables[tableIndex] != nullptr);

		kmemset(dir->Tables[tableIndex], 0, sizeof(PageTable));
		dir->PhysicalTables[tableIndex] = tmp | 0x7;	// Set Present, RW, User

		return &dir->Tables[tableIndex]->Pages[address % 1024];
	}
	else
	{
		return 0;
	}
}
/*

	OLD MEMORY MANAGER

myxp::MemoryManager *myxp::MemoryManager::activeMemoryManager = 0;

myxp::MemoryManager::MemoryManager(myxp::size_t start, myxp::size_t size)
{
	if(activeMemoryManager != 0)
	{
		// TODO: Add debug message, only allow one active memorymanager...

		return;
	}

	activeMemoryManager = this;

	if(size < sizeof(MemoryChunk))
	{
		first = 0;
	}
	else
	{
		first = (MemoryChunk*)start;

		first->allocated = false;
		first->prev = 0;
		first->next = 0;
		first->size = size - sizeof(MemoryChunk);
	}
}

myxp::MemoryManager::~MemoryManager()
{
	if(activeMemoryManager == this)
	{
		activeMemoryManager = 0;
	}
}

void* myxp::MemoryManager::malloc(myxp::size_t size)
{
	MemoryChunk *result = 0;

	for(MemoryChunk* chunk = first; chunk != 0 && result == 0; chunk = chunk->next)
	{
		if(chunk->size > size && !chunk->allocated)
		{
			result = chunk;
		}
	}

	if(result == 0)
	{
		return 0;
	}

	if(result->size >= size + sizeof(MemoryChunk) + 1)
	{
		MemoryChunk *temp = (MemoryChunk*)((size_t)result + sizeof(MemoryChunk) + size);

		temp->allocated = false;
		temp->size = result->size - size - sizeof(MemoryChunk);
		temp->prev = result;
		temp->next = result->next;

		if(temp->next != 0)
		{
			temp->next->prev = temp;
		}

		result->size = size;
		result->next = temp;
	}

	result->allocated = true;

	return (void*)(((size_t)result) + sizeof(MemoryChunk));
}

void myxp::MemoryManager::free(void *ptr)
{
	MemoryChunk *chunk = (MemoryChunk*)((size_t)ptr - sizeof(MemoryChunk));

	chunk->allocated = false;

	if(chunk->prev != 0 && !chunk->prev->allocated)
	{
		chunk->prev->next = chunk->next;
		chunk->prev->size += chunk->size + sizeof(MemoryChunk);

		if(chunk->next != 0)
		{
			chunk->next->prev = chunk->prev;
		}

		chunk = chunk->prev;
	}

	if(chunk->next != 0 && !chunk->next->allocated)
	{
		chunk->size += chunk->next->size + sizeof(MemoryChunk);
		chunk->next = chunk->next->next;

		if(chunk->next != 0)
		{
			chunk->next->prev = chunk;
		}
	}
}
*/