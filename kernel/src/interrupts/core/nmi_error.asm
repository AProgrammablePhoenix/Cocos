BITS 64

extern kpanic
extern core_dump_setup

global int_nmi_error

section .rodata
error_msg:
    db "A SEVERE SYSTEM FAILURE OCCURED", 0

section .text
int_nmi_error:
    call core_dump_setup
    lea rcx, [rel error_msg]
    xor edx, edx
    call kpanic
    iretq
