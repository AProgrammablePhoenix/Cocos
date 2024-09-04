BITS 64

extern kpanic
extern main_core_dump

global int_divide_error

section .rodata
error_msg:
    db "A DIVISION ERROR OCCURED WITHIN THE KERNEL", 0

section .text
int_divide_error:
    call main_core_dump
    lea rcx, [rel error_msg]
    xor edx, edx
    call kpanic
    iretq
    