BITS 64

extern main_core_dump
extern main_core_reload
extern system_timer_event_handler

global PIT_IRQ0_handler

section .data
execution_context:
    context_cr3: dq 0
    context_rsp: dq 0

section .text
PIT_IRQ0_handler:
    call main_core_dump
    lea rcx, [rel execution_context]
    sub rsp, 8
    call system_timer_event_handler
    add rsp, 8
    cmp QWORD [rel context_cr3], 0
    jz .L0
    mov rax, [rel context_cr3]
    mov rcx, [rel context_rsp]
    mov cr3, rax
    mov rsp, rcx
.L0:
    ; Sends the EOI to the PIC
    mov al, 0x20
    out 0x20, al
    call main_core_reload
    iretq
