BITS 64

%define EFER_MSR_NUMBER 0xC0000080

extern core_dump_registers

global core_dump_setup

section .text
core_dump_setup:
    mov [rel core_dump_registers + 0x000], rax
    mov [rel core_dump_registers + 0x008], rbx
    mov [rel core_dump_registers + 0x010], rcx
    mov [rel core_dump_registers + 0x018], rdx
    mov [rel core_dump_registers + 0x020], rsi
    mov [rel core_dump_registers + 0x028], rdi
    mov [rel core_dump_registers + 0x030], rbp
    mov rax, [rsp + 0x20]
    mov [rel core_dump_registers + 0x038], rax
    mov [rel core_dump_registers + 0x040], r8
    mov [rel core_dump_registers + 0x048], r9
    mov [rel core_dump_registers + 0x050], r10
    mov [rel core_dump_registers + 0x058], r11
    mov [rel core_dump_registers + 0x060], r12
    mov [rel core_dump_registers + 0x068], r13
    mov [rel core_dump_registers + 0x070], r14
    mov [rel core_dump_registers + 0x078], r15
    mov rax, [rsp + 0x8]
    mov [rel core_dump_registers + 0x080], rax
    mov rax, [rsp + 0x18]
    mov [rel core_dump_registers + 0x088], rax
    mov ax, es
    mov [rel core_dump_registers + 0x090], ax
    mov rax, [rsp + 0x10]
    mov [rel core_dump_registers + 0x092], ax
    mov rax, [rsp + 0x28]
    mov [rel core_dump_registers + 0x094], ax
    mov ax, ds
    mov [rel core_dump_registers + 0x096], ax
    mov ax, fs
    mov [rel core_dump_registers + 0x098], ax
    mov ax, gs
    mov [rel core_dump_registers + 0x09A], ax
    sldt [rel core_dump_registers + 0x0A0]
    str  [rel core_dump_registers + 0x0A2]
    sgdt [rel core_dump_registers + 0x0B0]
    sidt [rel core_dump_registers + 0x0C0]
    mov rax, cr0
    mov [rel core_dump_registers + 0x0D0], rax
    mov rax, cr2
    mov [rel core_dump_registers + 0x0D8], rax
    mov rax, cr3
    mov [rel core_dump_registers + 0x0E0], rax
    mov rax, cr4
    mov [rel core_dump_registers + 0x0E8], rax
    mov rax, cr8
    mov [rel core_dump_registers + 0x0F0], rax
    mov ecx, EFER_MSR_NUMBER
    rdmsr
    mov [rel core_dump_registers + 0x0F8], eax
    mov [rel core_dump_registers + 0x0FC], edx
    mov rax, dr0
    mov [rel core_dump_registers + 0x100], rax
    mov rax, dr1
    mov [rel core_dump_registers + 0x108], rax
    mov rax, dr2
    mov [rel core_dump_registers + 0x110], rax
    mov rax, dr3
    mov [rel core_dump_registers + 0x118], rax
    mov rax, dr6
    mov [rel core_dump_registers + 0x120], rax
    mov rax, dr7
    mov [rel core_dump_registers + 0x128], rax
    ret
