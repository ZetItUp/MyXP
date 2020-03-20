/*
 * My Experience Operating System
 * Copyright (C) 2019, 2020, Andreas Lindström
 */
#include <lib/mem.h>
#include <memorym.h>

/*
 * Allocate memory in the current virtual address space.
 * Returns a pointer to the newly-allocated memory, -1 on error.
 */
 void *malloc(myxp::uintptr_t n)
 {
    myxp::uint16_t data_sel;
	myxp::int32_t a, b, c, i, j, k, l, t, f_start, f_size;
	myxp::uintptr_t pgdir_ent, pgtab_ent;

	asm volatile("pushl %%eax\n\t"
                 "movw %%ds, %%ax\n\t"
	             "movw %%ax, %0\n\t" 
                 "popl %%eax": "=m" (data_sel));

    // If n is not 4kB aligned, align it
    if(n & 0xFFF)
    {
        n &= 0xFFFFF000;
        n += 0x1000;
    }

    // Get n in pages
    n >>= PAGE_SHIFT;

    if(n == 0)
    {
        return (void*)-1;
    }
 }