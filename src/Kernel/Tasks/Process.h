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

    //u128 m_cycles_started;
    //u128 m_cycles_finished;

    Memory::Vector<Region> m_regions;

    void build_stack(void* stack_addr_base);
    Process(char* name);
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
