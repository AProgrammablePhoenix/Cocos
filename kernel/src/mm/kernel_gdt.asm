BITS 64
global kernel_gdt_setup

%define CODE_SEGMENT    0x0008
%define DATA_SEGMENT    0x0010
%Define TSS_SEGMENT     0x0028

%define KERNEL_STACK    0xFFFF800021FFFFF8

section .data
    align 8

TSS:
    dq 0                    ; reserved
    dq KERNEL_STACK         ; RSP0
    dq 0                    ; RSP1
    dq 0                    ; RSP2
    dq 0                    ; reserved
    dq 0                    ; reserved
    dq 0                    ; IST1
    dq 0                    ; IST2
    dq 0                    ; IST3
    dq 0                    ; IST4
    dq 0                    ; IST5
    dq 0                    ; IST6
    dq 0                    ; IST7
    dq 0                    ; reserved
    dq 0                    ; reserved
    dw 0                    ; reserved
    dw IOPM - $             ; IOPB
TSS_END:
IOPM:
    db 0xFF
    align 8

GDT:
    dq 0x0000000000000000   ; null
    dq 0x00209A0000000000   ; kernel code
    dq 0x0000920000000000   ; kernel data
    dq 0x0020FA0000000000   ; user code
    dq 0x0000F20000000000   ; user data
TSS_GDT_ENTRY:
    dw TSS_END - TSS - 1    ; limit 0-15
    dw 0                    ; base 0-15
    db 0                    ; base 16-23
    db 0x89                 ; access
    db 0x20                 ; flags + limit 16-19
    db 0                    ; base 24-31
    dd 0                    ; base 32-63
    dq 0                    ; reserved

GDT_END:
    align 8
    dq 0

GDTP:
    dw GDT_END - GDT - 1
    dq GDT

section .text
kernel_gdt_setup:
    lea rax, [rel TSS]
    mov [rel TSS_GDT_ENTRY + 2], ax
    shr rax, 16
    mov [rel TSS_GDT_ENTRY + 4], al
    shr rax, 8
    mov [rel TSS_GDT_ENTRY + 7], al
    shr rax, 8
    mov [rel TSS_GDT_ENTRY + 8], eax

    lgdt [rel GDTP]
    mov ax, TSS_SEGMENT
    ltr ax

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