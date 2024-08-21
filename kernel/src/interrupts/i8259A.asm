BITS 64

;; Legacy 8259A PIC initialization, and various functions to reprogram it

%define PIC1        0x20
%define PIC2        0xA0
%define PIC1_CMD    PIC1
%define PIC1_DATA   PIC1 + 1
%define PIC2_CMD    PIC2
%define PIC2_DATA   PIC2 + 1

%define ICW1        0x11

%define PIC1_ICW2   0x20        ; IRQ0 mapped at interrupt 0x20
%define PIC2_ICW2   0x28        ; IRQ8 mapped at interrupt 0x28

%define PIC1_ICW3   0000_0100b
%define PIC2_ICW3   0000_0010b

%define ICW4        0x01

global initialize_pic
global disable_pic
global mask_irq
global enable_irq

section .text
initialize_pic:
; ICW1 :
;   - bit 0: ICW4 enabled
;   - bit 1: only one PIC if set, multiple PICs if cleared (and ICW3 enabled)
;   - bit 2: CALL address interval (cleared = 4, set = 8), defaults to 0 on x86
;   - bit 3: Level triggerd if set, edge triggered if cleared
;   - bit 4: PIC is being initialized if set
;   - bit 5: reserved (must be 0) on x86
;   - bit 6: reserved (must be 0) on x86
;   - bit 7: reserved (must be 0) on x86
    mov al, ICW1
    out PIC1_CMD, al
    out PIC2_CMD, al

;   creates a small latency to give time for the PICs to receive the data correctly
    mov al, 0
    out 0x80, al

; ICW2 :
;   Contains the offset at which the IRQs of the corresponding PIC are remapped
    mov al, PIC1_ICW2
    out PIC1_DATA, al
    mov al, PIC2_ICW2
    out PIC2_DATA, al

;   wait
    mov al, 0
    out 0x80, al

; ICW3 :
;   Indicates to each PIC how they are connected to each other
;   - For PIC1, this is the one-hot encoding of the IRQ the slave PIC (PIC2) uses
;   - For PIC2, this is the IRQ number (in normal encoding, not one-hot)
    mov al, PIC1_ICW3
    out PIC1_DATA, al
    mov al, PIC2_ICW3
    out PIC2_DATA, al

;   wait
    mov al, 0
    out 0x80, al

; ICW4 :
;   Final Initialization Control Word
;   - bit 0: 80x86 mode if set, MCS-80/86 mode if cleared
;   - bit 1: Automatic EOI if set, manual EOI if cleared
;   - bit 2: Uses master buffer if set, slave buffer if cleared (has no impact if the PICs are not buffered)
;   - bit 3: Buffered mode if set, unbuffered mode if cleared
;   - bit 4: Special Fully Nested Mode
;   - bit 5: reserved (must be 0)
;   - bit 6: reserved (must be 0)
;   - bit 7: reserved (must be 0)
    mov al, ICW4
    out PIC1_DATA, al
    out PIC2_DATA, al

;   wait
    mov al, 0
    out 0x80, al

; disabling all IRQs except the Timer and Keyboard ones
    mov al, 0xfc
    out PIC1_DATA, al
    mov al, 0xff
    out PIC2_DATA, al

    ret

disable_pic:
; Masks all IRQs on both PICs
    mov al, 0xff
    out PIC1_DATA, al
    out PIC2_DATA, al
; wait
    mov al, 0x80
    out 0x80, al
    ret

mask_irq:
    cmp cl, 8
    jge .L1
.L0:
    mov dx, PIC1_DATA
    jmp .L2
.L1:
    mov dx, PIC2_DATA
    sub cl, 8
.L2:
    mov al, 1
    shl al, cl
    mov cl, al
    in al, dx
    or al, cl
    out dx, al
    ret

enable_irq:
    cmp cl, 8
    jge .L1
.L0:
    mov dx, PIC1_DATA
    jmp .L2
.L1:
    mov dx, PIC2_DATA
    sub cl, 8
.L2:
    mov al, 1
    shl al, cl
    not al
    mov cl, al
    in al, dx
    and al, cl
    out dx, al
    ret
