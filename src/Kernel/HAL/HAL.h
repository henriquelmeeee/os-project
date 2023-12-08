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

typedef struct {
    u16 limit_low;            // Limite 15:0
    u16 base_low;             // Base 15:0
    u8 base_middle;           // Base 23:16
    u8 access;                // Tipo de acesso
    u8 limit_high : 4;        // Limite 19:16
    u8 flags : 4;             // Flags
    u8 base_high;             // Base 31:24
    u32 base_upper;           // Base 63:32
    u32 reserved;             // Reservado
} __attribute__((packed, aligned(8))) gdt_descriptor;

typedef struct {
    u16 size;                 // Tamanho da GDT - 1
    u64 address;              // Endereço linear da GDT
} __attribute__((packed)) gdt_pointer;

static void encode_gdt_entry(gdt_descriptor *target, u64 base, u32 limit, u8 access, u8 flags) {
    // Inicialize a entrada com zeros
    *target = (gdt_descriptor){0};

    // Limite (tamanho do segmento)
    target->limit_low = limit & 0xFFFF;
    target->limit_high = (limit >> 16) & 0x0F;
    
    // Base (início do segmento)
    target->base_low = base & 0xFFFF;
    target->base_middle = (base >> 16) & 0xFF;
    target->base_high = (base >> 24) & 0xFF;
    target->base_upper = (u32)(base >> 32);
    
    // Definindo acesso e flags
    target->access = access;
    target->flags = flags & 0x0F;
    
    // Reservado deve ser zero
    target->reserved = 0;
}

#define GDT_ENTRY_COUNT 5

static gdt_descriptor gdt[GDT_ENTRY_COUNT];
static gdt_pointer gdtr;

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
        __asm__ volatile("hlt");
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

        //configure_gdt_with_tss();
      }

      void configure_gdt_with_tss();

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
