//#include "Process.h"

#include "../Drivers/Disk.h"
#include "../Filesystem/Filesystem.h"
#include "../Drivers/VIDEO/preload.h"
#include "../Utils/Base.h"
#include "../Memory/Base_Mem.h"
#include "../panic.h"
#include "Process.h"
#include "KernelTasks/KTasks.h"
#if 0
namespace Process{

  #define vPage i16

  u64 amount_of_procs = 0;
  Memory::Vector<Process::SysProc> procs;

  SysProc::SysProc(const char *p_name, Binary *pbin, u32 pid) {

      //if(pbin->magic_number != BINARY_MAGIC_NUMBER)
        //return;
      
      //Utils::kmemcpy(name, p_name, 16);

      // TODO arredondar pra cima pra alinhar dados text_section_size e data_section_size
      //TextSection = (void)Memory::kmmap(pbin->text_section_size, 5);
      //DataSection = (void*)kmmap(pbin->data_section_size, 5);

      // TODO pegar as paginas alocadas de kmmap e jogar na tabela de páginas 
      // TODO configurar a tabela de paginas de acordo com o layout da memoria virtual dos processos
      
      /*
       * É aqui que a diversão começa
       * nós precisamos alocar a região de código, de dados e da stack na Heap
       * e então pegar o endereço resultante e colocar em cada página
      */
      
      
      dbg("Process::constructor-> Inicializando páginas para processo novo\n");

      int offset_to_text = pbin->text_section_offset;
      this->TextSection.start_address = (512*4096);

      __builtin_memcpy((char*)this->TextSection.start_address, (char*)pbin+offset_to_text, 4096);
      
      this->pid = pid;
      dbg("Process::constructor-> Alocado 4KB para .text e 4KB para .data\n");
      
      // Kernel identity-mapping (por enquanto!)

      this->PML4[0] = (unsigned long)(&(this->PDPT));
      this->PML4[0] |= 0x3;

      // kernel identity-mapping
      
      this->PDPT[0] = (unsigned long)(&(this->kPD)); // 0-1GB
      this->PDPT[0] |= 0x3;

      // text

      this->PDPT[1] = (unsigned long)(&(this->textPD));
      this->PDPT[1] |= 0x3;


      int actual_page=0; 
      for(int PD_entry = 0; PD_entry < 512; PD_entry++) {
        for(int PT_entry = 0; PT_entry < 512; PT_entry++) {
          this->kPT[PD_entry][PT_entry] = actual_page*4096;
          actual_page++;
          this->kPT[PD_entry][PT_entry] |= 0x3;
        }
        this->kPD[PD_entry] = (unsigned long)(&(this->kPT[PD_entry]));
        this->kPD[PD_entry] |= 0x3;
        
        this->textPD[PD_entry] = (unsigned long)(&(this->textPT[PD_entry]));
        this->textPD[PD_entry] |= 0x3;

      }

      this->textPT[0][0] = (unsigned long)((char*)(this->TextSection.start_address)); // NEEDS TO BE ALLIGNED!
      this->textPT[0][0] |= 0x3;

      dbg("TESTANDO NOVA PAGINAÇÃO\n");

      __asm__ volatile( 
        "mov %0, %%cr3;" 
        : 
        : "r" (this->PML4)
        :"rax","memory"
      );

      dbg("SE VC ESTA VENDO ESSA MENSAGEM, ENTAO FUNCIONOU\n");
      __asm__ volatile(
          "jmp %[target]"
          : [target] "=r" (this->TextSection.start_address)
          :
          :
          );

    };
    bool init() {
      // -----
      // Inicializando primeiro processo
      // -----
      procs = Memory::Vector<Process::SysProc>();
      dbg("Process::init()-> Vetor de processos inicializado\n");
      //TODO resolver problema em que uma chamada à memcpy() acontece se eu fizer atribuição direta
      //procs[0] = SysProc("init", "init", (u32)0);
      Binary* shell;
      read_from_sector((char*)shell, (unsigned long long) 500);
      dbg("Process::init()-> shell carregado\n");
      procs[0] = SysProc("Shell", shell, 1);

      dbg("Process::init()-> Finalizado com sucesso\n");
      // procs.append(SysProc(const_cast<char*>("init"), const_cast<char*>("init"), (u32)0)); // Inicializa um SysProc
      // pages_in_use.append(Page((SysProc*)&(procs[0]), (u64)0)); // Cria uma página para o novo SysProc
      return true;
}; 


};

Memory::Vector<Process::SysProc> procs;
Process::SysProc* proc_current = 0;

class Page {
  public:
    Process::SysProc* proc;
      u64 page_number;
      u64 base_addr;

      Page(Process::SysProc* p, u64 b_addr) {
        this->proc = p;
        this->page_number = b_addr / PAGE_SIZE;
        this->base_addr = b_addr;
      };
};



u32 getNewPid() {
  return 1;
}

bool CreateProcess(const char* name, u16 privilege, const char* fs_binary_location) {
  if(privilege != 0){ throw_panic(0, "CreateProcess::privilege>0: not yet implemented");}

  //Binary *bin = FS::LoadBinary(fs_binary_location);
  //if(bin->magic_number != BINARY_MAGIC_NUMBER)
    //return false;

  //Process::SysProc new_process = Process::SysProc(name, bin, getNewPid());

  //if ( (new_process.TextSection == 0) || (new_process.DataSection == 0) )
    //return false;

  //kfree(bin);

  return true;
}

#endif

// temporariamente iremos ter apenas tarefas do kernel



class KernelProcess {
  public:
    void* callback;
    u128 last_cycles_counter;
    Registers regs; 

    KernelProcess(void* callback) {
      this->callback = callback;
    }
};

KernelProcess* actual_ktask;
Memory::Vector<KernelProcess> kTasks;


#define SAVE_ALL_REGISTERS() asm volatile (  \
    "pushq %rax;"                          \
    "pushq %rbx;"                          \
    "pushq %rcx;"                          \
    "pushq %rdx;"                          \
    "pushq %rsi;"                          \
    "pushq %rdi;"                          \
    "pushq %rbp;"                          \
    "pushq %r8;"                           \
    "pushq %r9;"                           \
    "pushq %r10;"                          \
    "pushq %r11;"                          \
    "pushq %r12;"                          \
    "pushq %r13;"                          \
    "pushq %r14;"                          \
    "pushq %r15;"                          \
    )


#define RESTORE_ALL_REGISTERS() asm volatile ( \
    "popq %r15;"                            \
    "popq %r14;"                            \
    "popq %r13;"                            \
    "popq %r12;"                            \
    "popq %r11;"                            \
    "popq %r10;"                            \
    "popq %r9;"                             \
    "popq %r8;"                             \
    "popq %rbp;"                            \
    "popq %rdi;"                            \
    "popq %rsi;"                            \
    "popq %rdx;"                            \
    "popq %rcx;"                            \
    "popq %rbx;"                            \
    "popq %rax;"                            \
    :                                       \
    :                                       \
    : "r15", "r14", "r13", "r12", "r11", "r10", "r9", "r8", "rbp", "rdi", "rsi", "rdx", "rcx", "rbx", "rax" \
)

#define LOAD_GP_REGISTERS_FROM_TASK() asm volatile ( \
    "movq %0, %%rax;"                        \
    "movq %1, %%rbx;"                        \
    "movq %2, %%rcx;"                        \
    "movq %3, %%rdx;"                        \
    "movq %4, %%rsi;"                        \
    "movq %5, %%rdi;"                        \
    "movq %6, %%rbp;"                        \
    "movq %7, %%r8;"                         \
    "movq %8, %%r9;"                         \
    "movq %9, %%r10;"                        \
    "movq %10, %%r11;"                       \
    "movq %11, %%r12;"                       \
    "movq %12, %%r13;"                       \
    "movq %13, %%r14;"                       \
    "movq %14, %%r15;"                       \
    :                                       \
    : "m"(actual_ktask->regs.rax), "m"(actual_ktask->regs.rbx), "m"(actual_ktask->regs.rcx), "m"(actual_ktask->regs.rdx), \
      "m"(actual_ktask->regs.rsi), "m"(actual_ktask->regs.rdi), "m"(actual_ktask->regs.rbp), "m"(actual_ktask->regs.r8),  \
      "m"(actual_ktask->regs.r9), "m"(actual_ktask->regs.r10), "m"(actual_ktask->regs.r11), "m"(actual_ktask->regs.r12), \
      "m"(actual_ktask->regs.r13), "m"(actual_ktask->regs.r14), "m"(actual_ktask->regs.r15)                            \
    : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "rbp", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "memory"  \
)

#define STORE_GP_REGISTERS_IN_TASK() asm volatile ( \
    "movq %%rax, %0;" \
    "movq %%rbx, %1;" \
    "movq %%rcx, %2;" \
    "movq %%rdx, %3;" \
    "movq %%rsi, %4;" \
    "movq %%rdi, %5;" \
    "movq %%rbp, %6;" \
    "movq %%r8,  %7;" \
    "movq %%r9,  %8;" \
    "movq %%r10, %9;" \
    "movq %%r11, %10;" \
    "movq %%r12, %11;" \
    "movq %%r13, %12;" \
    "movq %%r14, %13;" \
    "movq %%r15, %14;" \
    : "=m"(actual_ktask->regs.rax), "=m"(actual_ktask->regs.rbx), "=m"(actual_ktask->regs.rcx), "=m"(actual_ktask->regs.rdx), \
      "=m"(actual_ktask->regs.rsi), "=m"(actual_ktask->regs.rdi), "=m"(actual_ktask->regs.rbp), "=m"(actual_ktask->regs.r8), \
      "=m"(actual_ktask->regs.r9), "=m"(actual_ktask->regs.r10), "=m"(actual_ktask->regs.r11), "=m"(actual_ktask->regs.r12), \
      "=m"(actual_ktask->regs.r13), "=m"(actual_ktask->regs.r14), "=m"(actual_ktask->regs.r15) \
    : \
    : "memory" \
)

void __attribute__((interrupt)) Scheduler(TimerStack *stack) {
  SAVE_ALL_REGISTERS();
  actual_ktask->regs.rip = stack->rip;
  actual_ktask->regs.cs = stack->cs;
  actual_ktask->regs.rflags = stack->rflags;
  actual_ktask->regs.rsp = stack->rsp;
  actual_ktask->regs.ss = stack->ss;
  STORE_GP_REGISTERS_IN_TASK();

  for(int i = 0; i < 1; i++) {

    unsigned short divisor = 1193180 / 50; // 20ms
    outb(0x43, 0x36);
    unsigned char l = (unsigned char)(divisor & 0xFF);
    unsigned char h = (unsigned char)((divisor >> 8)&0xFF);
    outb(0x40, l);
    outb(0x40, h);
    actual_ktask = &kTasks[0];

    LOAD_GP_REGISTERS_FROM_TASK();
    stack->rip = actual_ktask->regs.rip;

  }
}


bool CreateKernelProcess(void* callback) {
  if(callback == nullptr) {
    throw_panic(0, "Fatal: Kernel Task address is null");
  }
  kTasks.append(KernelProcess(callback));
  return true;
}

