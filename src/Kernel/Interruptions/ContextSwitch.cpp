#include "../Tasks/Process.h"
#include "../Utils/Base.h"
#include "../Memory/Base_Mem.h"
#include "preload.h"
#include "ContextSwitch.h"

#include "../Tasks/KernelTasks/Watchdog.h"


int kernel_procs_round_robin = 0;

extern "C" void quantum_interruption_handle(u64 rsp) {
  // por enquanto, esse handler apenas pega um unico processo
  // q é o watchdog
  // cria ele (pq o Memory::Vector n ta funcionando com append direito)
  // e então pega seus registradores, coloca, e faz jmp nele
  // é só um protótipo funcional
  Process* next_proc;
  if(g_kernel_procs[kernel_procs_round_robin] == nullptr)
    kernel_procs_round_robin = 0;
  next_proc = g_kernel_procs[kernel_procs_round_robin];
  ++kernel_procs_round_robin;
  dbg("RSP do processo atual é: %p", (void*)rsp);
  if(g_current_proc != nullptr)
    g_current_proc->m_regs.rsp = rsp;
  dbg("RSP do próximo processo é : %p", (void*)next_proc->m_regs.rsp);
  
  /*
   É importante utilizarmos "m_regs.rsp+128" porque
   a stack salva em m_regs é a stack original do processo
   que no caso é antes dele realmente salvar todos os registradores
   depois do interrupt frame
   então precisamos fazer com que cobremos os registradores salvos pelo push
  */

  __asm__ volatile(
    "mov %0, %%rsp;"
    "lea second_timer_isr(%%rip), %%rax;"
    "jmp *%%rax;"
    :
    : "r" ((u64)(next_proc->m_regs.rsp))
    :
    );

  throw_panic(0, "'ret' from quantum_interruption_handle failed");
  __asm__ volatile("hlt");
  //change_page_table((void*)kPML4);



  // TODO:
#if 0
  if(g_current_proc != nullptr) {
    g_current_proc->m_regs.rsp = args->rsp;
    __rbp = (u64*) (*__rbp);
    g_current_proc->m_regs.rbp = (u64)__rbp;
    // ...
  }
#endif



#if 0
  __asm__ volatile(
      "movq %%rsp, %%rbx\n"
      "movq %0, %%rsp\n"
      :
      : "r"(g_timer_temporary_stack)
      : "rbx", "rsp"
    );
  //u64 rax, rbx, rcx, rdx, rdi, rsi, rbp, r8, r9, r10, r11, r12, r13, r14, r15;
  Process* next_proc = &(g_kernel_procs[0]); // temporariamente!
      __asm__ volatile(
        "movq %0, %%rax\n"
        "movq %1, %%rbx\n"
        "movq %2, %%rcx\n"
        "movq %3, %%rdx\n"
        "movq %4, %%rsi\n"
        "movq %5, %%rdi\n"
        "movq %6, %%r8\n"
        "movq %7, %%r9\n"
        "movq %8, %%r10\n"
        "movq %9, %%r11\n"
        "movq %10, %%r12\n"
        "movq %11, %%r13\n"
        "movq %12, %%r14\n"
        "movq %13, %%r15\n"
        :
        : "m"(next_proc->m_regs.rax), "m"(next_proc->m_regs.rbx),
          "m"(next_proc->m_regs.rcx), "m"(next_proc->m_regs.rdx),
          "m"(next_proc->m_regs.rsi), "m"(next_proc->m_regs.rdi),
          "m"(next_proc->m_regs.r8), "m"(next_proc->m_regs.r9),
          "m"(next_proc->m_regs.r10), "m"(next_proc->m_regs.r11),
          "m"(next_proc->m_regs.r12), "m"(next_proc->m_regs.r13),
          "m"(next_proc->m_regs.r14), "m"(next_proc->m_regs.r15)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
    );

  __asm__ volatile("hlt");
  __asm__ volatile(
      "movq %0, %%rax\n"
      : "=m" (next_proc->m_regs.rax)
      :
      : "rax"
      );

  dbg("Troca de contexto iniciada pelo gatilho do timer");
  dbg("rip: %p", (void*) args->rip);

  u64 next_proc_stack = next_proc->m_regs.rsp;

  dbg("next_proc_stack = %p", (void*)next_proc_stack);
  dbg("rip para pular = %p", (void*) (*((u64*)next_proc_stack-8-8-8-8)));
  if(!(next_proc->m_kernel_process)); // TODO mudar tabela de paginação
  __asm__ volatile(
      "movq %%rsp, %%rbx\n"
      "movq %0, %%rsp\n"
      :
      : "r"(next_proc_stack)
      : "rbx", "rsp"

    );
  RESTORE_ALL_REGISTERS();
  asm volatile("iret");
  //STI;
  return;
  //asm volatile("hlt");
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
#endif

}
