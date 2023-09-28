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

#define PORT (unsigned short)0x3f8 /* COM1 register, for control */

#if 0
extern u64 kPML4[512];
extern u64 kPDPT[512];
extern u64 kPD[512];
extern u64 kPT[512][512];
#endif

extern Memory::PML4Entry kPML4[512];
extern Memory::PDPTEntry kPDPT[512];
extern Memory::PDEntry kPD[512];
extern Memory::PTEntry kPT[512][512];
struct IDTEntry16 {
  u16 offset_low;
  u16 selector = 0x08;
  u8 ist;
  u8 flags;
  u16 offset_middle;
  u32 offset_high;
  u32 reserved;
} __attribute__((packed));

extern IDTEntry16 IDT_entries[256];

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
  };

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
  };

  struct RSDT {
    ACPI_STD_HEADER header;
    u32 entry[1]; // all of tables in system; check "header length"
  };

  struct FADT{};
  struct MADT{};
  struct SSDT{};
  struct DSDT{};
  struct SBST{};
  struct ECDT{};


#define BIOS_ENTRY 0xE0000
#define BIOS_END 0xFFFFF

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
      
      return available;
    }
};

namespace HAL {
  class System {
    private:
      ACPI acpi;
    public:
      unsigned long long TOTAL_RAM;

      System() {
        // Hard-coded temporarily FIXME
        Text::NewLine();
        Text::Writeln("HAL: Looking for system information...", 0xe);
        this->TOTAL_RAM = 2 GB;
        unsigned long long TOTAL_RAM_BUFFER = this->TOTAL_RAM;
        char buffer[32];
        Text::Write("HAL: ", 0xe);
        

        if(TOTAL_RAM_BUFFER < 1024) {
          itos(TOTAL_RAM_BUFFER, buffer);
          Text::Write(buffer);
          Text::Write("B");
        } else if ((TOTAL_RAM_BUFFER >= 1024) && (TOTAL_RAM_BUFFER < 1000000)) {
          itos(TOTAL_RAM_BUFFER / 1024, buffer);
          Text::Write(buffer);
          Text::Write("KB");
        } else {
          itos(TOTAL_RAM_BUFFER / 1000000, buffer);
          Text::Write(buffer);
          Text::Write("MB");
        }
        Text::Writeln(" of RAM found", 0xe);

        Text::Writeln("HAL: Now trying to initialize ACPI", 0xe);
        acpi.enable();
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

      bool change_to_kernel_addr_space() {
#if 0
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
#endif
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

      bool append_idt(u64 addr, u32 offset, u32 flags = 0x8E) {
        if(addr == 0) {
          throw_panic(0, "Invalid ISR address");
        }

        dbg("append_idt addr: %p offset: %d", (void*)addr, offset);

        IDTEntry16 *entry = &(IDT_entries[offset]);

        entry->offset_low = addr & 0xFFFF;
        entry->offset_middle = (addr >> 16) & 0xFFFF;
        entry->offset_high = (addr >> 32) & 0xFFFFFFFF;

        entry->selector = 0x08;
        entry->flags = 0x8E;

        entry->ist = 0;

#if 0
        u32 idx = offset * 2;
        IDT_entries[idx] = low_bits;
        IDT_entries[idx + 1] = high_bits;
#endif
#if 0
        u16 selector = 0x08;
        u8 flags = 0x8E;
        u128 entry = 
          (u64)(addr & 0xFFFF) | 
          ((u64)selector << 16) | 
          ((u64)flags << 32) | 
          ((addr & 0xFFFF0000) << 32) | 
          ((u64)(addr >> 32) << 48);
        struct IDTEntry16* entry = &(IDT_entries[offset]);
        entry->selector = 0x08;
        entry->offset_low = (u16)(addr & 0xFFFF);
        entry->ist = 0;
        entry->flags = 0x8E;
        entry->offset_middle = (u16)((addr >> 16) & 0xFFFF);
        entry->offset_high = (u32)((addr >> 32) & 0xFFFFFFFF);
        entry->reserved = 0;

#endif


        dbg("addr de Drivers::Keyboard::keyboard_interrupt_key = %p\naddr enviado = %p", (void*)Drivers::Keyboard::keyboard_interrupt_key, (void*)addr);

        return true;
      }

      bool init_idt() {
        __builtin_memset(IDT_entries, 0, sizeof(IDT_entries));

        for(int i = 0; i<255; i++)
          append_idt(reinterpret_cast<u64>(i_spurious), i);

        volatile struct {
          u16 length;
          u64 base;
        } __attribute__((packed)) IDTR = { 4095, (u64)IDT_entries};
        __asm__ volatile (
            "lidt %0"
            :
            : "m" (IDTR) // "lidt &IDTR"
            );

        Text::Writeln("HAL: IDT initialized", 0xe);

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1
#define PIC_EOI      0x20
#define PIC_INIT     0x11
#define PIC_ICW4     0x01 // Interrupt control word

        outb(PIC1_COMMAND, PIC_INIT);
        outb(PIC2_COMMAND, PIC_INIT);
    
        // ICW2: Definir o vetor de início
        outb(PIC1_DATA, 32);   // Definir o vetor de interrupção inicial do PIC1 para 32 (0x20)
        outb(PIC2_DATA, 40);    // Definir PIC2 para ser acionado pela linha IRQ2 do PIC1 (40)
      
        // ICW3: Informar ao PIC1 que o PIC2 está em IRQ2 e informar ao PIC2 seu número de cascata
        outb(PIC1_DATA, 0x04); 
        outb(PIC2_DATA, 0x02);

        outb(PIC2_DATA, /*PIC_ICW4*/0x01);
        outb(PIC1_DATA, /*PIC_ICW4*/0x01);

        // Desmascarando interrupção do teclado e timer apenas
      
        //outb(PIC1_DATA, 0xFD); // desmascara teclado
        //outb(PIC2_DATA, 0xFF); // mascara tudo da PIC2
        
        outb(PIC1_DATA, 0);
        outb(PIC2_DATA, 0);

        outb(PIC1_DATA, 0xFE); // desmascara apenas o TIMER
        outb(PIC2_DATA, 0xFF);

        return true;
      }

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
