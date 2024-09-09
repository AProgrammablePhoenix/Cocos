BITS 64

extern main_core_dump
extern main_core_reload
extern ps2_keyboard_event_handler

global PS2_IRQ1_handler

section .text
PS2_IRQ1_handler:
    call main_core_dump
    call ps2_keyboard_event_handler
    mov al, 0x20
    out 0x20, al
    call main_core_reload
    iretq
   