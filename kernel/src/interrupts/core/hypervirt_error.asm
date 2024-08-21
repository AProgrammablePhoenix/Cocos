BITS 64

extern kpanic
extern core_dump_setup

global int_hypervirt_error

section .rodata
error_msg:
    db "AN HYPERVISOR INJECTION WAS ATTEMPTED", 0

section .text
int_hypervirt_error:
    call core_dump_setup
    lea rcx, [rel error_msg]
    xor edx, edx
    call kpanic
    iretq
    