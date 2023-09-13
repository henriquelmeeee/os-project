#include "../Tasks/Process.h"
#include "../Utils/Base.h"
#include "../Memory/Base_Mem.h"

void __attribute__((interrupt)) quantum_interruption_handle(void*) {
  CLI;
  dbg("interrupcao de timer!");
  asm volatile("hlt");
#if 0
  __asm__ volatile("hlt");
  // TODO semáforo para garantir q não haverá condição de corrida
  // pq se houver, o proximo processo da lista escolhido pode ser um que na mesma hora seja escolhido pelo 
  // outro processador(core)
  // também vamos pegar somente os processos que fazem parte do core atual
  // Calcular ciclos de clock

  unsigned long long cpu_cycles_used;
  __asm__ volatile(
    "rtscp"
    : "=A" (cpu_cycles_used));
  proc_current->cycles_finished = ( proc_current->cycles_started ) - ( cpu_cycles_used );
  proc_current->cycles_started = 0;

  // Salvar registradores
  
  //proc_current->regs_state = *state;

  // TODO calcular prioridade desse processo atual

  SysProc next_process = procs[0]; // TODO precisa pegar o próximo processo na lista
  __asm__ volatile(
      "mov %0, %%cr3"
      : "=A" (next_process.PML4)
      // cr3 = pti next proc
      );
  
  /*__asm__ volatile(
      "jmp %0"
      : "=r" (next_process->regs_state.rip)
      :
      :
      );*/
#endif

}
