BITS 64

;; Exposes funcions to save/restore General Purpose Registers on interrupts handler enter/exit

global isr_context_save
global isr_context_restore

section .data
registers_store:
    dq 0    ; RAX
    dq 0    ; RCX
    dq 0    ; RDX
    dq 0    ; R8
    dq 0    ; R9
    dq 0    ; R10
    dq 0    ; R11
    
section .text
isr_context_save:
    mov [rel registers_store + 0x00], rax
    mov [rel registers_store + 0x08], rcx
    mov [rel registers_store + 0x10], rdx
    mov [rel registers_store + 0x18], r8
    mov [rel registers_store + 0x20], r9
    mov [rel registers_store + 0x28], r10
    mov [rel registers_store + 0x30], r11
    ret

isr_context_restore:
    mov rax, [rel registers_store + 0x00]
    mov rcx, [rel registers_store + 0x08]
    mov rdx, [rel registers_store + 0x10]
    mov r8, [rel registers_store + 0x18]
    mov r9, [rel registers_store + 0x20]
    mov r10, [rel registers_store + 0x28]
    mov r11, [rel registers_store + 0x30]
    ret
