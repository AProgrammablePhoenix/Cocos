BITS 64

extern kpanic
extern main_core_dump

global int_simd_error

section .rodata
error_msg:
    db "A SIMD ERROR OCCURED", 0

section .text
int_simd_error:
    call main_core_dump
    lea rcx, [rel error_msg]
    xor edx, edx
    call kpanic
    iretq
    