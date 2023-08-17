// "proc_currenct" é o processo atual e vem de Process.cpp

#include "../Tasks/Process.h"
#include "../Memory/Base_Mem.h"
#include "../Processor.h"
#include "../Interruptions/Stack.h"
#include "Syscall.h"

#define TOTAL_AMOUNT_OF_SYSCALLS 2

#define SYSCALL_ARGS struct SyscallStack* s

enum Syscalls {
  EXIT,
  WRITE,
};

u64 NO_CALLER_SAVED_REGISTERS volatile sys$exit(SYSCALL_ARGS) {
  // TODO FIXME implement that
  return 1;
}

u64 NO_CALLER_SAVED_REGISTERS volatile sys$write(SYSCALL_ARGS) {
  // TODO FIXME implement that
  return 1;
}

void __attribute__((interrupt)) handle_syscall_routine(SYSCALL_ARGS) {
  CLI;
  dbg("syscall found");
  unsigned long syscall_id = s->rax;
  u64 to_ret;

  //Process::SysProc* actual_process = Process::proc_current;
  switch (syscall_id) {
    case EXIT:
      to_ret = sys$exit(s); break;
    case WRITE:
      to_ret = sys$write(s); break;
    default:
      to_ret=0;break;
  }
  STI;
}
