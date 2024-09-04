BITS 64

extern kpanic
extern main_core_dump

global int_machine_error

section .rodata
error_msg:
    db "INTERNAL HARDWARE ERRORS DETECTED", 0

section .text
int_machine_error:
    call main_core_dump
    lea rcx, [rel error_msg]
    xor edx, edx
    call kpanic
    iretq
    