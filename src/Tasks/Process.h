// Processes starts in 100MB address (100.000.000)
#pragma once

#ifndef PROCESS__
#define PROCESS__

#include "../Filesystem/Filesystem.h"
#include "../Memory/Base_Mem.h"
#include "../Utils/Base.h"
#include "../Memory/Heap/Heap.h"
#include "../Drivers/VIDEO/preload.h"

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
