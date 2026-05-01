; Bootloader - 512 bytes, loads kernel at 0x1000
; NASM syntax

[bits 16]
[org 0x7C00]

start:
    cli                     ; Disable interrupts
    xor ax, ax              ; Setup stack
    mov ss, ax
    mov sp, 0x7C00
    
    mov ax, 0x0003          ; Set text mode
    int 0x10
    
    mov si, msg_loading
    call print_string
    
    ; Load kernel from disk
    mov ax, 0x0001          ; Read 1 sector
    mov bx, 0x1000          ; Load to 0x0000:0x1000
    mov cx, 0x0001          ; Sector 1
    mov dx, 0x0000          ; Drive 0 (floppy)
    mov es, bx
    xor bx, bx
    
    mov ah, 0x02            ; BIOS read sector
    int 0x13
    jc disk_error           ; Jump if error
    
    mov si, msg_done
    call print_string
    
    ; Jump to kernel
    jmp 0x1000:0x0000

disk_error:
    mov si, msg_error
    call print_string
    jmp $

print_string:
    mov ah, 0x0E
    mov bx, 0x0007
.next_char:
    lodsb
    or al, al
    jz .done
    int 0x10
    jmp .next_char
.done:
    ret

msg_loading db 'Loading kernel...', 0x0D, 0x0A, 0
msg_done    db 'OK', 0x0D, 0x0A, 0
msg_error   db 'Disk error!', 0x0D, 0x0A, 0

times 510-($-$$) db 0
dw 0xAA55
