# Setup GRUB values so we can boot from GRUB.
.set ALIGN, 1<<0
.set MEM_INFO, 1<<1
# .set MEM_GRAPHICS, 1<<2
.set FLAGS, ALIGN | MEM_INFO # | MEM_GRAPHICS
.set MAGIC, 0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot
.align 4

.long MAGIC
.long FLAGS
.long CHECKSUM
.long 0x00000000 /* header_addr */
.long 0x00000000 /* load_addr */
.long 0x00000000 /* load_end_addr */
.long 0x00000000 /* bss_end_addr */
.long 0x00000000 /* entry_addr */

# Request graphics mode
# .long 0x00000000
# .long 0
# .long 0
# .long 32

.section .text
# Our Kernel Start Point
.extern kernelMain
# Kernel constructor pointer for start and end
.extern callConstructors
.global loader

loader:
    mov $kernel_stack, %esp

    and $-16, %esp  # Align the stack to 16-byte
    
    call callConstructors

    pushl %esp
    pushl %eax  # Multiboot Header Magic
    pushl %ebx  # Multiboot Header Pointer
    
    cli

    call kernelMain

    cli

_stop:
    cli
    hlt
    jmp _stop

/* .stack resides in .bss */
.section .bss
.align 16
stack_bottom:
# .skip 16384 /* 16KiB */
.skip 32768 /* 32KiB */
kernel_stack:

# .size loader, . - loader