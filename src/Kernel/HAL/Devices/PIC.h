#pragma once

#include "../../Utils/Base.h"
#include "../../Drivers/VIDEO/preload.h"
#include "../../Interruptions/preload.h"

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

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1
#define PIC_EOI      0x20
#define PIC_INIT     0x11
#define PIC_ICW4     0x01 // Interrupt control word

class PIC {
  private:


  public:
    PIC() {
      kprintf("PIC: Initializing...");
      __initialize_idt();
    }

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
      entry->flags = flags;

      entry->ist = 0;

      return true;

    }
  private:
    inline void __initialize_idt() {
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
    }
};

