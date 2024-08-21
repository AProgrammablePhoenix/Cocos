BITS 64

extern kpanic
extern core_dump_setup

global int_simd_error

section .rodata
error_msg:
    db "A SIMD ERROR OCCURED", 0

section .text
int_simd_error:
    call core_dump_setup
    lea rcx, [rel error_msg]
    xor edx, edx
    call kpanic
    iretq
    