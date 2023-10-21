[org 0x7c00]
[bits 16]

jmp main

main:
  cli
  mov ax, 0
  mov ds, ax
  mov es, ax
  mov gs, ax
  hlt
