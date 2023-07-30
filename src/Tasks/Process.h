// Processes starts in 100MB address (100.000.000)
#ifndef PROCESS
#define PROCESS
#include "../Utils/Base.h"
#include "../Memory/Heap/Heap.h"
#include "../Memory/Base_Mem.h"
#include "../Drivers/VIDEO/preload.h"
#include "../Filesystem/Filesystem.h"
namespace Process {
  bool init();

  #define BINARY_MAGIC_NUMBER 0x5150

  enum state {
    RUNNING,
    PAUSED,
    ZOMBIE,
  };

  extern enum state p_state;
  class SysProc {
    public:
      char name[17]; // 17 pois precisamos incluir null byte
      u64 *ptr;
      u128 cycles_started;
      u128 cycles_finished;
      u32 pid;
      enum state p_state;

      void* TextSection;
      void* DataSection;
      void* StackSection;

      struct PT pt;
      struct PD pd;
      struct PDPT pdpt;
      struct PML4 pml4;

      FrameBuffer *framebuffer; // NEED kmalloc() if GUI initializes
   
      SysProc(const char *p_name, FS::Binary* pbin, unsigned int pid);
  };

  extern Memory::Vector<SysProc> procs;
  extern SysProc* proc_current;
  extern u64 amount_of_procs;

  bool CreateProcess(const char* name, u16 privilege);
}
// Every page has 2MB size


#endif
