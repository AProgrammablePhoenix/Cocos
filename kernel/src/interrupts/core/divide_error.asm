BITS 64

extern kpanic
extern core_dump_setup

global int_divide_error

section .rodata
error_msg:
    db "A DIVISION ERROR OCCURED WITHIN THE KERNEL", 0

section .text
int_divide_error:
    call core_dump_setup
    lea rcx, [rel error_msg]
    xor edx, edx
    call kpanic
    iretq
    