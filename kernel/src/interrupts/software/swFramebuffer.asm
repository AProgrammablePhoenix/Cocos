;;;;; Cocos framebuffer management procedures
;;;;
;;;
;;
;; Checks performed:
;; - Task is either in ring 0, or is 
;; Calling convetions MS-ABI with function ID in RAX

BITS 64

extern swFramebufferRequest
extern swFramebufferWrite

global swFramebufferManager

section .rodata
procedure_table:
    dq swFramebufferRequest
    dq swFramebufferWrite
procedure_table_end:

%define PROCEDURE_TABLE_ENTRIES ((procedure_table_end - procedure_table) / 8)

section .text

;; Forwards calls to the different procedures
;; - RAX: function to be used
swFramebufferManager:
    mov rdx, [rsp + 8] ; CS
    cmp rax, PROCEDURE_TABLE_ENTRIES
    jge .L0
    lea rax, [rel procedure_table]
    lea rax, [rax + 8*rax]
    call rax
.L0:
    iretq
