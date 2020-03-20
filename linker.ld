ENTRY(loader)
OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386:i386)

SECTIONS
{
    . = 1M;
    phys = .;

    .text BLOCK(4K) : ALIGN(4K)
    {
        code = .;
        *(.multiboot)
        *(.text*)
    }

    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    }

    .data BLOCK(4K) : ALIGN(4K)
    {
        data = .;
        start_ctors = .;
        KEEP(*( .init_array ));
        KEEP(*(SORT_BY_INIT_PRIORITY( .init_array.* )));
        end_ctors = .;

        *(.data)
        *(.symbols)
    }

    .bss BLOCK(4K) : ALIGN(4K)
    {
        bss = .;
        *(COMMON)
        *(.bss)
        *(.stack)
    }

    end = .;

    /DISCARD/ : { *(.fini_array*) *(.comment) }
}