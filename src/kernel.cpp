extern "C" void kmain();
#define KERNEL_SIZE 26112
// Drivers:
//#include "Drivers/Keyboard.h"
#include "Drivers/VIDEO/preload.h"
#include "Drivers/Disk.h"
//#include "Drivers/Mouse.h"

#include "Filesystem/Filesystem.h"

#include "DefaultConfig.h"
#include "Utils/Base.h"
#include "Utils/Errors.h"

#include "Memory/Base_Mem.h"
#include "Memory/Heap/Heap.h"

#include "Tasks/Process.h"
#include "panic.h"

// Interrupções:
#include "Interruptions/preload.h"
#include "Drivers/Keyboard.h"


unsigned long long mem_usage = 0;

extern "C" void kentrypoint() {kmain();}

volatile void sleep(unsigned long long ticks) {
  for(unsigned long long i = 0; i<ticks; i++) {for (volatile int j = 0; j<1000000; j++){}}
}

struct HardwareInformation {
  unsigned int ram;
};
// Bootloader carrega a informação da quantidade de RAM no endereço "15000"
// então nós devemos recuperar o valor nesse endereço 

unsigned long long TOTAL_RAM = 0;

unsigned long *pages_in_use;

// IDT -> configurações gerais

struct IDT_entry { // Interruption Gate
  u16 base_lo; // endereço mais baixo da ISR (deslocamento baseado no segmento)
  u16 sel; // seletor de segmento do kernel
  unsigned char always0;
  unsigned char flags;
  u16 base_mid;
  u32 base_hi;
  u32 reserved; // FIXME u32 or unsigned char or char?
} __attribute__((packed));

struct IDT_ptr {
  unsigned short limit;
  struct IDT_entry* base;
} __attribute__((packed));

struct IDT_entry IDT_entries[256]; // aloca elementos para IDT

void halt() {
  __asm__ volatile(
      "hlt"
      );
}

void disable_irq(int irq) {
    unsigned short port;
    unsigned char value;

    if(irq < 8) {
        port = 0x21;
    } else {
        port = 0xA1;
        irq -= 8;
    }
    value = inb(port) | (1 << irq);
    outb(port, value);        
}

namespace Initialize {
  namespace FirstStage {
  bool init_idt_entries() {
  for(int i = 0; i<sizeof(IDT_entries) / sizeof(IDT_entries[0]); i++) {
    #ifndef IDT_entries_amount
      #define IDT_entries_amount 256
    #endif

    if(i+1 > IDT_entries_amount)
      break;
    

    switch (i) {
      case 16: { // FPU-x87 error
        u64 fpu_err_addr = reinterpret_cast<u64>(i_fpuerr);
        IDT_entries[i] = (struct IDT_entry){
          .base_lo = static_cast<u16>(fpu_err_addr & (u64) 0xFFFF),
          .sel = (unsigned short)0x08,
          .always0 = (char)0, // interrupt stack table
          .flags = (unsigned char)0x8E,
          .base_mid = static_cast<u16>((fpu_err_addr>>16) & (u64)0xFFFF),
          .base_hi = static_cast<u32>(fpu_err_addr>>32),
          .reserved = (u32)0, 
        };
      };
      case 502: { //14+32 disco      
        break;
      };
      case 501: {//15+32 disco também
        break;
      }
      /*case 500:{ // 12+32 IRQ12 = moue
        u64 mouse_interrupt_addr = reinterpret_cast<u64>(Drivers::Mouse::mouse_interrupt);
        IDT_entries[i] = (struct IDT_entry){
          .base_lo = static_cast<u16>(mouse_interrupt_addr & (u64) 0xFFFF),
          .sel = 0x08,
          .always0 = 0,
          .flags = 0x8E,
          .base_mid = static_cast<u16>((mouse_interrupt_addr>>16) & (u64)0xFFFF),
          .base_hi = static_cast<u32>(mouse_interrupt_addr>>32),
          .reserved = (u32)0,
        };
      };*/
      case 33:{ // 33 IRQ1 - Teclado
        u64 keyboard_interrupt_addr = reinterpret_cast<u64>(Drivers::Keyboard::keyboard_interrupt_key);
        IDT_entries[i] = (struct IDT_entry){
          .base_lo = static_cast<u16>(keyboard_interrupt_addr & (u64) 0xFFFF),
          .sel = 0x08,
          .always0 = 0,
          .flags = 0x8E,
          .base_mid = static_cast<u16>((keyboard_interrupt_addr>>16) & (u64)0xFFFF),
          .base_hi = static_cast<u32>(keyboard_interrupt_addr>>32),
          .reserved = (u32)0,
        };
        break;}
      default:
        u64 i_spurious_addr = reinterpret_cast<u64>(i_spurious);
        IDT_entries[i] = (struct IDT_entry){
          .base_lo = static_cast<u16>(i_spurious_addr & (u64) 0xFFFF),
          .sel = 0x08,
          .always0 = 0,
          .flags = 0x8E,
          .base_mid = static_cast<u16>((i_spurious_addr>>16) & (u64)0xFFFF),
          .base_hi = static_cast<u32>(i_spurious_addr>>32),
          .reserved = (u32)0,
        };
        break;
    };
  }
  return true;
}

    bool init_idt_ptr(u16 size) {
      // Inicializa os descritores da IDT
      volatile struct {
        u16 length;
        u64 base;
      } __attribute__((packed)) IDTR = { size, (u64)IDT_entries};
      __asm__ volatile (
          "lidt %0"
          :
          : "m" (IDTR) // "lidt &IDTR"
          );
      dbg("Initialize::FirstStage::init_idt_ptr()-> finalizado\n");
      return true;
    }
    
    bool init_pages_in_use() { // aloca uma chunk para a array de páginas em uso na Heap
      pages_in_use = (unsigned long*)kmalloc(PAGE_SIZE);
      return true;
    }

    bool init_idt_config() {
      #define PIC1_COMMAND 0x20
      #define PIC1_DATA    0x21
      #define PIC2_COMMAND 0xA0
      #define PIC2_DATA    0xA1
      #define PIC_EOI      0x20
      #define PIC_INIT     0x11
      #define PIC_ICW4     0x01
      //
      /*outb(PIC1_COMMAND, PIC_INIT);
      outb(PIC2_COMMAND, PIC_INIT);
    
      outb(PIC1_DATA, 32);   // Definir o vetor de interrupção inicial do PIC1 para 32 (0x20)
      outb(PIC1_DATA, 96);    // Definir PIC2 para ser acionado pela linha IRQ2 do PIC1 (96)
      outb(PIC1_DATA, PIC_ICW4);

      outb(PIC1_DATA, 0x04);   // Definir o vetor de interrupção inicial do PIC2 para 40 (0x28)
      outb(PIC2_DATA, 0x02);    // Definir PIC2 para ser acionado pela linha IRQ2 do PIC1

      outb(PIC2_DATA, PIC_ICW4);
      outb(PIC1_COMMAND, PIC_EOI);*/
      outb(0x20, 0x11);
      outb(0xA0, 0x11);
      outb(0x21, 0x21); // teclado
      outb(0xA1, 0x70);    /* start vector = 96 */

    /* Initialization of ICW3 */
      outb(0x21, 0x04);
      outb(0xA1, 0x02);

    /* Initialization of ICW4 */
      outb(0x21, 0x01);
      outb(0xA1, 0x01);

    /* mask interrupts */
      outb(PIC1_DATA, 0x0);
      outb(PIC1_DATA, (u8)(inb(PIC1_DATA) & ~(1 << 1)));
      outb(0xA1, 0x0);
      return true;
    }
    bool init() {
      TRY(init_idt_entries(), ErrorType{CRITICAL}, "An error occurred while trying to initialize the IDT");
      TRY(init_idt_ptr(sizeof(IDT_entries)), ErrorType{CRITICAL}, "An error occurred while trying to initialize the IDT (lidt)");
      TRY(init_idt_config(), ErrorType{CRITICAL}, "An error occurred while trying to configure the IDT");
      //TRY(init_pages_in_use(), ErrorType{CRITICAL});

      return true;
    }
  }
  namespace SecondStage {
    bool init_procs() {
      //if(!(Process::init())) 
        //return false;
      //if(!(Process::CreateProcess("initd", 0)))
      return true;
    return true;
        return false;
      return true;
    }
    bool init() {
      return true;
      //TRY(FS::map_files_on_boot(), ErrorType{CRITICAL}, "Critical file \"initd\" not found");
      // se não houver nenhum arquivo, então não há nem mesmo o processo "init"
      TRY(init_procs(), ErrorType{CRITICAL}, "Unable to initialize task system");
      return true;
    }
  }
}

#define fo (char)15

#define PORT (char)0x3f8 /* COM1 register, for control */

void init_serial() {
   outb(PORT + 1, (char)0x00);    // Disable all interrupts
   outb(PORT + 3, (char)0x80);    // Enable DLAB (set baud rate divisor)
   outb(PORT + 0, (char)0x03);    // Set divisor to 3 (lo byte) 38400 baud
   outb(PORT + 1, (char)0x00);    //                  (hi byte)
   outb(PORT + 3, (char)0x03);    // 8 bits, no parity, one stop bit
   outb(PORT + 2, (char)0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   outb(PORT + 4, (char)0x0B);    // IRQs enabled, RTS/DSR set
}


alignas(4096) Memory::PML4Entry kPML4[512];
alignas(4096) Memory::PDPTEntry kPDPT[512];
alignas(4096) Memory::PDEntry kPD[512];
alignas(4096) Memory::PTEntry kPT[512][512];

extern "C" void kmain() {
  CLI;
  mem_usage+=KERNEL_SIZE;
  TOTAL_RAM = 1 GB;
  
  init_serial();
  
  dbg("kmain()-> Kernel iniciando\n");


  #ifndef GRAPHICAL_MODE
    dbg("kmain()-> Modo de texto está habilitado\n");
    char* txtaddr = (char*)0xB8000;
    Text::text_clear();
    Text::Write("Kernel loaded");
    dbg("kmain()-> AVISO Modo de texto habilitado\n");
  #else
    dbg("kmain() -> Modo gráfico está habilitado\n");
    //Graphics::draw_window();
  #endif
  
  //TRY(Initialize::FirstStage::init(), ErrorType{CRITICAL}, "FirstStage init failed");
  CLI;
  //TRY(Initialize::SecondStage::init(), ErrorType{CRITICAL}, "SecondStage init failed");

  /*
    * Agora, nós precisamos configurar uma nova tabela de paginação base
    * essa tabela vai ser genérica, principalmente para conseguirmos usar
    * a região de memória mapeada para APIC
  */

  Memory::PML4Entry entry;
  
  char buffertmp[128];
  itos((u64)kPML4, buffertmp);
  dbg(buffertmp);

  for(int i = 0; i<512; i++) {
    kPDPT[i] = {0};
    kPML4[i] = {0};
    kPD[i] = {0};
  }

  entry.flags = 0; // limpa todas flags (define todas como 0)
  entry.flag_bits.present = 1;
  entry.flag_bits.read_write = 1;
  entry.flag_bits.user_supervisor = 1;
  for(int pml4e = 0; pml4e<512; pml4e++) {
    entry.flag_bits.physical_address = (reinterpret_cast<u64>(kPDPT)); // Inicialmente alocaremos 1 única PDPT
    kPML4[pml4e] = entry;
  }

  if(reinterpret_cast<u64>(kPDPT) % 4096 != 0){
    throw_panic(0, "PDPT are not alligned");}
  if(reinterpret_cast<u64>(kPML4) % 4096 != 0) {
    throw_panic(0, "PML4 are not alligned");}
  if(reinterpret_cast<u64>(kPD) % 4096 != 0) {
    throw_panic(0, "PD are not alligned");}
  if(reinterpret_cast<u64>(kPT) % 4096 != 0) {
    throw_panic(0, "PT are not alligned");}

  // iremos mapear as 450 primeiras páginas para a mesma página física
  // depois, usaremos o mapeamento para o endereço 0xFEE00 pois é onde está a APIC
 
  Memory::PDPTEntry PDPT_entry;

  PDPT_entry.flags = 0;
  PDPT_entry.flag_bits.present = 1;
  PDPT_entry.flag_bits.read_write = 1;
  PDPT_entry.flag_bits.user_supervisor = 1; // TODO FIXME isso é temporário para não termos erros de permissão
  for(int PDPTentry = 0; PDPTentry<512; PDPTentry++) {
    PDPT_entry.flag_bits.physical_address = (reinterpret_cast<u64>(&(kPD[PDPTentry])));
    kPDPT[PDPTentry] = PDPT_entry;
  }

  Memory::PDEntry PD_entry;
  PD_entry.flags = 0;
  PD_entry.flag_bits.present = 1;
  PD_entry.flag_bits.read_write = 1;
  PD_entry.flag_bits.user_supervisor = 1;

  Memory::PTEntry PT_entry;
  PT_entry.flags = 0;
  PT_entry.flag_bits.present = 1;
  PT_entry.flag_bits.read_write = 1;
  PT_entry.flag_bits.user_supervisor = 1;
 
  /*
   * Cada diretório de páginas precisa apontar para uma tabela de páginas
   * cada tabela de páginas contém 512 entradas contíguas (aninhadas)
   * portanto, usaremos a matriz bidimensional kPT[][] 
  */
  int actual_page = 0;
  for(int PDentry = 0; PDentry<512; PDentry++) {
    for(int PTentry = 0; PTentry<512; PTentry++) {
      if(actual_page < 6)
        PT_entry.flag_bits.present = 0;
      else 
        PT_entry.flag_bits.present = 1;
      PT_entry.flag_bits.physical_address = actual_page;
      actual_page++;
      kPT[PDentry][PTentry] = PT_entry;
    }
    PD_entry.flag_bits.present = 1;
    PD_entry.flag_bits.physical_address = (reinterpret_cast<u64>(&(kPT[PDentry])));
    kPD[PDentry] = PD_entry;
  }


  /*

  // 0x7F7 = página-base do APIC (0x00000000FEE00000)
  //entry.flag_bits.page_ppn = 0x7F7;
  //kPDPT[450] = entry;

  // 900MB até 902MB de memória == APIC.
  // o resto da memória só será configurada na hora de inicializar tabela para processos
*/
  
  ASSERT(kPT[0][0].flag_bits.present == 1,   -ENOPT,   "Failed to create PT");
  ASSERT(kPD[0].flag_bits.present    == 1,   -ENOPD,   "Failed to create PD");
  ASSERT(kPDPT[0].flag_bits.present  == 1,   -ENOPDPT, "Failed to create PDPT");
  ASSERT(kPML4[0].flag_bits.present  == 1,   -ENOPML4, "Failed to create PML4");
  

  dbg("kmain()-> Recriando tabela de paginação\n");
  __asm__ volatile( 
      "mov %%cr4, %%rax;" 
      "and $0xffffffdf, %%eax;" 
      "mov %%rax, %%cr4;" 
      "mov %0, %%cr3;" 
      : 
      : "r" (kPML4)
      :"rax","memory"
      );
  dbg("kmain()-> Tabela de paginação recriada com sucesso\n");
  halt();
 
  

  //STI;*/
}
