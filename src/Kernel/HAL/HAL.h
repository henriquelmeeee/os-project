#pragma once

#ifndef _HAL
#define _HAL

#include "../Utils/Base.h"
#include "../Memory/Base_Mem.h"
#include "../Drivers/VIDEO/preload.h"

#include "../Interruptions/preload.h"

#include "../Core/Debugging/Symbols.h"
#include "../Drivers/Keyboard.h"

#include "../Syscalls/Syscall.h"

#include "Devices/PIC.h"
#include "../Core/API/Preload.h"
#include "../Processor.h"

#define PORT (unsigned short)0x3f8 /* COM1 register, for control */

extern u64 kPML4[512];
extern u64 kPDPT[512];
extern u64 kPD[512];
extern u64 kPT[512][512];

//extern Memory::PML4Entry kPML4[512];
//extern Memory::PDPTEntry kPDPT[512];
//extern Memory::PDEntry kPD[512];
//extern Memory::PTEntry kPT[512][512];


#define KERNEL_START 10485760

  struct RSDP {
    char signature[8];
    char sum;
    char oem_id[6];
    char revision;
    unsigned int rsdt_addr;

    // ACPI 2.0+:
    u32 length;
    u64 xsdt_addr;
    u8 extended_checksum;
    u8 reserved[3];
  } __attribute__((packed));

  struct ACPI_STD_HEADER {
    char signature[4];
    u32 length;
    u8 revision;
    u8 checksum;
    char oem_id[6];
    char oem_table_id[8];
    u32 oem_revision;
    u32 creator_id;
    u32 creator_revision;
  } __attribute__((packed));

  struct RSDT {
    ACPI_STD_HEADER header;
    u32 entry[1]; // all of tables in system; check "header length"
  } __attribute__((packed));

  struct FADT{};
  struct MADT {
    char signature[4];
    u32 length;
    u8 revision;
    u8 checksum;
    char oem_id[6];
    char oem_table_id[8];
    u32 oem_revision;
    u32 creator_id;
    u32 creator_revision;
    u32 local_controller_addr; // Endereço físico do LAPIC!
    u32 flags;
  } __attribute__((packed));
  struct SSDT{};
  struct DSDT{};
  struct SBST{};
  struct ECDT{};




class ACPI {
  private:
    bool available=false;
    bool xsdt_in_use=false;
    u16 rsdt_entries = 0;

    RSDP rsdp; // root system descriptor pointer
    RSDT rsdt; // root system descriptor table
    FADT fadt; // fixed ACPI description table
    MADT madt; // multiple apic description table
    SSDT ssdt; // secondary system description table
    DSDT dsdt; // differentiated system description table
    SBST sbst; // smart battery specification table
    ECDT ecdt; // embedded controller boot resources table

  public:

    ACPI() {}

    bool enable() {
      for(char* i = (char*) BIOS_ENTRY; (unsigned long)i<BIOS_END; i+=16) {
        if(!kstrncmp(i, "RSD PTR ", 8)) {
          Text::Writeln("HAL: ACPI: Found Root System Descriptor", 0xe);
          RSDP* buf = (RSDP*)i;
          rsdp.rsdt_addr = buf->rsdt_addr;
          if(buf->revision > 0) {
            xsdt_in_use = true;
            rsdp.xsdt_addr = buf->xsdt_addr;
            dbg("XSDT found!");
            // currently we will just use RSDP.
          }

          RSDT* _buf = (RSDT*)(u64)buf->rsdt_addr;
          if(_buf == nullptr) {
            throw_panic(0, "Invalid RSDT pointer (null pointer is not valid)");
          }
          //rsdt.header = _buf->header; FIXME why panic?
          rsdt_entries = rsdt.header.length - sizeof(ACPI_STD_HEADER)/4;
          available=true;
          break;
        }
      }
      int entries = ((rsdt.header.length - sizeof(RSDT))) / 4;
      for(int i = 0; i<entries; ++i) {
        u32 entry_addr = rsdt.entry[i];
        MADT* __madt = (MADT*) entry_addr;

        if(kstrncmp(__madt->signature, "APIC", 4) == 0) {
          __builtin_memcpy((char*)__madt, (char*)&(this->madt), sizeof(MADT));
          dbg("HAL: MADT encontrada");
          processor->m_local_apic_addr = madt.local_controller_addr;
          break;
        }
      }
      return available;
    }
};

namespace HAL {
  class System {
    public:
      ACPI acpi;
      PIC pic;
      unsigned long long TOTAL_RAM;

      System() {
        // Hard-coded temporarily FIXME
        Text::NewLine();
        Text::Writeln("HAL: Looking for system information...", 0xe);
        this->TOTAL_RAM = 2 GB;
        unsigned long long TOTAL_RAM_BUFFER = this->TOTAL_RAM;

        Text::Writeln("HAL: Initializing ACPI", 0xe);
        acpi.enable();
        this->pic = PIC();
      }

      bool volatile __attribute__((noinline)) dump_stack() {
        u64* rbp;
        __asm__ volatile("mov %%rbp, %0" : "=r"(rbp));

        Text::NewLine();
        while(*rbp > KERNEL_START) {
          if(!rbp)
            break;
          u64* ret_addr = rbp;
          ret_addr+=1;
          u64 _ret_addr = (u64)ret_addr;
          Symbols::Symbol* sym = Symbols::sym_lookup((void*)_ret_addr);
          while(true) {
            sym = Symbols::sym_lookup((void*)_ret_addr);
            if(sym == nullptr) {
              --_ret_addr;
            } else {
              break;
            }
          }

          if(sym != nullptr) {
            Text::Write("Call @ ");
            Text::Writeln(sym->name);
          } else {
            char buffer[32];
          
            itoh(*(rbp+1), buffer);
            Text::Write("Call @0x");
            Text::Writeln(buffer);
          }
          rbp = (u64*)*rbp;
        }


        return true;
      }

      bool init_serial_for_dbg() {
        outb(PORT + 1, (char)0x00);    // Disable all interrupts
        outb(PORT + 3, (char)0x80);    // Enable DLAB (set baud rate divisor)
        outb(PORT + 0, (char)0x03);    // Set divisor to 3 (lo byte) 38400 baud
        outb(PORT + 1, (char)0x00);    //                  (hi byte)
        outb(PORT + 3, (char)0x03);    // 8 bits, no parity, one stop bit
        outb(PORT + 2, (char)0xC7);    // Enable FIFO, clear them, with 14-byte threshold
        outb(PORT + 4, (char)0x0B);    // IRQs enabled, RTS/DSR set

        return true;
      }

      bool m_kernel_addr_space_created = false;

      bool change_to_kernel_addr_space() {
        if(!m_kernel_addr_space_created) {
          for(int i = 0; i<512; i++) {
            kPML4[i] = {0};
            kPDPT[i] = {0};
            kPD[i] = {0};
          }

          if(reinterpret_cast<u64>(kPDPT) % 4096 != 0){
            throw_panic(0, "PDPT are not alligned");
          }
          if(reinterpret_cast<u64>(kPML4) % 4096 != 0) {
            throw_panic(0, "PML4 are not alligned");
          }
          if(reinterpret_cast<u64>(kPD) % 4096 != 0) {
            throw_panic(0, "PD are not alligned");
          }
          if(reinterpret_cast<u64>(kPT) % 4096 != 0) {
            throw_panic(0, "PT are not alligned");
          }

          for(int pml4e = 0; pml4e<512; pml4e++) {
            kPML4[pml4e] = ((reinterpret_cast<u64>(&(kPDPT[pml4e])) & ~0xFFF) | 0x3);
          }

          for(int pdpte = 0; pdpte<512; pdpte++) {
            kPDPT[pdpte] = ((reinterpret_cast<u64>(&(kPD[pdpte])) & ~0xFFF) | 0x3);
          }

          int current_page = 0;
          for(int pde = 0; pde<512; pde++) {
            for(int pte = 0; pte<512; pte++) {
              kPT[pde][pte] = (current_page * 4096) | 0x3;
              current_page++;
            }
            kPD[pde] = ((reinterpret_cast<u64>(&(kPT[pde])) & ~0xFFF) | 0x3);
          }
        
          //ASSERT(kPT[0][0].flag_bits.present == 1,   -ENOPT,    "Failed to create PT");
          //ASSERT(kPD[0].flag_bits.present    == 1,   -ENOPD,    "Failed to create PD");
          //ASSERT(kPDPT[0].flag_bits.present  == 1,   -ENOPDPT,  "Failed to create PDPT");
          //ASSERT(kPML4[0].flag_bits.present  == 1,   -ENOPML4,  "Failed to create PML4");


          __asm__ volatile(
              "mov %0, %%cr3;"
              :
              : "r" (kPML4)
              : "rax", "memory"
            );
          m_kernel_addr_space_created = true;
        } else {
          // TODO mudar c0 apenas
        }
        return true;
      }

      /*
       O trap frame é o principal tipo de frame usado para interrupções
       principalmente envolvendo o timer

       struct TrapFrame {
        uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
        uint64_t rdi, rsi, rbp, rbx, rdx, rcx, rax;
        uint64_t int_num, err_code;
        uint64_t rip, cs, rflags, rsp, ss;
      };

      "rsp" e "ss" não estarão obrigatoriamente na stack, mas não há problema
      porque eles são os últimos

      */

      void initialize_syscalls() {
        // Don't care about syscall initialization, its just about MSR write
        __asm__ volatile(
            "wrmsr"
            :
            : "c" (0xC0000081), "a" (0x08ULL << 3), "d" (0x10ULL << 3)
            :
          );

        u64 __syscall_handler_pointer = (u64)syscall_handler_stub;

        __asm__ volatile(
          "wrmsr" 
          : 
          : "c" (0xC0000082), "a" (__syscall_handler_pointer & 0xFFFFFFFF), "d" (__syscall_handler_pointer >> 32)
          :
        );

        u32 __low, __high;
        asm volatile("rdmsr" : "=a"(__low), "=d"(__high) : "c"(0xC0000080));
        u64 efer = ((u64)__high << 32) | __low;

        efer |= 1; // syscall enable
        __low = (u32)efer;
        __high = (u32)(efer >> 32);
        asm volatile("wrmsr" : : "a"(__low), "d"(__high), "c"(0xC0000080));
      }

      void DoAutomatedTests() {
        auto* any_chunk = kmalloc(32);
        if(any_chunk == 0) {
          throw_panic(0, "Test not passed: kmalloc() returned nullptr");
        }
        //kfree(any_chunk);
      }


  };
};

extern HAL::System system;

#endif
