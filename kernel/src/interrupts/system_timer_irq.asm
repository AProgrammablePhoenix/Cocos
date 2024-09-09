BITS 64

extern isr_context_save
extern isr_context_restore
extern system_timer_event_handler

global PIT_IRQ0_handler

section .data
execution_context:
    context_cr3: dq 0
    context_rsp: dq 0

section .text
PIT_IRQ0_handler:
    call isr_context_save
    lea rcx, [rel execution_context]
    call system_timer_event_handler
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
    call isr_context_restore
    iretq
