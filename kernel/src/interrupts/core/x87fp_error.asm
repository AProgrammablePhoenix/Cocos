BITS 64

extern kpanic
extern main_core_dump

global int_x87fp_error

section .rodata
error_msg:
    db "A FLOATING-POINT EXCEPTION OCCURED", 0

section .text
int_x87fp_error:
    call main_core_dump
    lea rcx, [rel error_msg]
    xor edx, edx
    sub rsp, 8
    call kpanic
    add rsp, 8
    iretq
    