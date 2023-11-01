#pragma once

#include "../Utils/Base.h"
#include "../Memory/Memory.h"
#include "../Drivers/VIDEO/preload.h"

#include "../Interruptions/preload.h"
//#include "../../Cores/Debugging/Symbols.h"
#include "../Drivers/Keyboard.h"
#include "../Syscalls/Syscall.h"

#include "Devices/PIC.h"
#include "Devices/APIC.h"
#include "Devices/Processor.h"

extern u64 kPML4[512];
extern u64 kPDPT[512];
extern u64 kPD[512];
extern u64 kPT[512][512];

#define KERNEL_START 10485760

extern "C" [[noreturn]] volatile void ring3_entry();

namespace HAL {
  class System {
    private:
      bool kernel_pagination_already_created = false;
      void* __kernel_pml4 = nullptr;
    public:
      PIC pic = {};
      APIC apic = {};
      Memory::Vector<Processor*> processors;
      Processor* bsp = nullptr;

      System() {
        Text::NewLine();
        Text::Writeln("HAL: Starting devices");
        processors.append(new Processor(true, 0));
        this->bsp = processors[0];

        Text::Writeln("Creating kernel pages");
        change_to_kernel_addr_space();

        Text::Writeln("Initializing the PIC");
        this->pic = PIC();
        this->pic.initialize();

        Text::Writeln("Initializing the APIC");
        this->apic = APIC();
        this->apic.initialize();

        Text::Writeln("Initializing syscalls");
        initialize_syscalls();
      }

      bool change_to_kernel_addr_space();
      void initialize_syscalls();

      void write_cr3(u64 value);
      u64 read_cr3();

      bool init_serial_for_dbg() {return true;} // TODO
      void change_ring(int ring) {
      
        asm volatile(
          "pushq $0x23\n"               // Seletor de segmento de Ring 3
          "pushq $999\n"                // Endereço de pilha de Ring 3
          "pushfq\n"                    // Flags
          "pushq $0x1B\n"               // Seletor de segmento de código de Ring 3
          "pushq %0\n"        
          "iretq\n"
          :
          : "r" (ring3_entry)
          : "memory"
        );
      }
  };
}

extern HAL::System system;
