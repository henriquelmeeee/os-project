// "proc_currenct" é o processo atual e vem de Process.cpp

#include "Tasks/Process.h"
#include "Driver/Keyboard.h"

#define TOTAL_AMOUNT_OF_SYSCALLS 2

void volatile sys_exit() {
  int id = 0;
  __asm__ volatile (
      "mov %%eax, %0;" // eax é usado para definir o ID da saída
      : "=r" (id)
      );
  // por enquanto, não usaremos nada do ID
  Process::procs[Process::proc_current->pid] = 0; // remove o processo da lista de processos
  
}

char sys_read_keyboard() { // syscall id 1
  return Driver::Keyboard::get_last_key();
}

char sys_wait_read_keyboard() { // syscall id 2
  char key, buffer = Driver::Keyboard::get_last_key();
  while(true) {
    buffer = Driver::Keyboard::get_last_key();
    if(buffer != key)
      break;
    else
      continue;
  }
  Driver::Keyboard::remove_last_key();
  // a função só irá de fato remover a última tecla caso proc_number == 0
  // se não ela só irá decrementar proc_number até o último processo
  return buffer;
}

unsigned long handle_syscall_routine(unsigned char syscall_id) {
  unsigned long callback;
  switch (syscall_id) {
    case 0:
      callback = sys_exit();
    case 1:
      callback = sys_read_keyboard();
      break;
    case 2:
      callback = sys_wait_read_keyboard();
      break;
    default:
      return 0;
  }
  return callback;

}
