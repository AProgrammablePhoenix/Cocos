BITS 64

%define PAT_UNSUPPORTED          1
%define SETUP_OK                 0
%define ERROR_CPUID_UNSUPPORTED -1
%define ERROR_NXE_UNSUPPORTED   -2

%define CPUID_PAT               0x00000001
%define CPUID_CAPABILITIES      0x80000000
%define CPUID_NXE               0x80000001
%define CPUID_ADDRESSING        0x80000008

%define CPUID_NXE_MASK          0x00100000
%define EFER_MSR                0xC0000080
%define EFER_NXE_MASK           0x00000800
%define MAXPHYADDR_MASK         0x000000FF
%define CPUID_PAT_MASK          0x00010000
%define MTRR_MSR                0x00000277
%define PAT_WC                  0x01

section .text
global efi_loader_setup

; error status in EAX, if an error occured on CPUID, the unsupported command code is placed in r9
efi_loader_setup:
; force-enables SSE
    mov rax, cr0
    and ax, 0xFFFB
    or ax, 0x2
    mov cr0, rax
    mov rax, cr4
    or ax, (3 << 9)
    mov cr4, rax
; loads CPUID capabilities
    mov eax, CPUID_CAPABILITIES
    cpuid
    mov r11d, eax

; prepares NXE bit
    ; checks if CPUID supports command 0x80000001
    mov r9d, CPUID_NXE
    cmp r11d, r9d
    mov eax, ERROR_CPUID_UNSUPPORTED
    jl return
    mov eax, CPUID_NXE
    cpuid
    ; check if CPU supports NXE bit in EFER
    and edx, CPUID_NXE_MASK
    mov eax, ERROR_NXE_UNSUPPORTED
    je return
    mov ecx, EFER_MSR
    rdmsr
    or eax, EFER_NXE_MASK
    wrmsr

; recovers the physical address width and puts it in r10
    ; checks if CPUID supports command 0x80000008
    mov r9d, CPUID_ADDRESSING
    cmp r11d, r9d
    mov rax, ERROR_CPUID_UNSUPPORTED
    jl return
    mov eax, CPUID_ADDRESSING
    cpuid
    mov r10d, eax
    and r10, MAXPHYADDR_MASK

; no code after this instruction may generate a fatal error
    xor eax, eax
; programs PAT4 in the PAT MSR in order to set up Write Combining, does not generate an error if PAT is unsupported, but puts the value PAT_UNSUPPORTED in eax
    mov eax, CPUID_PAT
    cpuid
    and edx, CPUID_PAT_MASK
    test edx, edx
    jne .L0
    or rax, PAT_UNSUPPORTED
    jmp .L1
.L0:
    push rax
    mov ecx, MTRR_MSR
    rdmsr
    mov dl, PAT_UNSUPPORTED
    wrmsr
    pop rax
.L1:
return:
    ret