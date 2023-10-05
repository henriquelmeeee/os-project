// Processes starts in 100MB address (100.000.000)
#pragma once

#ifndef PROCESS__
#define PROCESS__

#include "../Filesystem/Filesystem.h"
#include "../Memory/Memory.h"
#include "../Utils/Base.h"
#include "../Memory/Heap/Heap.h"
#include "../Drivers/VIDEO/preload.h"

#include "../kstd/stdlib.h"
#include "ElfImage.h"
#include "../Filesystem/Filesystem.h"

struct TSS64 {
  u32 reserved0;
  u64 rsp0;
  u64 rsp1;
  u64 rsp2;
  u64 reserved1;
  u64 ist1;
  u64 ist2;
  u64 ist3;
  u64 ist4;
  u64 ist5;
  u64 ist6;
  u64 ist7;
  u64 reserved2;
  u16 reserved3;
  u16 iomap_base;
} __attribute__((packed));

struct Registers {
  u64 rax;
  u64 rdi;
  u64 rsi;
  u64 rdx;
  u64 rbx;
  u64 rcx;

  u64 rsp;
  u64 rbp;
  
  u64 r8;
  u64 r9;
  u64 r10;
  u64 r11;
  u64 r12;
  u64 r13;
  u64 r14;
  u64 r15;

  u64 rip;
  
  u32 cs;
  u32 ds;
  u32 es;
  u32 fs;
  u32 gs;
  u32 ss;
  
  u64 rflags;
  
  u64 cr0;
  u64 cr2;
  u64 cr3;
  u64 cr4;
  
  u64 dr0;
  u64 dr1;
  u64 dr2;
  u64 dr3;
  u64 dr6;
  u64 dr7;
} __attribute__((packed));

class Process {
  private:
  public:
    Registers m_regs = {0};
    u64 pml4[512];
    u64 pdpt[512];
    ElfImage m_elf_image;

    char* m_name;

    bool m_kernel_process;

    //TSS64 m_tss = {0};

    //u128 m_cycles_started;
    //u128 m_cycles_finished;

    Memory::Vector<Region> m_regions;

    void initialize_stack(void* rip_addr, void* rsp_addr) {
      // ...
    }

    void build_stack(void* addr) {
      m_regs.rsp = ((u64) kmalloc(1024)) + 512;
      u64 __init_rsp = m_regs.rsp;
      m_regs.rbp = m_regs.rsp;
      u64 cr0, cr2, cr3, cr4, rflags;
      asm volatile("mov %%cr0, %0" : "=r"(cr0));
      asm volatile("mov %%cr2, %0" : "=r"(cr2));
      asm volatile("mov %%cr3, %0" : "=r"(cr3));
      asm volatile("mov %%cr4, %0" : "=r"(cr4));
      asm volatile("pushfq; popq %0;" : "=r" (rflags));
      m_regs.cr0 = cr0;
      m_regs.cr2 = cr2;
      m_regs.cr3 = cr3;
      m_regs.cr4 = cr4;
      m_regs.rflags = rflags;
      
      // Precisamos colocar 128 bytes de dados
      // que são o conteúdo dos registradores
      // o último é o RSP e o penúltimo o RBP, que o timer-handler dará "pop"
      // antes disso, temos o Interrupt Frame que precisamos "simular"
      // pois a primeira vez que o processo for executado ele não terá uma interrupt frame
      // por padrão
      *((u64*)(m_regs.rsp)) = 0xDEADBEEF;
      m_regs.rsp+=(120);
      //*((u64*)(m_regs.rsp-16)) = m_regs.rsp;    // RBP inicial
      *((u64*)(m_regs.rsp-8)) = m_regs.rsp;   // RSP inicial
      
      // Agora, vamos terminar de construir a stack
      // simulando o interrupt frame
      // TODO SS e RSP parecem não fazer parte, provavelmente por conta daquela parada de
      // "se não mudou o nível de privilégio, não vale"
      

      //m_regs.rsp+=8;
      //*((u64*)m_regs.rsp) = 0xBEEFDEAD; // SS
      //m_regs.rsp+=8;
      //*((u64*)m_regs.rsp) = m_regs.rsp; // RSP
      dbg("process rsp: %p", (void*)m_regs.rsp);
      //m_regs.rsp+=8;
      *((u64*)m_regs.rsp) = (u64)addr; // rip
      m_regs.rsp+=8;
      *((u16*)m_regs.rsp) = 0x08; // cs
      m_regs.rsp+=8;
      *((u64*)m_regs.rsp) = rflags; // rflags
      m_regs.rsp += 8;
      *((u64*)m_regs.rsp) = m_regs.rsp; // rsp
      m_regs.rsp += 8;
      *((u64*)m_regs.rsp) = 0; // ss
      
      m_regs.rip = (u64)addr;
      m_regs.rsp = __init_rsp;
      return;

#if 0
      // Agora, precisamos criar uma pilha inicial
      // ela conterá todos registradores salvos em uma interrupção de timer
      // isso é importante porque nos baseamos no timer
      // para restaurar registradores.
      __builtin_memset(((char*)m_regs.rsp)-1024, 0, 1024);
      m_regs.rsp-=120; // 120 bytes é o tamanho total de todos registradores gerais que são salvos
      // agora precisamos salvar na seguinte ordem:
      // ss, rsp, rflags, cs, rip
      
      // Antes, vamos transformar o RBP salvo na stack como realmente a base
      *((u64*)m_regs.rsp - 8*8) = m_regs.rsp;
      u64 _ss = 0;
      u64 _rsp = m_regs.rsp+120;
      dbg("_rsp: %p", (void*)_rsp);
      u64 _rflags;
      u64 _cs = 0x08;
      u64 _rip = (u64)addr;
      asm volatile(
          "pushf\n"
          "pop %0\n"
          : "=r" (_rflags)
          :
          :
        );

      m_regs.rsp-=8;
      (* ((u64*)m_regs.rsp)) = _ss;
      m_regs.rsp-=8;
      (* ((u64*)m_regs.rsp)) = _rsp;
      m_regs.rsp-=8;
      (* ((u64*)m_regs.rsp)) = _rflags;
      m_regs.rsp-=8;
      (* ((u64*)m_regs.rsp)) = _cs;
      m_regs.rsp-=8;
      (* ((u64*)m_regs.rsp)) = _rip;

      // Agora, vamos voltar o ponteiro RSP do processo para a base a qual ignora o InterruptFrame
      m_regs.rsp+=8*5;
      m_regs.rbp = m_regs.rsp;
      return;
#endif
    }

    void kernel_routine_set(void* addr) {
      m_regs.rip = (u64) addr;
      return;
    }

    Process(char* name, bool kernel_process = false, void* addr=0) : m_name(name) {
      dbg("Processo %s criado", m_name);
      // Ignore "addr" se kernel_process == false!
      FILE* __elf_binary = g_fs->fopen("...");
      m_elf_image = ElfImage((void*)__elf_binary->m_raw_data);

      auto callback = MakeFunctor([&](Elf64_Phdr* current_phdr) {
        if(current_phdr->p_type != _ELF_PT_LOAD)
          return;
        dbg("current_phdr->p_vaddr: %p", (void*)current_phdr->p_vaddr);
        Region tmp = Region(this, current_phdr->p_vaddr);
        // Inicialmente iremos fazer apenas 1 página de alocação
        // para facilitar o map()
        // como temporariamente nós também usamos a mesma página física
        // para o p_vaddr
        // podemos usar ele de base para copiar os dados antes de mapear pro processo
        char* src_addr = (char*)(__elf_binary->m_raw_data) + current_phdr->p_offset;
        __builtin_memcpy((char*)current_phdr->p_vaddr, src_addr, current_phdr->p_filesz);
        tmp.map(); // mapeia para a área do processo
        m_regions.append(tmp);
        
          });
      m_elf_image.for_each_program_header(callback);
      dbg("finalizado");
      while(true);
      return;
      // Inicialmente, precisamos criar as regiões para o código e a stack
      // Essas regiões conterão uma lista de VMObjects
      // e o mapeamento para a memória física ocorrerá de forma transparente
      // A página virtual 500 do processo é onde a região de código começa
      // Já a página virtual 600 é onde a stack começa, em direção ao código
      // esse é um layout temporário.
      
      //build_stack(addr);
      //return;
      Region code_region      = Region(this, 0);
      Region stack_region     = Region(this, 0);

      // TODO precisamos carregar o binário do filesystem
      // e pegar tudo dele e colocar em diferentes páginas VMObject
      
      // temporariamente hard-coded

      short raw_data_test[] = {0x90, 0x90, 0x90, 0x90};
      VMObject* current_code_page = (VMObject*) kmalloc(sizeof(VMObject)); // TODO operador new()
      current_code_page->m_virtual_page = 500;
      current_code_page->m_physical_page = 600;
      // TODO memset() para a m_physical_page baseado em raw_data_test
      if(!current_code_page->map(this)) {
        throw_panic(0, "current_code_page->map(this) falhou");
      }

      m_regions.append(code_region);

      m_regions.append(stack_region);
      m_kernel_process = kernel_process;

#if 0
      // Agora, precisamos configurar o TSS: (ainda não é usada)

      TSS64* rsp0_chunk      = (TSS64*) kmalloc(512);
      //TSS64* rsp1_chunk      = (TSS64*) kmalloc(512);
      TSS64* rsp2_chunk      = (TSS64*) kmalloc(512);

      TSS64* ist1_chunk      = (TSS64*) kmalloc(512);
      TSS64* ist2_chunk      = (TSS64*) kmalloc(512);
      TSS64* ist3_chunk      = (TSS64*) kmalloc(512);
      TSS64* ist4_chunk      = (TSS64*) kmalloc(512);
      TSS64* ist5_chunk      = (TSS64*) kmalloc(512);
      TSS64* ist6_chunk      = (TSS64*) kmalloc(512);
      TSS64* ist7_chunk      = (TSS64*) kmalloc(512);

      // TODO salvar ponteiros para esses ist_chunk e rsp_chunk para kfree()


      m_tss.rsp0             = ((u64) (rsp0_chunk));
      //m_tss.rsp1             = ((u64) (rsp1_chunk)) + 512;
      m_tss.rsp2             = ((u64) (rsp2_chunk)) + 512;

      m_tss.ist1             = ((u64) (ist1_chunk)) + 512;
      m_tss.ist2             = ((u64) (ist2_chunk)) + 512;
      m_tss.ist3             = ((u64) (ist3_chunk)) + 512;
      m_tss.ist4             = ((u64) (ist4_chunk)) + 512;
      m_tss.ist5             = ((u64) (ist5_chunk)) + 512;
      m_tss.ist6             = ((u64) (ist6_chunk)) + 512;
      m_tss.ist7             = ((u64) (ist7_chunk)) + 512;
#endif
    }
};

extern Memory::Vector<Process> g_procs;
extern Memory::Vector<Process*> g_kernel_procs;
extern Process* g_current_proc;
extern u64 g_timer_temporary_stack;
#if 0
  enum state {
    RUNNING,
    PAUSED,
    ZOMBIE,
  };
namespace Process {
  class SysProc {
    public:
      char name[17]; // 17 pois precisamos incluir null byte
      u64 *ptr;
      u128 cycles_started;
      u128 cycles_finished;
      u32 pid;
      enum state p_state;

      alignas(4096) Memory::PhysicalRegion TextSection;
      alignas(4096) Memory::PhysicalRegion DataSection;
      alignas(4096) Memory::PhysicalRegion StackSection;
      
      alignas(4096) unsigned long PML4[5]; // 512GB each entry
      alignas(4096) unsigned long PDPT[512]; // 1GB each entry
      
      alignas(4096) unsigned long kPD[512]; // 2MB each entry
      alignas(4096) unsigned long textPD[512];

      alignas(4096) unsigned long kPT[512][512];
      alignas(4096) unsigned long textPT[512][512];

      FrameBuffer *framebuffer; // NEED kmalloc() if GUI initializes
   
      SysProc(const char *p_name, Binary* pbin, unsigned int pid);
  };
}
namespace Process {
  bool init();

  #define BINARY_MAGIC_NUMBER 0x5150
  extern enum state p_state;

  extern Memory::Vector<Process::SysProc> procs;
  extern Process::SysProc* proc_current;
  extern u64 amount_of_procs;

  bool CreateProcess(const char* name, u16 privilege);
}
// Every page has 2MB size
#endif

struct TimerStack {
  unsigned long long rip, cs, rflags, rsp, ss;
};

void __attribute__((interrupt)) Scheduler(TimerStack *stack);
bool CreateKernelProcess(void* callback);

#endif
