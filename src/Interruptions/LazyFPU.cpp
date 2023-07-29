#include <Tasks/Process.h>
#include <Utils/Base.h>

void lazy_fpu_context_switch() {
  CLI;
  
  if(last_proc_used_fpu != 0)
    __asm__ volatile(
        "fnsave %0"
        : "=m" (last_proc_used_fpu->fpu_state)
        );

  if(proc_current->used_fpu) {
    __asm__ volatile(
        "frstor %0"
        : "=m" (proc_current->fpu_state)
        );
  } else {
    proc_current->used_fpu = true;
    // TODO reset all FPU state
    __asm__ volatile("finit");
    __asm__ volatile(
        "fnsave %0"
        : "=m" (proc_current->fpu_state)
        );

    // é importante salvar o estado padrão da FPU no processo
    // porque isso inclui registradores de controle
    //
    // outra maneira de fazer isso é salvando o estado padrão da FPU em "fpu_state" 
    // no momento da criação do processo
    
  }

  last_proc_used_fpu = proc_current;

  STI;
}
