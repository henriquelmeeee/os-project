// "proc_currenct" é o processo atual e vem de Process.cpp

#include "../Tasks/Process.h"
#include "../Memory/Base_Mem.h"
#include "../Processor.h"
#include "../Interruptions/Stack.h"

#define TOTAL_AMOUNT_OF_SYSCALLS 2

#define SYSCALL_ARGS struct SyscallStack* s

enum Syscalls {
  EXIT,
  WRITE,
};

u64 NO_CALLER_SAVED_REGISTERS volatile sys$exit(SYSCALL_ARGS) {
  // TODO FIXME implement that
  int id = 0;
  __asm__ volatile (
      "mov %%eax, %0;" // eax é usado para definir o ID da saída
      : "=r" (id)
      ); 
  return 1;
}

u64 NO_CALLER_SAVED_REGISTERS volatile sys$write(SYSCALL_ARGS) {
  // TODO FIXME implement that
  return 1;
}

void __attribute__((interrupt)) handle_syscall_routine(SYSCALL_ARGS) {
  __asm__ volatile(
    "push %%rax;"
    "push %%rbx;"
    "push %%rcx;"
    "push %%rdx;"
    "push %%rsi;"
    "push %%rdi;"
    "push %%rbp;"
    "push %%r8;"
    "push %%r9;"
    "push %%r10;"
    "push %%r11;"
    "push %%r12;"
    "push %%r13;"
    "push %%r14;"
    "push %%r15;"
    :
    :
    : "r15", "r14", "r13", "r12", "r11", "r9", "r8", "rbp", "rdi", "rsi", "rdx", "rcx", "rbx", "rax"
  );
  unsigned long* stack_ptr; 
  __asm__ volatile("mov %%rsp, $0" : "=r"(stack_ptr));
  unsigned long syscall_id = stack_ptr[0];
  u64 to_ret;

  Process::SysProc* actual_process = Process::proc_current;
  Processor->get_eax(&syscall_id);
  switch (syscall_id) {
    case EXIT:
      to_ret = sys$exit(s); break;
    case WRITE:
      to_ret = sys$write(s); break;
    default:
      to_ret=0;break;
  }
  __asm__ volatile(
    "pop %%r15;"
    "pop %%r14;"
    "pop %%r13;"
    "pop %%r12;"
    "pop %%r11;"
    "pop %%r10;"
    "pop %%r9;"
    "pop %%r8;"
    "pop %%rbp;"
    "pop %%rdi;"
    "pop %%rsi;"
    "pop %%rdx;"
    "pop %%rcx;"
    "pop %%rbx;"
    "pop %%rax;"
    "mov %0, %%rax;"
    :
    : "=r"(to__ret)
    : "r15", "r14", "r13", "r12", "r11", "r9", "r8", "rbp", "rdi", "rsi", "rdx", "rcx", "rbx", "rax"
  );

}
