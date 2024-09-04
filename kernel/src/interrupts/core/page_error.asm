BITS 64

extern page_fault_handler
extern main_core_dump
extern main_core_reload

global int_page_error

section .data
temp:
    dq 0

section .text
int_page_error:
    mov [rel temp], rax
    pop rax
    xchg [rel temp], rax
    call main_core_dump
    mov rcx, [rel temp]
    call page_fault_handler
    call main_core_reload
    iretq
    