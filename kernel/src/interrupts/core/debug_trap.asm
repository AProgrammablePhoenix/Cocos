BITS 64

extern kpanic
extern main_core_dump

global int_debug_trap

section .rodata
info_msg:
    db "A DEBUG TRAP WAS TRIGGERED WITHIN THE KERNEL", 0

section .text
int_debug_trap:
    call main_core_dump
    lea rcx, [rel info_msg]
    xor edx, edx
    sub rsp, 8
    call kpanic
    add rsp, 8
    iretq
    