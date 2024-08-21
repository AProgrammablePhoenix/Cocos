BITS 64

extern kpanic
extern core_dump_setup

global int_debug_trap

section .rodata
info_msg:
    db "A DEBUG TRAP WAS TRIGGERED WITHIN THE KERNEL", 0

section .text
int_debug_trap:
    call core_dump_setup
    lea rcx, [rel info_msg]
    xor edx, edx
    call kpanic
    iretq
    