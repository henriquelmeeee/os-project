[org 0x7c00]
[bits 16]

jmp main

main:
; "cs" starts with "0x07c0"
  cli
  mov ax, 0x07c0
  mov ds, ax

  mov ax, 0x1000
  mov es, ax
  cld

  mov ah, 0x02
  mov al, SECTORS_KERNEL
  mov ch, 0
  mov cl, 2 ; setor inicial
  mov dh, 0
  mov dl, 0x80
  mov bx, 0x1000
  int 0x13

  jmp 0x1000:0000
  mov si, msg
  mov di, si
  jmp print_error_and_hlt

print_error_and_hlt:
  mov ah, 0x0e
  mov al, byte [si]
  mov bh, 0
  mov bl, 7

  next_char:
    cmp al, 0
    je done
    int 0x10
    inc di
    mov al, byte [msg+di]
    jmp next_char
  done:
    hlt
  
msg db "Fatal: kernel unexpected returned to bootloader", 0

times 510-($-$$) db 0
dw 0xAA55
