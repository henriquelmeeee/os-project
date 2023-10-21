#pragma once

#include "../Utils/Base.h"
#include "../Memory/Memory.h"
#include "../Drivers/VIDEO/preload.h"

#include "../Interruptions/preload.h"
#include "../Cores/Debugging/Symbols.h"
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

namespace HAL {
  class System {
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

        Text::Writeln("Trying to create kernel pages");
        change_to_kernel_addr_space();

        Text::Writeln("Trying to initialize the PIC");
        this->pic = PIC();
        this->pic.initialize();

        Text::Writeln("Trying to initialize the APIC");
        this->apic = APIC();
        this->apic.initialize();

        Text::Writeln("Trying to initialize syscalls");
        initialize_syscalls();
      }

      bool change_to_kernel_addr_space();
      void initialize_syscalls();

      void write_cr3(u64 value);
      u64 read_cr3();


  };
}

extern HAL::System system;
