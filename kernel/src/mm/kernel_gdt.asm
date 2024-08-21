BITS 64
global kernel_gdt_setup

%define CODE_SEGMENT    0x0008
%define DATA_SEGMENT    0x0010

section .data
    align 8
GDT:
    dq 0x0000000000000000   ; null
    dq 0x00209A0000000000   ; kernel code
    dq 0x0000920000000000   ; kernel data
    dq 0x0020FA0000000000   ; user code
    dq 0x0000F20000000000   ; user data

GDT_END:
    align 8
    dq 0

GDTP:
    dw GDT_END - GDT - 1
    dq GDT

section .text
kernel_gdt_setup:
    lgdt [rel GDTP]

    mov ax, DATA_SEGMENT
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    pop rdx                 ; caller return address
    mov rax, CODE_SEGMENT
    push rax
    push rdx

    retfq