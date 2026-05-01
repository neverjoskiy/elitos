; Kernel entry point - switches to protected mode
; NASM syntax

section .text align=16
bits 16

extern kernel_main
global _start

_start:
    cli                     ; Disable interrupts
    
    ; Load GDT
    lgdt [gdt_descriptor]
    
    ; Enable protected mode (CR0 bit 0)
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    
    ; Far jump to 32-bit code
    jmp CODE_SEG:protected_mode_entry

bits 32
protected_mode_entry:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax
    
    ; Set up stack
    mov esp, kernel_stack + 4096
    
    ; Enable paging (CR0 bit 31) - will be done in C
    ; For now, just call kernel main
    call kernel_main
    
    ; If kernel returns, halt
    cli
.halt:
    hlt
    jmp .halt

; GDT
gdt_start:
    dq 0x0000000000000000   ; Null segment
    dq 0x00CF9A000000FFFF   ; Code segment (4GB, present, ring 0, execute/read)
    dq 0x00CF92000000FFFF   ; Data segment (4GB, present, ring 0, read/write)
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ 0x08
DATA_SEG equ 0x10

kernel_stack:
    times 4096 db 0
