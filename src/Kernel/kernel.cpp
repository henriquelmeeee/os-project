/* Virtual Memory Layout:
 * 0-1GB          -> Kernel related
 * 1->2GB         -> Stack related (userspace)
 * 2->3GB         -> Heap related (userspace)
 * 3->4GB         -> Static Data related (userspace)
*/

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

#include "Syscalls/Syscall.h"

alignas(4096) IDTEntry16 IDT_entries[256];

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

#if 0
alignas(4096) u64 kPML4[512];
alignas(4096) u64 kPDPT[512];
alignas(4096) u64 kPD[512];
alignas(4096) u64 kPT[512][512];
#endif

//Memory::PhysicalRegion physical_stack;
//Memory::PhysicalRegion physical_heap;
//Memory::PhysicalRegion physical_data;

volatile void kernel_process_test() {
  STI;
  dbg("Kernel process 1 here!");
  while(true);
_label:
  STI;
  goto _label;
#if 0
_label:
  kprintf("kernel process!");
  for(int i = 0; i<99999999; i++);
  goto _label;
#endif
}

volatile void kernel_process_test2() {
  STI;
  dbg("Kernel process 2 here!");
  __asm__ volatile("int $3");
  while(true);
_label2:
  STI;
  goto _label2;
#if 0
  kprintf("another kernel process!");
  for(int i = 0; i<99999999; i++);
  goto _label2;
#endif
}

Memory::Vector<Process> g_procs; //= Memory::Vector<Process>();
Memory::Vector<Process*> g_kernel_procs; // = Memory::Vector<Process>();
Process* g_current_proc = nullptr;
u64 g_timer_temporary_stack;
HAL::System system = HAL::System();
FS* g_fs;

#include "Interruptions/ContextSwitch.h"

extern "C" void __attribute__((noinline)) kmain(BootloaderInfo* info) { // point kernel
  u64* rbp;
  __asm__ volatile("mov %%rbp, %0" : "=r" (rbp));
  dbg("RBP for kmain(): %p\n", rbp);
  dbg("*RBP for kmain(): %p\n", (void*)*rbp);
  dbg("*(RBP+8) for kmain(): %p, kentrypoint() address: %p\n", (void*)(*(rbp+1)), (void*)kentrypoint);
  dbg("BootloaderInfo: %p", (void*)info);
  

  Text::text_clear();
  kprintf("Loading kernel", 9);
  // TODO check if BootloaderInfo is corrupted
  
  if(info == 0) {
    throw_panic(0, "BootloaderInfo structure pointer is null");
  }

  //FIXME info->boot_type is not working idk

  if(info->boot_type == BIOS)
    kprintf("BIOS boot detected", 9);
  else 
    kprintf("UEFI boot detected", 9);

  system = HAL::System();
  mem_usage+=KERNEL_SIZE;

  if(!(system.init_serial_for_dbg()))
      kprintf("Warning: cannot initialize serial for debugging", 0x4);

  dbg("kmain()-> Kernel iniciando");
  char* txtaddr = (char*) 0xB8000;

  //if(!(system.init_idt())) {
    //throw_panic(0, "Failed to initialize IDT");
  //} 
#if 0
  if(!(system.change_to_kernel_addr_space())) {
    throw_panic(0, "Failed to recreate pagination for Kernel");
  }
#endif

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


  
  // Iremos mapear muita memória para o Kernel, tudo identity-mapping
  // para que simplifiquemos a forma como o Kernel acessa a memória
  // o que incluirá o endereço 0xFEE00, localização da APIC
 
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
  


  dbg("kmain()-> Recriando tabela de paginação");
  __asm__ volatile( 
      //"mov %%cr4, %%rax;" 
      //"and $0xffffffdf, %%eax;" 
      //"mov %%rax, %%cr4;" 
      "mov %0, %%cr3;" 
      : 
      : "r" (kPML4)
      :"rax","memory"
      );

  //physical_stack        = kmmap();
  //physical_heap         = kmmap();
  //physical_data         = kmmap();

  //CreateKernelProcess((void*)KernelTask::Watchdog);
//outb(0x21, 0x20);
//outb(0xA1, 0x28);
//outb(0x21, 0x04);
//outb(0xA1, 0x02);
//outb(0x21, 0x01);
//outb(0xA1, 0x01);

  g_fs = (FS*)kmalloc(sizeof(FS));
  g_fs->init();
  FILE* initd = g_fs->fopen("initd");
  //g_fs->list_dir("/diretorio");
  system.DoAutomatedTests();
    
  g_timer_temporary_stack = ((u64)kmalloc(1024))+1024; // nao está em uso POR ENQUANTO, mas TALVEZ eu use

  system.pic.append_idt((u64) timer_isr, 32);
  g_kernel_procs = Memory::Vector<Process*>();
  //g_kernel_procs[0] = (proc);
  dbg("kernel_process_test: %p", (void*)kernel_process_test);
  
  Process proc = Process("teste", true, (void*)kernel_process_test);
  g_kernel_procs[0] = &proc;
  g_kernel_procs[1] = nullptr;
  
  //u64 rip = (u64) proc.m_regs.rip;
  //asm volatile("jmp *%0" : : "r" (rip));

  kprintf("System booted");

  u16 divisor = 23864;
  outb(0x43, 0x36);  // Define o modo e o canal do PIT

  u8 low = (u8)(divisor & 0xFF);
  u8 high = (u8)((divisor >> 8) & 0xFF);

  outb(0x40, low);  // Envia o byte inferior do divisor
  outb(0x40, high); // Envia o byte superior do divisor

  system.initialize_syscalls();
  STI;
  while(true);

}
