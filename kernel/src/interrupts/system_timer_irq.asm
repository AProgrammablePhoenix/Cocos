BITS 64

extern isr_context_save
extern isr_context_restore
extern system_timer_event_handler

global PIT_IRQ0_handler

section .text
PIT_IRQ0_handler:
    call isr_context_save
    call system_timer_event_handler
    ; Sends the EOI to the PIC
    mov al, 0x20
    out 0x20, al
    call isr_context_restore
    iretq
