BITS 64

;; Legacy 8254 PIT initialization


;; PIT 8254 IO Ports
%define PIT_CHANNEL_0_DATA  0x40
%define PIT_CHANNEL_1_DATA  0x41
%define PIT_CHANNEL_2_DATA  0x42
%define PIT_MODE_REGISTER   0x43

;; Current configuration settings
%define PIT_RELOAD_VALUE    119         ; frequency divider (divides 1193182)
%define IRQ0_FREQUENCY      10027       ; number of times IRQ0 is fired per second
%define IRQ0_US             100         ; number of microseconds between each IRQ

%define PIT_COMMAND_BYTE    00110100b   ; channel 0, low byte + high byte, rate generator

;; imports
extern mask_irq

;; exports
global SYSTEM_TIMER_US
global IRQ0_frequency
global IRQ0_us
global PIT_reload_value
global initialize_pit
global disable_pit

;; variables used by the kernel
section .data
SYSTEM_TIMER_US:    dq 0                    ; number of microseconds elapsed since the PIT was initialized
IRQ0_frequency:     dd IRQ0_FREQUENCY       ; see above
IRQ0_us:            dw IRQ0_US              ; see above
PIT_reload_value:   dw PIT_RELOAD_VALUE     ; see above


;; 8254 PIT Command Byte fields:
;; bit 6 - 7:
;;      0: channel 0 (the channel selected here)
;;      1: channel 1
;;      2: channel 2
;;      3: read-back
;; bit 4 - 5:
;;      0: latch count value
;;      1: low byte only
;;      2: high byte only
;;      3: low byte followed by high byte (the access mode chosen here)
;; bit 1 - 3:
;;      0: interrupt on terminal count
;;      1: hardware re-triggerable one-shot
;;      2: rate generator (the operating mode selected here)
;;      3: square wave generator
;;      4: software triggered strobe
;;      5: hardware triggered strobe
;;      6: rate generator, same as 2
;;      7: square wave generator, same as 3
;; bit 0:
;;      0: 16-bit binary (the encoding chosen here)
;;      1: four-digit BCD

section .text
;; Initializes the PIT to a frequency of around 10KHz, uses a frequency divider of 119 as 1193182 / 119 is very close to 10KHz
initialize_pit:
    cli
    xor eax, eax
    mov [rel SYSTEM_TIMER_US], rax  ; resets system timer
    mov al, PIT_COMMAND_BYTE
    out PIT_MODE_REGISTER, al       ; sets the PIT operating mode
    mov ax, PIT_RELOAD_VALUE
    out PIT_CHANNEL_0_DATA, al      ; reload value low byte
    mov al, ah
    out PIT_CHANNEL_0_DATA, al      ; reload value high byte
    ret

;; Disables the PIT by masking its IRQ line in the PIC (IRQ0)
disable_pit:
    xor ecx, ecx
    jmp mask_irq    ; already returns
