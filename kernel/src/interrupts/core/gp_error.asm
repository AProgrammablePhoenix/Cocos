BITS 64

extern kpanic
extern main_core_dump

global int_gp_error

section .rodata
error_msg:
    db "GENERAL PROTECTION FAULT - INVALID OPERATION DETECTED", 0

section .data
temp:
    dq 0

section .text
int_gp_error:
    mov [rel temp], rax
    pop rax
    xchg [rel temp], rax
    call main_core_dump
    lea rcx, [rel error_msg]
    mov rdx, [rel temp]
    call kpanic
    iretq
