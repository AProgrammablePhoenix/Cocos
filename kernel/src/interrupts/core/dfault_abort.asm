BITS 64

extern kpanic
extern core_dump_setup

global int_doublefault_error

section .rodata
error_msg:
    db "A DOUBLE FAULT OCCURED", 0

section .data
temp:
    dq 0

section .text
int_doublefault_error:
    mov [rel temp], rax
    pop rax
    xchg [rel temp], rax
    call core_dump_setup
    lea rcx, [rel error_msg]
    mov rdx, [rel temp]
    call kpanic
    iretq
    