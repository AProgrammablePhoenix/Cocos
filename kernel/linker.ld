OUTPUT_ARCH(i386:x86-64)

ENTRY(kmain)

SECTIONS {
    . = 0x0000008000000000;
    .userembedded ALIGN(CONSTANT(MAXPAGESIZE)) : {
        userembedded_start = .;
        *(.userembedded*)
        . = ALIGN(CONSTANT(MAXPAGESIZE));
        userembedded_end = .;
    }
    . = 0xffff800000000000;
    .text ALIGN(CONSTANT(MAXPAGESIZE)) : {
        *(.text*)
    }
    .data ALIGN(CONSTANT(MAXPAGESIZE)) : {
        *(.data*)
    }
    .rodata ALIGN(CONSTANT(MAXPAGESIZE)) : {
        *(.rodata*)
    }
    .rdata ALIGN(CONSTANT(MAXPAGESIZE)) : {
        *(.rdata*)
    }
    .bss ALIGN(CONSTANT(MAXPAGESIZE)) : {
        *(.bss*)
        *(COMMON)
    }
    .init_array ALIGN(CONSTANT(MAXPAGESIZE)) : {
        kernel_init_array_start = .;
        *(.init_array*)
        kernel_init_array_end = .;
    }
    /DISCARD/ : {
        *(.reloc*)
        *(.idata*)
        *(.pdata*)
        *(.xdata*)
        *(.note*)
        *(.comment*)
        *(.eh_frame*)
    }
}