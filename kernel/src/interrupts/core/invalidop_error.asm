BITS 64

extern kpanic
extern core_dump_setup

global int_invalidop_error

section .rodata
error_msg:
    db "AN INVALID OPCODE EXCEPTION OCCURED WITHIN THE KERNEL", 0

section .text
int_invalidop_error:
    call core_dump_setup
    lea rcx, [rel error_msg]
    xor edx, edx
    call kpanic
    iretq
    