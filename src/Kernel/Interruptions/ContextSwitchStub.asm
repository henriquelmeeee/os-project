[bits 64]


global timer_isr
global second_timer_isr
extern quantum_interruption_handle

timer_isr:
  cli
  push rsp
  push rbp
  mov rbp, rsp
  pop rdx
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
  mov rax, rbp
  sub rax, 16 ; agora RAX = RSP original do processo atual
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
  pop rsp
  sti
  iretq
