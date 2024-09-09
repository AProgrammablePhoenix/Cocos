BITS 64

extern kpanic
extern main_core_dump

global int_hypervirt_error

section .rodata
error_msg:
    db "AN HYPERVISOR INJECTION WAS ATTEMPTED", 0

section .text
int_hypervirt_error:
    call main_core_dump
    lea rcx, [rel error_msg]
    xor edx, edx
    sub rsp, 8
    call kpanic
    add rsp, 8
    iretq
    