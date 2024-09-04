BITS 64

extern kpanic
extern main_core_dump

global int_device_error

section .rodata
error_msg:
        db "AN FPU DEVICE ERROR OCCURED WITHIN THE KERNEL", 0

section .text
int_device_error:
    call main_core_dump
    lea rcx, [rel error_msg]
    xor edx, edx
    call kpanic
    iretq
    