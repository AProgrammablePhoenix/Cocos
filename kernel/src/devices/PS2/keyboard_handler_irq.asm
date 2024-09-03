BITS 64

extern isr_context_save
extern isr_context_restore
extern ps2_keyboard_event_handler

global PS2_IRQ1_handler

section .text
PS2_IRQ1_handler:
    call isr_context_save
    call ps2_keyboard_event_handler
    mov al, 0x20
    out 0x20, al
    call isr_context_restore
    iretq
   