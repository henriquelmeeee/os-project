[bits 64]

global syscall_handler_stub
extern handle_syscall_routine

syscall_handler_stub:
  cli
  push rbp
  mov rbp, rsp
  push rax
  push rdi
  push rsi
  push rcx
  push rdx
  push r8
  push r9
  push r10
  push r11
  push r12
  push r13
  push r14
  push r15
  call handle_syscall_routine
  pop r15
  pop r14
  pop r13
  pop r12
  pop r11
  pop r10
  pop r9
  pop r8
  pop rdx
  pop rcx
  pop rsi
  pop rdi
  pop rax
  pop rbp
  sti
  sysret
