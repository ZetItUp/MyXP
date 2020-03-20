#ifndef __MYXP__MEMORY__MANAGEMENT_H
#define __MYXP__MEMORY__MANAGEMENT_H

/*
 * This file contains definitions for the x86 memory management unit (MMU),
 * including paging- and segmentation-related data structures and constants,
 * the %cr0, %cr4, and %eflags registers, and traps.
 */

/*
 *
 *	Part 1.  Paging data structures and constants.
 *
 */

// A linear address 'la' has a three-part structure as follows:
//
// +--------10------+-------10-------+---------12----------+
// | Page Directory |   Page Table   | Offset within Page  |
// |      Index     |      Index     |                     |
// +----------------+----------------+---------------------+
//  \--- PDX(la) --/ \--- PTX(la) --/ \---- PGOFF(la) ----/
//  \---------- PGNUM(la) ----------/
//
// The PDX, PTX, PGOFF, and PGNUM macros decompose linear addresses as shown.
// To construct a linear address la from PDX(la), PTX(la), and PGOFF(la),
// use PGADDR(PDX(la), PTX(la), PGOFF(la)).

#include <lib/types.h>

namespace myxp
{
    namespace paging
    {
        typedef myxp::uint32_t pte_t;
        typedef myxp::uint32_t pde_t;
    };
};

// Page Offset of Page Table Index in a Linear Address
#define PTXSHIFT	        12
// Page Offset of Page Directory Index in a Linear Address
#define PDXSHIFT	        22

// Page Number Field of Address
#define PAGE_NUMBER(la)     (((myxp::uintptr_t) (la) >> PTXSHIFT))
// Page Directory Index
#define PAGE_DIR_INDEX(la)  (((myxp::uintptr_t) (la) >> PDXSHIFT) & 0x3FF)
// Page Table Index
#define PAGE_TBL_INDEX(la)  (((myxp::uintptr_t) (la) >> PTXSHIFT) & 0x3FF)
// Page Offset
#define PAGE_OFFSET(la)     (((myxp::uintptr_t) (la)) & 0xFFF)
// Page Address
#define PAGE_ADDRESS(dir, tbl, off)   ((void*) ((dir) << PFXSHIFT | (tbl) << PTXSHIFT | (off)))

// Page Directory Entries Per Page Directory
#define PAGE_DIR_ENTRIES    1024
// Page Table Entries Pet Page Table
#define PAGE_TABLE_ENTRIES  1024
// Page Size
//#define PAGE_SIZE           4096
// Page Shift
#define PAGE_SHIFT          12  // log2(PAGE_SIZE)

#define PAGE_TABLE_SIZE     (PAGE_SIZE * PAGE_TABLE_ENTRIES)    // Bytes Mapped by a Page Directory Entry
#define PAGE_TABLE_SHIFT    22  // log2(PAGE_TABLE_SIZE)

// Page Table/Directory Entry Flags
#define PAGE_TABLE_P        0x001   // Present
#define PAGE_TABLE_W        0x002   // Writeable
#define PAGE_TABLE_U        0x004   // User
#define PAGE_TABLE_PWT      0x008   // Write-Through
#define PAGE_TABLE_PCD      0x010   // Cache-Disable
#define PAGE_TABLE_A        0x020   // Accessed
#define PAGE_TABLE_D        0x040   // Dirty
#define PAGE_TABLE_PS       0x080   // Page Size
#define PAGE_TABLE_G        0x100   // Global

// The PAGE_TABLE_AVAIL bits aren't used by the kernel or interpreted by the
// hardware, so user processes are allowed to set them arbitrarily.
#define PAGE_TABLE_AVAIL	0xE00	// Available for software use

// Flags in PTE_SYSCALL may be used in system calls.  (Others may not.)
#define PAGE_TABLE_SYSCALL	(PAGE_TABLE_AVAIL | PAGE_TABLE_P | PAGE_TABLE_W | PAGE_TABLE_U)

// Address in page table or page directory entry
#define PAGE_TABLE_ADDRESS(pt)  ((myxp::uintptr_t) (pt) & ~0xFFF)

// Control Register flags
#define CR0_PE		0x00000001	// Protection Enable
#define CR0_MP		0x00000002	// Monitor coProcessor
#define CR0_EM		0x00000004	// Emulation
#define CR0_TS		0x00000008	// Task Switched
#define CR0_ET		0x00000010	// Extension Type
#define CR0_NE		0x00000020	// Numeric Errror
#define CR0_WP		0x00010000	// Write Protect
#define CR0_AM		0x00040000	// Alignment Mask
#define CR0_NW		0x20000000	// Not Writethrough
#define CR0_CD		0x40000000	// Cache Disable
#define CR0_PG		0x80000000	// Paging

#define CR4_PCE		0x00000100	// Performance counter enable
#define CR4_MCE		0x00000040	// Machine Check Enable
#define CR4_PSE		0x00000010	// Page Size Extensions
#define CR4_DE		0x00000008	// Debugging Extensions
#define CR4_TSD		0x00000004	// Time Stamp Disable
#define CR4_PVI		0x00000002	// Protected-Mode Virtual Interrupts
#define CR4_VME		0x00000001	// V86 Mode Extensions

// Eflags register
#define FL_CF		0x00000001	// Carry Flag
#define FL_PF		0x00000004	// Parity Flag
#define FL_AF		0x00000010	// Auxiliary carry Flag
#define FL_ZF		0x00000040	// Zero Flag
#define FL_SF		0x00000080	// Sign Flag
#define FL_TF		0x00000100	// Trap Flag
#define FL_IF		0x00000200	// Interrupt Flag
#define FL_DF		0x00000400	// Direction Flag
#define FL_OF		0x00000800	// Overflow Flag
#define FL_IOPL_MASK	0x00003000	// I/O Privilege Level bitmask
#define FL_IOPL_0	0x00000000	//   IOPL == 0
#define FL_IOPL_1	0x00001000	//   IOPL == 1
#define FL_IOPL_2	0x00002000	//   IOPL == 2
#define FL_IOPL_3	0x00003000	//   IOPL == 3
#define FL_NT		0x00004000	// Nested Task
#define FL_RF		0x00010000	// Resume Flag
#define FL_VM		0x00020000	// Virtual 8086 mode
#define FL_AC		0x00040000	// Alignment Check
#define FL_VIF		0x00080000	// Virtual Interrupt Flag
#define FL_VIP		0x00100000	// Virtual Interrupt Pending
#define FL_ID		0x00200000	// ID flag

// Page fault error codes
#define FEC_PR		0x1	// Page fault caused by protection violation
#define FEC_WR		0x2	// Page fault caused by a write
#define FEC_U		0x4	// Page fault occured while in user mode

#endif