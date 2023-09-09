#pragma once

#ifndef _HAL
#define _HAL

#include "../Utils/Base.h"
#include "../Memory/Base_Mem.h"
#include "../Drivers/VIDEO/preload.h"

#include "../Interruptions/preload.h"

#include "../Core/Debugging/Symbols.h"
#include "../Drivers/Keyboard.h"

#define PORT (unsigned short)0x3f8 /* COM1 register, for control */

struct IDTEntry16 {
  u16 offset_low;
  u16 selector = 0x08;
  u8 ist;
  u8 flags;
  u16 offset_middle;
  u32 offset_high;
  u32 reserved;
} __attribute__((packed));

alignas(4096) u64 IDT_entries[256*2];
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
          Symbols::Symbol* sym = Symbols::sym_lookup((void*)ret_addr);
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


      bool append_idt(u64 addr, u32 offset) {
        if(addr == 0) {
          throw_panic(0, "Invalid ISR address");
        }

        dbg("append_idt addr: %p offset: %d", (void*)addr, offset);

        u64 low_bits = ((u64)0x08 << 16); // seletor
        low_bits |= (addr & 0xFFFF);
        low_bits |= ((u64)0x8E << 40); // flags

        u64 high_bits = ((addr >> 16) & 0xFFFF);
        high_bits |= ((addr>>32) & 0xFFFFFFFFULL) << 32;


        u64* idt_entry = &IDT_entries[offset * 2];
        *idt_entry = low_bits;
        *(idt_entry + 1) = high_bits;
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
        } __attribute__((packed)) IDTR = { 256, (u64)IDT_entries};
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

        outb(PIC1_DATA, /*PIC_ICW4*/0x01);
        outb(PIC2_DATA, /*PIC_ICW4*/0x01);

        // Desmascarando interrupção do teclado e timer apenas
      
        outb(PIC1_DATA, 0xFD); // desmascara teclado
        outb(PIC2_DATA, 0xFF); // mascara tudo da PIC2
        

        return true;
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
