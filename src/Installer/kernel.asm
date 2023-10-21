[org 0x1000]
[bits 16]

jmp main
video_addr equ 0xB8000
position resb 1

main:
  mov si, msg_starting_installer
  call kprint
  
  call configure_video
  
  call start_installer
  mov si, unexpected_error
  call kprint
  hlt
  jmp $

configure_video:
  push bp
  mov bp, sp
  call reset_cursor
  ; Tudo azul!
  mov ah, 9
  mov al, ' '
  mov bh, 0
  mov bl, 0x1f
  mov cx, 2000 ; quantidade
  int 0x10
  call reset_cursor
  pop bp
  ret

reset_cursor:
  push bp
  mov bp, sp
  xor bh, bh ; pagina de video
  xor dh, dh ; linha
  xor dl, dl ; coluna
  mov ah, 2
  int 0x10
  pop bp
  ret

kprint:
  push bp
  mov bp, sp

  mov ah, 0x0e
  mov bh, 0
  mov bl, 7 ; cinza sobre preto

  next_char:
    mov al, byte [si]
    test al, al
    jz done

    int 0x10
    inc si
    jmp next_char

  done:
    pop bp
    ret


msg_starting_installer db "Starting the installer...", 0
unexpected_error db "Unexpected error", 0

start_installer:
  push bp
  mov bp, sp



  ;hlt
  pop bp
  ret 

times 512-($-$$) db 0
