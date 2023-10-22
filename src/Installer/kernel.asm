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

%define MID_LINE 11
%define MID_COLUMN 33

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
  call enumerate_disks
  call display_disk_options

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

display_disk_options:
  push bp
  mov bp, sp
  
  call configure_video

  ; First disk
  mov LINE, (MID_LINE - 1)
  mov COLUMN, MID_COLUMN
  call set_video
  mov si, first_disk_option
  call kprint

  mov ax, bool_second_disk_exists
  jnz second_disk_print
  jz wait_for_disk_option

  second_disk_print:
    mov LINE, (MID_LINE)
    mov COLUMN, MID_COLUMN
    call set_video
    mov si, second_disk_option
    call kprint

    mov ax, bool_third_disk_exists
    jnz third_disk_print
    jz wait_for_disk_option

  third_disk_print:
    mov LINE, (MID_LINE + 1)
    mov COLUMN, MID_COLUMN
    call set_video
    mov si, third_disk_option
    call kprint

  wait_for_disk_option:
    hlt

  pop bp
  ret

enumerate_disks:
  pusha
  mov ah, 0x41
  mov dl, 0x80
  int 0x13
  jc no_drive_error

  mov ah, 0x41
  mov dl, 0x81
  int 0x13
  jc enumerate_disks_end

  mov al, 1
  mov [bool_second_disk_exists], al

  mov ah, 0x41
  mov dl, 0x82
  int 0x13
  jc enumerate_disks_end

  mov al, 1
  mov [bool_third_disk_exists], al

  enumerate_disks_end:
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

first_disk_option db "1. Disk 0", 0
second_disk_option db "2. Disk 1", 0
third_disk_option db "3. Disk 2", 0
bool_second_disk_exists db 0
bool_third_disk_exists db 0

current_disk_choice db 0

times 512-($-$$) db 0
