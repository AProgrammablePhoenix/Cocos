BITS 64

BITS 64
global IDT
global kernel_idt_load

%define IDT_MAX_SIZE 0x1000

section .data
align 16
IDT:
    ;; System Interrupts/Exceptions/Faults

    dq 0, 0   ;   Divide Error
    dq 0, 0   ;   Debug
    dq 0, 0   ;   NMI
    dq 0, 0   ;   Breakpoint
    dq 0, 0   ;   Overflow
    dq 0, 0   ;   BOUND Range Exceeded
    dq 0, 0   ;   Invalid Opcode
    dq 0, 0   ;   Device Not Available
    dq 0, 0   ;   Double Fault
    dq 0, 0   ;   Coprocessor Segment Overrun
    dq 0, 0   ;   Invalid TSS
    dq 0, 0   ;   Segment Not Present
    dq 0, 0   ;   Stack Fault
    dq 0, 0   ;   General Protection
    dq 0, 0   ;   Page Fault
    dq 0, 0   ;   --- Reserved ---
    dq 0, 0   ;   Floating-Point Error
    dq 0, 0   ;   Alignment Check
    dq 0, 0   ;   Machine Check
    dq 0, 0   ;   SIMD floating-point
    dq 0, 0   ;   Virtualization Exception
    dq 0, 0   ;   Control Protection
    dq 0, 0   ;   --- Reserved ---
    dq 0, 0   ;   --- Reserved ---
    dq 0, 0   ;   --- Reserved ---
    dq 0, 0   ;   --- Reserved ---
    dq 0, 0   ;   --- Reserved ---
    dq 0, 0   ;   --- Reserved ---
    dq 0, 0   ;   Undocumented
    dq 0, 0   ;   Undocumented
    dq 0, 0   ;   Undocumented
    dq 0, 0   ;   --- Reserved ---
IDT_CORE_EXCPTIONS_END:

    ;; Remaining 224 descriptors
    times (IDT_MAX_SIZE - (IDT_CORE_EXCPTIONS_END - IDT)) db 0

IDT_END:
    align 8
    dq 0

IDTP:
    dw IDT_END - IDT - 1
    dq IDT

section .text
kernel_idt_load:
    mov rax, IDTP
    lidt [rax]
    ret
