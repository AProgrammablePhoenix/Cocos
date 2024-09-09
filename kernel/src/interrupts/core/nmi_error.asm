BITS 64

extern kpanic
extern main_core_dump

global int_nmi_error

section .rodata
error_msg:
    db "A SEVERE SYSTEM FAILURE OCCURED", 0

section .text
int_nmi_error:
    call main_core_dump
    lea rcx, [rel error_msg]
    xor edx, edx
    sub rsp, 8
    call kpanic
    add rsp, 8
    iretq
