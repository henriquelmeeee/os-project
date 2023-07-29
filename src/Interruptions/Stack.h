#ifndef STACK
#define STACK
#include "../Utils/Base.h" 

#define IRET \
  outb(0x20, 0x20); \
  outb(0xA0, 0x20); \
  __asm__ volatile( \
      "add $8, %rsp;" \
      "pop %r15;" \
      "pop %r14;" \
      "pop %r13;" \
      "pop %r12;" \
      "pop %r11;" \
      "pop %r10;" \
      "pop %r9;" \
      "pop %r8;" \
      "pop %rbp;" \
      "pop %rdi;" \
      "pop %rsi;" \
      "pop %rdx;" \
      "pop %rcx;" \
      "pop %rbx;" \
      "pop %rax;" \
      "iretq;" \
      )

// EFLAGS,CS,RIP -> são desempilhados automaticamente no "iretq"
struct State {
  u64 r15, r14, r13, r12, r11, r10, r9, r8;
  u64 rbp, rdi, rsi, rdx, rcx, rbx, rax;
  u64 int_no, err_code;
  u64 rip, cs, eflags, rsp, ss;
};

struct ExceptionState {
  u64 rip, cs, rflags, rsp, ss;
};

/*struct State {
  unsigned short ip;
  unsigned short cs;
  unsigned short flags;
  unsigned short sp;
  unsigned short ss;
};*/
#endif

