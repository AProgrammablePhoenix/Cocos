OUTPUT_ARCH(i386:x86-64)

ENTRY(kmain)

SECTIONS {
    . = 0x00000000C0000000;
    .userembedded ALIGN(0x1000) : {
        *(.userembedded*)
    }
    . = 0xffff800000000000;
    .text ALIGN(0x1000) : {
        *(.text*)
    }
    .data ALIGN(0x1000) : {
        *(.data*)
    }
    .rodata ALIGN(0x1000) : {
        *(.rodata*)
    }
    .rdata ALIGN(0x1000) : {
        *(.rdata*)
    }
    .bss ALIGN(0x1000) : {
        *(.bss*)
        *(COMMON)
    }
    .init_array : {
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