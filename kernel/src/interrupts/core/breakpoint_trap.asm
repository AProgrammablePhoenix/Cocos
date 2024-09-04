BITS 64

extern kpanic
extern main_core_dump

global int_breakpoint_trap

section .rodata
info_msg:
    db "A BREAKPOINT WAS TRIGGERED WITHIN THE KERNEL", 0

section .text
int_breakpoint_trap:
    call main_core_dump
    lea rcx, [rel info_msg]
    xor edx, edx
    call kpanic
    iretq
    