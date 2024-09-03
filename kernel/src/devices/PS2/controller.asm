BITS 64

;; Legacy PS/2 i8042 Controller initialization

;; i8042 IO Ports
%define PS2_COMMAND_PORT    0x64    ; when written to
%define PS2_STATUS_PORT     0x64    ; when read from
%define PS2_DATA_PORT       0x60    ; read/write

;; i8042 Status register
;;  - bit 0: output buffer status (set = full, cleared = empty)
;;  - bit 1: input buffer status (set = full, cleared = empty)
;;  - bit 2: system flag (set = POST passed, cleared = POST failed) [POST := Power-On Self-Test]
;;  - bit 3: command/data (set = data written to input buffer is for a PS/2 device, cleared = data written to input buffer is for PS/2 controller)
;;  - bit 4: reserved (chipset specific)
;;  - bit 5: reserved (chipset specific)
;;  - bit 6: time-out error (set = time-out error, cleared = no error)
;;  - bit 7: parity error (set = parity error, cleared = no error)
;;
;; i8042 Controller commands                                      | Response byte meaning
;;  - 0x20      : Read byte 0 from internal RAM                     (controller configuration RAM)
;;  - 0x21-0x3F : Read byte N from internal RAM                     (unspecified)
;;  - 0x60      : Write to byte 0 of internal RAM                   (none)
;;  - 0x61-0x7F : Write to byte N of internal RAM                   (none)
;;  - 0xA7      : Disable second PS/2 port                          (none)
;;  - 0xA8      : Enable second PS/2 port                           (none)
;;  - 0xA9      : Test second PS/2 port                             (0x00 = test passed, 0x01 = clock line stuck low, 0x02 = clock line stuck hight,
;;                                                                      0x03 = data line stuck low, 0x04 = data line stuck high)
;;  - 0xAA      : Test PS/2 controller                              (0x55 = test passed, 0xFC = test failed)
;;  - 0xAB      : Test first PS/2 port                              (0x00 = test passed, 0x01 = clock line stuck low, 0x02 = clock line stuck hight,
;;                                                                      0x03 = data line stuck low, 0x04 = data line stuck high)
;;  - 0xAC      : diagnostic dump (Read all RAM)                    (unspecified)
;;  - 0xAD      : disable first PS/2 port                           (none)
;;  - 0xAE      : enable first PS/2 port                            (none)
;;  - 0xC0      : read controller input port                        (unspecified)
;;  - 0xC1      : copy bits 0-3 of input port to status bits 4-7    (none)
;;  - 0xC2      : copy bits 4-7 of input port to status bits 4-7    (none)
;;  - 0xD0      : read controller output port                       (controller output port)
;;  - 0xD1      : write next byte to controller output port         (none)
;;  - 0xD2      : write next byte to first PS/2 port output buffer  (none)
;;  - 0xD3      : write next byte to second PS/2 port output buffer (none)
;;  - 0xD4      : write next byte to second PS/2 port input buffer  (none)
;;  - 0xF0-0xFF : pulse output low for 6 ms                         (none)
;;                  (bits 0-3 mask the different output lines, 0 = pulse line, 1 = don't pulse line ; bit 0 is the reset line)
;;
;; i8042 Controller Configuration Byte
;;  - bit 0: first PS/2 port interrupt (set = enabled, cleared = disabled)
;;  - bit 1: second PS/2 port interrupt (set = enabled, cleared = disabled)
;;  - bit 2: system flag (set = system passed POST, cleared = system failed POST)
;;  - bit 3: reserved (must be 0)
;;  - bit 4: first PS/2 port clock (set = enabled, cleared = disabled)
;;  - bit 5: second PS/2 port clock (set = enabled, cleared = disabled)
;;  - bit 6: first PS/2 port translation (set = enabled, cleared = disabled)
;;  - bit 7: reserved (must be 0)
;;
;; i8042 Controller Output Port
;;  - bit 0: system reset (always set)
;;  - bit 1: A20 gate (set = enabled, cleared = disabled)
;;  - bit 2: second PS/2 port clock
;;  - bit 3: second PS/2 port data
;;  - bit 4: output buffer full with byte from first PS/2 port (IRQ1)
;;  - bit 5: output buffer full with byte from second PS/2 port (IRQ12)
;;  - bit 6: first PS/2 port clock
;;  - bit 7: first PS/2 port data

;; i8042 Commands used here
%define PS2_READ_CONFIG     0x20
%define PS2_WRITE_CONFIG    0x60
%define PS2_TEST_PORT_1     0xAB
%define PS2_DISABLE_PORT_1  0xAD
%define PS2_ENABLE_PORT_1   0xAE
%define PS2_DISABLE_PORT_2  0xA7
%define PS2_SELF_TEST       0xAA

;; PS2 Devices commands
%define PS2_IDENTIFY        0xF2
%define PS2_ENABLE_SCAN     0xF4
%define PS2_DISABLE_SCAN    0xF5
%define PS2_ACK             0xFA
%define PS2_RESET           0xFF
%define PS2_RESET_FAILED_0  0xFC
%define PS2_RESET_FAILED_1  0xFD
%define PS2_RESET_PASSED    0xAA

;; Constants
%define PS2_TEST_OK         0x55
%define PS2_CONFIG_MASK     00011100b   ; completely disables second PS/2 port, translation and first port interrupts
%define PS2_ERROR           -1

global initialize_ps2_controller
global identify_ps2_port_1
global send_byte_ps2_port_1
global recv_byte_ps2_port_1

section .text
;; Initializes the PS/2 Controller, returns a non-zero value on failure
initialize_ps2_controller:
; disables all device channels
    mov al, PS2_DISABLE_PORT_1
    out PS2_COMMAND_PORT, al
    mov al, PS2_DISABLE_PORT_2
    out PS2_COMMAND_PORT, al

; flush the controller buffer
    in al, PS2_DATA_PORT

; disables interrupts and translation
    mov al, PS2_READ_CONFIG
    out PS2_COMMAND_PORT, al
.L0:
    in al, PS2_STATUS_PORT
    and al, 0x1
    jz .L0     ; poll until the config byte is available
    in al, PS2_DATA_PORT
    and al, PS2_CONFIG_MASK
    mov cl, al
    mov al, PS2_WRITE_CONFIG
    out PS2_COMMAND_PORT, al
.L1:
    in al, PS2_STATUS_PORT
    and al, 0x2
    jnz .L1     ; poll until the input buffer is ready
    mov al, cl
    out PS2_DATA_PORT, al

; performs controller self test
    mov al, PS2_SELF_TEST
    out PS2_COMMAND_PORT, al
.L2:
    in al, PS2_STATUS_PORT
    and al, 0x1
    jz .L2
    in al, PS2_DATA_PORT
    cmp al, PS2_TEST_OK
    je .L3
    mov eax, PS2_ERROR
    ret
.L3:
; restores the configuration byte
    mov al, PS2_WRITE_CONFIG
    out PS2_COMMAND_PORT, al
.L4:
    in al, PS2_STATUS_PORT
    and al, 0x2
    jnz .L4     ; poll until the input buffer is ready
    mov al, cl
    out PS2_DATA_PORT, al

; tests the first PS/2 port
    mov al, PS2_TEST_PORT_1
    out PS2_COMMAND_PORT, al
.L5:
    in al, PS2_STATUS_PORT
    and al, 0x1
    jz .L5      ; poll until the response byte is available
    in al, PS2_DATA_PORT
    test al, al
    jz .L6  ; test OK
    mov eax, PS2_ERROR
    ret

; re-enables the first PS/2 port and the interrupts on IRQ1
.L6:
    mov al, PS2_ENABLE_PORT_1
    out PS2_COMMAND_PORT, al
    or cl, 0x1  ; enables first port interrupts
    mov al, PS2_WRITE_CONFIG
    out PS2_COMMAND_PORT, al
.L7:
    in al, PS2_STATUS_PORT
    and al, 0x2
    jnz .L7     ; poll until the input buffer is ready
    mov al, cl
    out PS2_DATA_PORT, al

; reset device on the first PS/2 por, disable interrupts to avoid receiving IRQ1
    cli
    call reset_ps2_port_1
    ret

;; retrieves "identify" information from the device connected to the first PS/2 port, puts the result in EAX
;; if the higher bits of eax are set, an error occured
identify_ps2_port_1:
    xor eax, eax
    ; time-out timer
    mov dx, 0x1000
.L0:
    test dx, dx
    jz LATENCY_ERROR
    dec dx
    ; creates a small latency
    mov al, 0x20
    out 0x80, al
    ; reads buffer status
    in al, PS2_COMMAND_PORT
    and al, 0x2
    jnz .L0
    mov dx, 0x1000
.L1_RESEND:
    mov al, PS2_DISABLE_SCAN
    out PS2_DATA_PORT, al
.L1:
    test dx, dx
    jz LATENCY_ERROR
    dec dx
    mov al, 0x20
    out 0x80, al
    in al, PS2_COMMAND_PORT
    and al, 0x1
    jz .L1
    in al, PS2_DATA_PORT
    cmp al, PS2_ACK
    jne .L1_RESEND
    mov dx, 0x1000
.L2:
    test dx, dx
    jz LATENCY_ERROR
    dec dx
    mov al, 0x20
    out 0x80, al
    in al, PS2_COMMAND_PORT
    and al, 0x2
    jnz .L2
    mov dx, 0x1000
.L3_RESEND:
    mov al, PS2_IDENTIFY
    out PS2_DATA_PORT, al
.L3:
    test dx, dx
    jz LATENCY_ERROR
    dec dx    
    mov al, 0x20
    out 0x80, al
    in al, PS2_COMMAND_PORT
    and al, 0x1
    jz .L3
    in al, PS2_DATA_PORT
    cmp al, PS2_ACK
    jne .L3_RESEND
    mov dx, 0x1000
.L4:
    test dx, dx
    jz LATENCY_ERROR
    dec dx
    mov al, 0x20
    out 0x80, al
    in al, PS2_COMMAND_PORT
    and al, 0x1
    jz .L4
    in al, PS2_DATA_PORT
    mov cl, al
    mov dx, 0x1000
.L5:
    test dx, dx
    jz .L6
    dec dx
    mov al, 0x20
    out 0x80, al
    in al, PS2_COMMAND_PORT
    and al, 0x1
    jz .L5
    in al, PS2_DATA_PORT
    mov ch, al
.L6:
    xor eax, eax
    mov ax, cx
    ret

;; resets the devices connected to the first PS/2 port
reset_ps2_port_1:
;; check if the device can identify, if not, it is not even worth it to reset it
    call identify_ps2_port_1
    cmp eax, 0xFFFF
    jle .L1
    mov eax, PS2_ERROR
    ret
.L1:
    mov ecx, eax
    mov dx, 0x1000
.L2:
    test dx, dx
    jz LATENCY_ERROR
    dec dx
    mov al, 0x20
    out 0x80, al
    in al, PS2_COMMAND_PORT
    and al, 0x2
    jnz .L2
    mov dx, 0x1000
.L3_RESEND:
    mov al, PS2_RESET
    out PS2_DATA_PORT, al
.L3:
    test dx, dx
    jz LATENCY_ERROR
    dec dx
    mov al, 0x20
    out 0x80, al
    in al, PS2_COMMAND_PORT
    and al, 0x1
    jz .L3
    in al, PS2_DATA_PORT
    cmp al, PS2_RESET_FAILED_0
    je LATENCY_ERROR
    cmp al, PS2_RESET_FAILED_1
    je LATENCY_ERROR
    cmp al, PS2_ACK
    jne .L3_RESEND
    mov dx, 0x1000
.L4:
    test dx, dx
    jz LATENCY_ERROR
    dec dx
    mov al, 0x20
    out 0x80, al
    in al, PS2_COMMAND_PORT
    and al, 0x1
    jz .L4
    in al, PS2_DATA_PORT
    cmp al, PS2_RESET_PASSED
    jne LATENCY_ERROR
;; reset done, OK
    xor eax, eax
    ret

;; procedure used OUTSIDE of this file to send data to the device connected to the first PS/2 port
send_byte_ps2_port_1:
    mov dx, 0x1000
.L0:
    test dx, dx
    jz LATENCY_ERROR
    dec dx
    mov al, 0x20
    out 0x80, al
    in al, PS2_COMMAND_PORT
    and al, 0x2
    jnz .L0
    mov al, cl
    out PS2_DATA_PORT, al
    xor eax, eax
    ret

recv_byte_ps2_port_1:
    xor eax, eax
    mov dx, 0x1000
.L0:
    test dx, dx
    jz LATENCY_ERROR
    dec dx
    mov al, 0x20
    out 0x80, al
    in al, PS2_COMMAND_PORT
    and al, 0x1
    jz .L0
    in al, PS2_DATA_PORT
    ret

;; called whenever there is a latency error, or an error in general
LATENCY_ERROR:
    mov eax, PS2_ERROR
    ret
