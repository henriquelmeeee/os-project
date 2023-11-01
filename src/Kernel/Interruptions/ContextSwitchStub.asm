[bits 64]


global timer_isr
global second_timer_isr
extern quantum_interruption_handle

timer_vezes db 0

timer_isr:
  cli
  push rbp
  mov rbp, rsp
  push rdx ; TODO FIXME tem q fazer isso ser "push rdx" mas tem q cuidar da stack pq inicialmente isso era "pop" mas ta errado
  push rax
  push rdi
  push rsi
  push rbx
  push rcx
  push r8
  push r9
  push r10
  push r11
  push r12
  push r13
  push r14
  push r15
  mov rax, rsp
  call quantum_interruption_handle
second_timer_isr:
  pop r15
  pop r14
  pop r13
  pop r12
  pop r11
  pop r10
  pop r9
  pop r8
  pop rcx
  pop rbx
  pop rsi
  pop rdi
  mov al, 0x20
  out 0x20, al
  pop rax
  pop rdx
  pop rbp

  ;pop rsp


  ; debugging purposes
 ; pop rax
 ; pop rbx
 ; pop rdi
  ; ...

  ; TODO FIXME como o 'iretq' está com problemas
  ; irei manualmente fazer ele
  ; mas isso significa ignorar os rx registers
  
  cmp byte [timer_vezes], 1
  je __debug
  popf ; rflags
  pop r12 ; cs
  pop r13 ; rip
  ; esquece o CS por agora
  inc byte [timer_vezes]
  sti
  jmp r13

; FIXME
; depois que o timer vai na segunda vez,
; parece haver alguma inconsistência
__debug:
  pop rax ; deve ser rflags
  pop rdi ; deve ser cs
  pop rsi ; deve ser rip
  int 3
