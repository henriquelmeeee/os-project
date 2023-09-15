// "proc_currenct" é o processo atual e vem de Process.cpp

#include "../Tasks/Process.h"
#include "../Memory/Base_Mem.h"
#include "../Processor.h"
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


extern "C" void handle_syscall_routine(u64 syscall_id, u64 arg2, u64 arg3, u64 arg4, u64 arg5) {
  CLI;
  dbg("syscall encontrada");
  return;

  switch(syscall_id) {
    case 0:
      // TODO exit() syscall
    case 1:
      {
        char* pointer_to_string = (char*)arg2;
        Text::Write(pointer_to_string);
        goto __syscall_exit;
      };

    default:
      {
        throw_panic(0, "panic temporário: syscall id invalido");
        goto __syscall_exit;
      };
  }

__syscall_exit:
  return;

}
