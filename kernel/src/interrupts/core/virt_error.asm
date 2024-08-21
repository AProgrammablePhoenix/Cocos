BITS 64

extern kpanic
extern core_dump_setup

global int_virt_error

section .rodata
error_msg:
    db "A VIRTUALIZATION ERROR OCCURED", 0

section .text
int_virt_error:
    call core_dump_setup
    lea rcx, [rel error_msg]
    xor edx, edx
    call kpanic
    iretq
    