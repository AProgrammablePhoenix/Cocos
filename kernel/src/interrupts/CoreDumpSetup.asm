BITS 64

; storage size: 32 KB
%define MAIN_STORAGE            0xFFFFFF8001000000         
%define SECONDARY_STORAGE       0xFFFFFF8001008000

%define USE_MAIN_STORAGE        0xF1827
%define USE_SECONDARY_STORAGE   0x26304

%define EFER_MSR_NUMBER         0xC0000080

global main_core_dump
global secondary_core_dump
global request_dump_type

section .data
temp:
    dq 0
mode:
    dq 0

section .text
core_dump_setup:
    cmp QWORD [rel mode], USE_SECONDARY_STORAGE
    jne .L0
    mov [rel temp], rax
    mov rax, SECONDARY_STORAGE
    xchg [rel temp], rax
    jmp .L1
.L0:
    mov [rel temp], rax
    mov rax, MAIN_STORAGE
    xchg [rel temp], rax
.L1:
    xchg [rel temp], rbx
    mov [rbx], rax
    mov rax, rbx
    mov rbx, [rel temp]
    mov [rax + 0x08], rbx
    mov [rax + 0x10], rcx
    mov [rax + 0x18], rdx
    mov [rax + 0x20], rsi
    mov [rax + 0x28], rdi
    mov [rax + 0x30], rbp
    mov rcx, [rsp + 0x20]   ; RSP
    mov [rax + 0x38], rcx
    mov [rax + 0x40], r8
    mov [rax + 0x48], r9
    mov [rax + 0x50], r10
    mov [rax + 0x58], r11
    mov [rax + 0x60], r12
    mov [rax + 0x68], r13
    mov [rax + 0x70], r14
    mov [rax + 0x78], r15
    mov rcx, [rsp + 0x8]    ; RIP
    mov [rax + 0x80], rcx
    mov rcx, [rsp + 0x18]   ; RFLAGS
    mov [rax + 0x88], rcx
    mov [rax + 0x90], es
    mov rcx, [rsp + 0x10]   ; CS
    mov [rax + 0x92], cx
    mov rcx, [rsp + 0x28]   ; SS
    mov [rax + 0x94], cx
    mov [rax + 0x96], ds
    mov [rax + 0x98], fs
    mov [rax + 0x9A], gs
    sldt [rax + 0x9C]
    str [rax + 0x9E]
    sgdt [rax + 0xA0]
    sidt [rax + 0xAA]
    mov dx, [rax + 0xAA]
    mov r8, [rax + 0xA2]
    mov r9, [rax + 0xAC]
    mov [rax + 0xA2], dx
    mov [rax + 0xA8], r8
    mov [rax + 0xB0], r9
    mov rcx, cr0
    mov rdx, cr2
    mov r8, cr3
    mov r9, cr4
    mov r10, cr8
    mov [rax + 0xB8], rcx
    mov [rax + 0xC0], rdx
    mov [rax + 0xC8], r8
    mov [rax + 0xD0], r9
    mov [rax + 0xD8], r10
    mov r8, rax
    mov ecx, EFER_MSR_NUMBER
    rdmsr
    mov [r8 + 0xE0], eax
    mov [r8 + 0xE4], edx
    mov rax, r8
    mov rcx, dr0
    mov rdx, dr1
    mov r8, dr2
    mov r9, dr3
    mov r10, dr6
    mov r11, dr7
    mov [rax + 0xE8], rcx
    mov [rax + 0xF0], rdx
    mov [rax + 0xF8], r8
    mov [rax + 0x100], r9
    mov [rax + 0x108], r10
    mov [rax + 0x110], r11

    ret

main_core_dump:
    mov QWORD [rel mode], USE_MAIN_STORAGE
    jmp core_dump_setup

secondary_core_dump:
    mov QWORD [rel mode], USE_SECONDARY_STORAGE
    jmp core_dump_setup

request_dump_type:
    mov rax, [rel mode]
    ret
