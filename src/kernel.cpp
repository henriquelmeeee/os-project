/* Virtual Memory Layout:
 * 0-1GB          -> Kernel related
 * 1->2GB         -> Stack related (userspace)
 * 2->3GB         -> Heap related (userspace)
 * 3->4GB         -> Static Data related (userspace)
*/

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

#include "Processor.h"
#include "HAL/HAL.h"

unsigned long long mem_usage = 0;

extern "C" void kentrypoint() {kmain();}

volatile void sleep(unsigned long long ticks) {
  for(unsigned long long i = 0; i<ticks; i++) {for (volatile int j = 0; j<1000000; j++){}}
}

unsigned long *pages_in_use;

struct IDT_ptr {
  unsigned short limit;
  unsigned long base;
} __attribute__((packed));

alignas(4096) unsigned long IDT_entries[256];

void halt() {
  __asm__ volatile(
      "hlt"
      );
}

bool IDT_append(u64 addr, i32 offset) {
  /*
  IDT_entries[offset] = \
    {
      .base_lo = static_cast<u16>(addr & (u64) 0xFFFF),
      .sel = (unsigned short)0x08,
      .always0 = (char)0,
      .flags = (unsigned char)0x8E,
      .base_mid = static_cast<u16>((addr>>16) & (u64)0xFFFF),
      .base_hi = static_cast<u32>(addr>>32),
      .reserved = (u32)0,
    };
  */
  u16 selector = 0x08;
  u8 flags = 0x8E;
  u64 entry = ((u64)flags << 40) |
    ((u64)selector << 16) |
    (addr & 0xFFFF) |
    ((addr & 0xFFFF0000) << 32) |
    ((u64)(addr >> 32) << 48);

  IDT_entries[offset] = entry;

  return true;
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
      for(auto entry : IDT_entries)
        __builtin_memset(&entry, 0, 8);

      for(int i = 0; i<255; i++)
        IDT_append(reinterpret_cast<u64>(i_spurious), i);

      //u64 keyboard_addr = reinterpret_cast<u64>(Drivers::Keyboard::keyboard_interrupt_key);

      //IDT_append(keyboard_addr, 31);
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
      #define PIC_ICW4     0x01 // Interrupt control word

      outb(PIC1_COMMAND, PIC_INIT);
      outb(PIC2_COMMAND, PIC_INIT);
    
      // ICW2: Definir o vetor de início
      outb(PIC1_DATA, 32);   // Definir o vetor de interrupção inicial do PIC1 para 32 (0x20)
      outb(PIC2_DATA, 40);    // Definir PIC2 para ser acionado pela linha IRQ2 do PIC1 (40)
      
      // ICW3: Informar ao PIC1 que o PIC2 está em IRQ2 e informar ao PIC2 seu número de cascata
      outb(PIC1_DATA, 0x04); 
      outb(PIC2_DATA, 0x02);

      outb(PIC1_DATA, PIC_ICW4);
      outb(PIC2_DATA, PIC_ICW4);

      // Desmascarando interrupção do teclado
      
      outb(PIC1_DATA, 0xFE);
      outb(PIC2_DATA, 0xFF);

      return true;
    }
    bool init() {
      TRY(init_idt_entries(), ErrorType{CRITICAL}, "An error occurred while trying to initialize the IDT");
      TRY(init_idt_ptr(256), ErrorType{CRITICAL}, "An error occurred while trying to initialize the IDT (lidt)");
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

alignas(4096) Memory::PML4Entry kPML4[512];
alignas(4096) Memory::PDPTEntry kPDPT[512];
alignas(4096) Memory::PDEntry kPD[512];
alignas(4096) Memory::PTEntry kPT[512][512];

Memory::PhysicalRegion physical_stack;
Memory::PhysicalRegion physical_heap;
Memory::PhysicalRegion physical_data;


HAL::System system = HAL::System();

extern "C" void kmain() {
  CLI;

  Text::text_clear();
  Text::Writeln("Loading kernel", 3);

  mem_usage+=KERNEL_SIZE;

  if(!(system.init_serial_for_dbg()))
      Text::Writeln("Warning: cannot initialize serial for debugging", 0x4);

  system = HAL::System();

  dbg("kmain()-> Kernel iniciando\n");
  char* txtaddr = (char*) 0xB8000;

  /*
    * Agora, nós precisamos configurar uma nova tabela de paginação base
    * essa tabela vai ser genérica, principalmente para conseguirmos usar
    * a região de memória mapeada para APIC
  */

  Memory::PML4Entry entry;
  
  for(int i = 0; i<512; i++) {
    kPDPT[i] = {0};
    kPML4[i] = {0};
    kPD[i] = {0};
  }

  entry.flags = 0; // limpa todas flags (define todas como 0)
  for(int pml4e = 0; pml4e<512; pml4e++) {
    //entry.flag_bits.physical_address = (reinterpret_cast<u64>(kPDPT)); // Inicialmente alocaremos 1 única PDPT
    entry.flags = (reinterpret_cast<u64>(kPDPT) & ~0xFFF) | 0x3; 
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
  for(int PDPTentry = 0; PDPTentry<512; PDPTentry++) {
    //PDPT_entry.flag_bits.physical_address = (reinterpret_cast<u64>(&(kPD[PDPTentry])));
    PDPT_entry.flags = (reinterpret_cast<u64>(&(kPD[PDPTentry])) & ~0xFFF) | 0x3; 
    kPDPT[PDPTentry] = PDPT_entry;
  }

  Memory::PDEntry PD_entry;
  PD_entry.flags = 0;

  Memory::PTEntry PT_entry;
  PT_entry.flags = 0;
 
  /*
   * Cada diretório de páginas precisa apontar para uma tabela de páginas
   * cada tabela de páginas contém 512 entradas contíguas (aninhadas)
   * portanto, usaremos a matriz bidimensional kPT[][] 
  */
  int actual_page = 0;
  for(int PDentry = 0; PDentry<512; PDentry++) {
    for(int PTentry = 0; PTentry<512; PTentry++) {
      PT_entry.flags = (actual_page*4096) | 0x3; 
      actual_page++;
      kPT[PDentry][PTentry] = PT_entry;
    }
    //PD_entry.flag_bits.physical_address = (reinterpret_cast<u64>(&(kPT[PDentry])));
    PD_entry.flags = (reinterpret_cast<u64>(&kPT[PDentry])) & ~0xFFF | 0x3; 
    kPD[PDentry] = PD_entry;
  }


  /*

  // 0x7F7 = página-base do APIC (0x00000000FEE00000)
  //entry.flag_bits.page_ppn = 0x7F7;
  //kPDPT[450] = entry;

  // 900MB até 902MB de memória == APIC.
  // o resto da memória só será configurada na hora de inicializar tabela para processos
*/
  
  ASSERT(kPT[0][0].flag_bits.present == 1,   -ENOPT,    "Failed to create PT");
  ASSERT(kPD[0].flag_bits.present    == 1,   -ENOPD,    "Failed to create PD");
  ASSERT(kPDPT[0].flag_bits.present  == 1,   -ENOPDPT,  "Failed to create PDPT");
  ASSERT(kPML4[0].flag_bits.present  == 1,   -ENOPML4,  "Failed to create PML4");
  
  dbg("kmain()-> Recriando tabela de paginação\n");
  __asm__ volatile( 
      //"mov %%cr4, %%rax;" 
      //"and $0xffffffdf, %%eax;" 
      //"mov %%rax, %%cr4;" 
      "mov %0, %%cr3;" 
      : 
      : "r" (kPML4)
      :"rax","memory"
      );

  Text::Writeln("Pagination for kernel enabled, initializing devices...", 3);

  dbg("kmain()-> Tabela de paginação recriada com sucesso\n");
  dbg("kmain()-> Criando regiões para stack, heap e data\n");

  physical_stack        = kmmap();
  physical_heap         = kmmap();
  physical_data         = kmmap();

  TRY(Initialize::FirstStage::init(), ErrorType{CRITICAL}, "FirstStage init failed");
  //TRY(Initialize::SecondStage::init(), ErrorType{CRITICAL}, "SecondStage init failed");
  //TRY(Process::init(), ErrorType{CRITICAL}, "Tasks initialization failed");
  
  //Binary* shell_buffer = FS::LoadBinary("Shell");
  //dbg("shell carregado (512 bytes)\n"); 
  
  FS filesystem;
  Text::NewLine();

  //Text::Write("Kernel loaded, invoking shell.", 2);
  Text::Writeln("Files in filesystem:", 2);

  //for(int i = 0; i < _filesystem.total_inodes_amount; i++)
    //dbg("a");
    //Text::Write(_filesystem.inodes[i].name);

  while(true);
 
  

}
