BITS 64

extern kpanic
extern core_dump_setup

global int_machine_error

section .rodata
error_msg:
    db "INTERNAL HARDWARE ERRORS DETECTED", 0

section .text
int_machine_error:
    call core_dump_setup
    lea rcx, [rel error_msg]
    xor edx, edx
    call kpanic
    iretq
    