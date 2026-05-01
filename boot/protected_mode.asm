; Kernel entry point - switches to protected mode
; NASM syntax

; Multiboot header (must be in first 8KB of kernel)
section .multiboot
align 4
    dd 0x1BADB002           ; Magic number
    dd 0x00                 ; Flags
    dd -(0x1BADB002 + 0x00) ; Checksum

section .text align=16
bits 32

extern kernel_main
global _start

_start:
    cli

    ; GRUB already put us in 32-bit protected mode
    ; Just set up segments and stack
    mov ax, 0x10            ; Data segment selector
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

    ; Set up stack
    mov esp, kernel_stack + 4096

    call kernel_main

    ; If kernel returns, halt
    cli
.halt:
    hlt
    jmp .halt

kernel_stack:
    times 4096 db 0
