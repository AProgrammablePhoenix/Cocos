BITS 64

extern kpanic
extern main_core_dump

global int_security_error

section .rodata
error_msg:
    db "A SECURITY ERROR OCCURED", 0

section .data
temp:
    dq 0

section .text
int_security_error:
    mov [rel temp], rax
    pop rax
    xchg [rel temp], rax
    call main_core_dump
    lea rcx, [rel error_msg]
    mov rdx, [rel temp]
    sub rsp, 8
    call kpanic
    add rsp, 8
    iretq
    