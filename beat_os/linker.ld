/* linker.ld - Memory layout for beat_os kernel */

ENTRY(_start)

SECTIONS
{
    /* GRUB loads the kernel at 1MB mark (standard Multiboot load address) */
    . = 1M;

    /* Multiboot2 header must be within first 32KB for GRUB to find it */
    .multiboot2 ALIGN(4K) :
    {
        *(.multiboot2)
    }

    /* Read-only code */
    .text ALIGN(4K) :
    {
        *(.text)
    }

    /* Read-only data */
    .rodata ALIGN(4K) :
    {
        *(.rodata)
    }

    /* Read-write data (initialized) */
    .data ALIGN(4K) :
    {
        *(.data)
    }

    /* Read-write data (zero-initialized) */
    .bss ALIGN(4K) :
    {
        *(COMMON)
        *(.bss)
    }

    /* Discard unwanted sections */
    /DISCARD/ :
    {
        *(.comment)
        *(.note*)
        *(.eh_frame*)
    }
}
