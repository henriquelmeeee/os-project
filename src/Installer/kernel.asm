[org 0x1000]
[bits 16]

jmp main
video_addr equ 0xB8000
position resb 1

%define LINE dh
%define COLUMN dl
%macro WAIT_FOR_INPUT 0
  push ax
  mov ah, 0
  int 0x16
  pop ax
%endmacro

main:
  mov ax, cs
  mov ds, ax
  mov si, msg_starting_installer
  call kprint
  
  call configure_video
  
  call start_installer
; Não podemos retornar de 'start_installer'
  call configure_video
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

set_video:
; dh = linha
; dl = coluna
  pusha
  xor bh, bh
  mov ah, 2
  int 0x10
  popa
  ret


kprint:
  push bp
  mov bp, sp

  kprint_loop:
    lodsb
    or al, al
    jz done
    mov ah, 0x0e
    mov bh, 0
    mov bl, 0x1f ; cinza sobre preto
    int 0x10
    jmp kprint_loop

  done:
    pop bp
    ret


msg_starting_installer db "Starting the installer...", 0
unexpected_error db "Unexpected error", 0

start_installer:
  push bp
  mov bp, sp
  
  call wait_for_any_key
  call check_first_disk_is_connected
  call display_installer_options

  hlt
  pop bp
  ret 

wait_for_any_key:
  mov LINE, 24
  mov COLUMN, 0
  call set_video
  mov si, press_any_key
  call kprint

  WAIT_FOR_INPUT

  ret

display_installer_options:
  push bp
  mov bp, sp
  
  call configure_video

  pop bp
  ret

check_first_disk_is_connected:
  pusha
  mov ah, 0x41
  mov bx, 0x55aa
  mov dl, 0x80
  int 0x13

  cmp bx, 0xaa55
  jnz no_drive_error
  popa
  ret




; Errors

no_drive_error:
  call configure_video
  mov si, no_drive
  call kprint

  hlt
  jmp $


press_any_key db "Press any key to start the installation...", 0
no_drive db "Fatal: No disks found", 0
installer_options db "", 0

times 512-($-$$) db 0
