[bits 16]
[org 0x7C00]

; memory layout:

; 10.000 addr -> prekernel
; 20.000 addr -> kernel
; 100.000 addr -> amount of ram, TEMPORARY BUFFER (unsigned long, QWORD)
; 1.000.000 - 500.000 addr -> stack (starts from 1.000.000)
; 2.000.000 - 10.000.000 addr -> page table for kernel
; 
; kernel can overflow 100.000 addr after some heap allocations, but in this state
; kernel will already have RAM size information

CODE_SELECTOR equ 0x08
DATA_SELECTOR equ 0x10

E820RowSize equ 2
E820MaxRows equ 2

start:
  xor ax, ax
  mov ds, ax
  ;deprecated_vga_graphics_mode:
    ;mov ax, 0x0013
    ;int 0x10
  xor ax, ax
  cld
  mov ah, 0x02
  mov al, SECTORS_PREKERNEL
  mov ch, 0
  mov cl, 2
  mov dh, 0
  mov dl, 0x80
  xor bx, bx
  mov es, bx
  mov bx, 0x1000 ; loads prekernel in 0x1000 addr
  int 0x13

  VBE_graphics_init:

    ; 00h = informações da VBE
    ; 01h = informações de um modo de vídeo específico
    ; 02h = define modo de vídeo
    ; 09h = informações sobre controlador de gráfico

    ;mov ax, 0x4F02 ; define modo grafico
    ;mov bx, 0x112 ; RGB
    ;int 0x10
  
    ;mov ax, 0x4F01 ; obter informações do modo de vídeo
    ;mov cx, 0x112
    ;mov di, 0x5000 ; endreço para armazenar a estrutura
    ;int 0x10

  ;jc disk_error
  ;jnc cont
;disk_error:
  ;mov ah, 0x0e
  ;mov al, 'E'
  ;int 0x10
  ;cli
  ;hlt
  

  lgdt [gdt_descriptor]
  mov eax, cr0
  or eax, 1
  mov cr0, eax
[bits 32]
  db 0x66
  jmp CODE_SELECTOR:start_protected_mode

; a GDT não será efetivamente usada porque o Kernel será responsável por configurar
; os segmentos através da paginação
; ainda sim, por questões de compatibilidade, usaremos a GDT
; nós definimos que o endereço-base e final de ambos segmentos são 0-4GB

gdt_start:
  dq 0x0
gdt_code:
  dw 0xFFFF
  dw 0x0
  db 0x0
  db 0x9A ; segmento pode ser lido e executado
  db 0XCF
  db 0x0
gdt_data:
  dw 0xFFFF 
  dw 0x0 ; endereço do segmento (bits 0-15)
  db 0x0 ; endereço do segmento (bits 16-23)
  db 0x92
  db 0xCF
  db 0x0 ; endereço do segmento (bits 24-31)
gdt_end:

gdt_descriptor:
  dw gdt_end - gdt_start ; calcula tamanho da gdt
  dd gdt_start

start_protected_mode:
  mov esp, (0x7c00 - 0x200)
  mov ebp, esp

  mov ax, DATA_SELECTOR
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax
  mov ecx, dword [E820Map]
  ;mov dword [0x5000], ecx
  jmp CODE_SELECTOR:0x1000 ; prekernel

E820Map: times 4 db 0

times 510-($-$$) db 0
dw 0xAA55
