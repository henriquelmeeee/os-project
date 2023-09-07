/* Virtual Memory Layout:
 * 0-1GB          -> Kernel related
 * 1->2GB         -> Stack related (userspace)
 * 2->3GB         -> Heap related (userspace)
 * 3->4GB         -> Static Data related (userspace)
*/

// FIXME se eu ler um setor aleatorio agora tipo o setor 2, na hora de ler o setor 200
// ele vai pegar um valor meio q aleatorio parece ser algo relacionado ao cache sla

#define KERNEL_SIZE 26112
// Drivers:
//#include "Drivers/Keyboard.h"
#include "Drivers/VIDEO/preload.h"
#include "Drivers/Disk.h"
//#include "Drivers/Mouse.h"

#include "Filesystem/Filesystem.h"

#include "Core/DefaultConfig.h"
#include "Utils/Base.h"
#include "Utils/Errors.h"

#include "Memory/Base_Mem.h"
#include "Memory/Heap/Heap.h"

#include "Tasks/Process.h"
#include "Tasks/KernelTasks/KTasks.h"
#include "Core/panic.h"

// Interrupções:
#include "Interruptions/preload.h"
#include "Drivers/Keyboard.h"

#include "Processor.h"
#include "HAL/HAL.h"

unsigned long long mem_usage = 0;

extern "C" void kmain(BootloaderInfo* info);
extern "C" __attribute__((sysv_abi)) void kentrypoint(BootloaderInfo* info) {kmain(info);}

volatile void sleep(unsigned long long ticks) {
  for(unsigned long long i = 0; i<ticks; i++) {for (volatile int j = 0; j<1000000; j++){}}
}

unsigned long *pages_in_use;

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

#define fo (char)15

alignas(4096) Memory::PML4Entry kPML4[512];
alignas(4096) Memory::PDPTEntry kPDPT[512];
alignas(4096) Memory::PDEntry kPD[512];
alignas(4096) Memory::PTEntry kPT[512][512];

//Memory::PhysicalRegion physical_stack;
//Memory::PhysicalRegion physical_heap;
//Memory::PhysicalRegion physical_data;

HAL::System system = HAL::System();

extern "C" void __attribute__((noinline)) kmain(BootloaderInfo* info) { // point kernel
  CLI;
 
  u64* rbp;
  __asm__ volatile("mov %%rbp, %0" : "=r" (rbp));
  dbg("RBP for kmain(): %p\n", rbp);
  dbg("*RBP for kmain(): %p\n", (void*)*rbp);
  dbg("*(RBP+8) for kmain(): %p, kentrypoint() address: %p\n", (void*)(*(rbp+1)), (void*)kentrypoint);
  Text::text_clear();
  Text::Writeln("Loading kernel", 9);
  // TODO check if BootloaderInfo is corrupted
  
  if(info == 0) {
    throw_panic(0, "BootloaderInfo structure pointer is null");
  }

  //FIXME info->boot_type is not working idk

  if(info->boot_type == BIOS) {
    Text::Writeln("BIOS boot detected", 9);
  } else {
    Text::Writeln("UEFI boot detected", 9);
  }

  system = HAL::System();
  mem_usage+=KERNEL_SIZE;

  if(!(system.init_serial_for_dbg()))
      Text::Writeln("Warning: cannot initialize serial for debugging", 0x4);

  initialize();

  pages_in_use = (unsigned long*)kmalloc(PAGE_SIZE);
  if(pages_in_use == nullptr) {
    throw_panic(0, "kmalloc() returned nullptr");
  }

  dbg("kmain()-> Kernel iniciando\n");
  char* txtaddr = (char*) 0xB8000;

  if(!(system.init_idt())) {
    throw_panic(0, "Failed to initialize IDT");
  } 

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

  Text::Writeln("Pagination for kernel enabled, initializing devices...", 9);

  dbg("kmain()-> Tabela de paginação recriada com sucesso\n");
  dbg("kmain()-> Criando regiões para stack, heap e data\n");

  //physical_stack        = kmmap();
  //physical_heap         = kmmap();
  //physical_data         = kmmap();


  
  //TRY(Initialize::SecondStage::init(), ErrorType{CRITICAL}, "SecondStage init failed");
  //TRY(Process::init(), ErrorType{CRITICAL}, "Tasks initialization failed");
  
  //Binary* shell_buffer = FS::LoadBinary("Shell");
  //dbg("shell carregado (512 bytes)\n"); 
  FS filesystem;
  Text::NewLine();

  Text::Writeln("Files in filesystem:", 3);

  for(int i = 0; i < filesystem.total_inodes_amount; i++) {
    dbg("a");
    Text::Writeln(filesystem.inodes[i].name);
  }

  Text::NewLine();
  Text::Writeln("Kernel: Shell will be spawned", 2);
  while(true);

  Text::Writeln("Kernel: Starting processes by Watchdog Kernel Task", 9);
  CreateKernelProcess((void*)KernelTask::Watchdog);
  system.append_idt((unsigned long)KernelTask::Watchdog, 32);

  outb(0x20, 0x11);
outb(0xA0, 0x11);
outb(0x21, 0x20);
outb(0xA1, 0x28);
outb(0x21, 0x04);
outb(0xA1, 0x02);
outb(0x21, 0x01);
outb(0xA1, 0x01);
outb(0x21, 0x0);
outb(0xA1, 0x0);

  u32 divisor = 1193180 / 50;
  outb(0x43, 0x36);
  u8 low = (u8)divisor&0xFF;
  u8 high = (u8)((divisor>>8)&0xFF);

  outb(0x40, low);
  outb(0x40, high);
 
  system.DoAutomatedTests();

  void* teste = (void*)kmalloc(96);
  dbg("teste alocado\n");

  dump_kernel_heap();

  while(true);

  

}
