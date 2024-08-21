BITS 64

extern kpanic
extern core_dump_setup

global int_bound_error

section .rodata
error_msg:
    db "AN INDEX OUT OF BOUNDS EXCEPTION OCCURED WITHIN THE KERNEL", 0

section .text
int_bound_error:
    call core_dump_setup
    lea rcx, [rel error_msg]
    xor edx, edx
    call kpanic
    iretq
