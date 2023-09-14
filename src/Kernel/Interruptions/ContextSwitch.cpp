#include "../Tasks/Process.h"
#include "../Utils/Base.h"
#include "../Memory/Base_Mem.h"
#include "preload.h"

#include "../Tasks/KernelTasks/Watchdog.h"

void __attribute__((interrupt)) quantum_interruption_handle(InterruptFrame *args) {
  // por enquanto, esse handler apenas pega um unico processo
  // q é o watchdog
  // cria ele (pq o Memory::Vector n ta funcionando com append direito)
  // e então pega seus registradores, coloca, e faz jmp nele
  // é só um protótipo funcional
  CLI;
  SAVE_ALL_REGISTERS();
  u64 __saved_rip = args->rip;
  u64 __saved_rsp = args->rsp;
  u64 __saved_rbp;
  u64* __rbp = nullptr;

  __asm__ volatile(
      "movq %0, %%rbp"
      : "=r" (__rbp)
      :
      : "memory"
    );
  __saved_rbp = *__rbp;

  if(g_current_proc != nullptr) {
    g_current_proc->m_regs.rip = __saved_rip;
    g_current_proc->m_regs.rsp = __saved_rsp;
    g_current_proc->m_regs.rbp = __saved_rbp;
  }

  Process* next_proc = g_kernel_procs[0];
  if(next_proc->m_regs.rip == 0) {
    throw_panic(0, "m_regs.rip invalido"); // TODO mensagem bonita de erro 
  }
  g_current_proc = next_proc;
  dbg("NEXT_PROC->m_regs.rip = %p", (void*)(next_proc->m_regs.rip));
  u64 rax, rbx, rcx, rdx, rdi, rsi, rbp, r8, r9, r10, r11, r12, r13, r14, r15;
  u64 rip;
  rax = next_proc->m_regs.rax;
  rbx = next_proc->m_regs.rbx;
  rcx = next_proc->m_regs.rcx;
  rdx = next_proc->m_regs.rdx;
  rdi = next_proc->m_regs.rdi;
  rsi = next_proc->m_regs.rsi;
  rbp = next_proc->m_regs.rbp;
  r8 = next_proc->m_regs.r8;
  r9 = next_proc->m_regs.r9;
  r10 = next_proc->m_regs.r10;
  r11 = next_proc->m_regs.r11;
  r12 = next_proc->m_regs.r12;
  r13 = next_proc->m_regs.r13;
  r14 = next_proc->m_regs.r14;
  r15 = next_proc->m_regs.r15;

  rip = next_proc->m_regs.rip;

  __asm__ volatile(
    "popq %%r15;"
    "popq %%r14;"
    "popq %%r13;"
    "popq %%r12;"
    "popq %%r11;"
    "popq %%r10;"
    "popq %%r9;"
    "popq %%r8;"
    "popq %%rbp;"
    "popq %%rsi;"
    "popq %%rdi;"
    "popq %%rdx;"
    "popq %%rcx;"
    "popq %%rbx;"
    "movb $0x20, %%al;"
    "outb %%al, $0x20;"
    "popq %%rax;"
    "sti;"
    "jmp *%0;"
    "cli;"
    "hlt;"
    :
    : "m" (rip)
    //: "r15", "r14", "r13", "r12", "r11", "r10", "r9", "r8",
      //"rbp", "rsi", "rdi", "rdx", "rcx", "rbx", "rax"
    :
  );



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
