// Processes starts in 100MB address (100.000.000)
#pragma once

#ifndef PROCESS__
#define PROCESS__

#include "../Filesystem/Filesystem.h"
#include "../Memory/Base_Mem.h"
#include "../Utils/Base.h"
#include "../Memory/Heap/Heap.h"
#include "../Drivers/VIDEO/preload.h"
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

struct Registers {
  // General Purpose Registers
  unsigned long long rax;
  unsigned long long rbx;
  unsigned long long rcx;
  unsigned long long rdx;
  unsigned long long rsi;
  unsigned long long rdi;
  unsigned long long rbp;
  unsigned long long rsp;
  unsigned long long r8;
  unsigned long long r9;
  unsigned long long r10;
  unsigned long long r11;
  unsigned long long r12;
  unsigned long long r13;
  unsigned long long r14;
  unsigned long long r15;
  
  // Segment Registers
  unsigned short cs;
  unsigned short ds;
  unsigned short es;
  unsigned short fs;
  unsigned short gs;
  unsigned short ss;
  
  // Pointer and Index Registers
  unsigned long long rip;
  unsigned long long rflags;
  
  // Control Registers
  unsigned long long cr0;
  unsigned long long cr2;
  unsigned long long cr3;
  unsigned long long cr4;
  
  // Debug Registers
  unsigned long long dr0;
  unsigned long long dr1;
  unsigned long long dr2;
  unsigned long long dr3;
  unsigned long long dr6;
  unsigned long long dr7;
};

struct TimerStack {
  unsigned long long rip, cs, rflags, rsp, ss;
};

void __attribute__((interrupt)) Scheduler(TimerStack *stack);
bool CreateKernelProcess(void* callback);

#endif
