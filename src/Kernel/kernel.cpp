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

#if 0
alignas(4096) Memory::PML4Entry kPML4[512];
alignas(4096) Memory::PDPTEntry kPDPT[512];
alignas(4096) Memory::PDEntry kPD[512];
alignas(4096) Memory::PTEntry kPT[512][512];
#endif

alignas(4096) u64 kPML4[512];
alignas(4096) u64 kPDPT[512];
alignas(4096) u64 kPD[512];
alignas(4096) u64 kPT[512][512];

//Memory::PhysicalRegion physical_stack;
//Memory::PhysicalRegion physical_heap;
//Memory::PhysicalRegion physical_data;

Processor* processor = new Processor(0);

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
      kprintf("Warning: cannot initialize serial for debugging", 0xe);

  dbg("kmain()-> Kernel iniciando");
  char* txtaddr = (char*) 0xB8000;

  system.change_to_kernel_addr_space();

  //if(!(system.init_idt())) {
    //throw_panic(0, "Failed to initialize IDT");
  //} 
#if 0
  if(!(system.change_to_kernel_addr_space())) {
    throw_panic(0, "Failed to recreate pagination for Kernel");
  }
#endif


  //g_fs = (FS*)kmalloc(sizeof(FS));
  //g_fs->init();
  //FILE* initd = g_fs->fopen("initd");
  //g_fs->list_dir("/diretorio");
  //system.DoAutomatedTests();
    
  g_timer_temporary_stack = ((u64)kmalloc(1024))+1024; // nao está em uso POR ENQUANTO, mas TALVEZ eu use

  system.pic.append_idt((u64) timer_isr, 32);
  g_kernel_procs = Memory::Vector<Process*>();
  //g_kernel_procs[0] = (proc);
  
  Process proc = Process("teste");
  g_kernel_procs[0] = &proc;
  g_kernel_procs[1] = nullptr;
  
  //u64 rip = (u64) proc.m_regs.rip;
  //asm volatile("jmp *%0" : : "r" (rip));

  u16 divisor = 23864;
  outb(0x43, 0x36);  // Define o modo e o canal do PIT

  u8 low = (u8)(divisor & 0xFF);
  u8 high = (u8)((divisor >> 8) & 0xFF);

  outb(0x40, low);  // Envia o byte inferior do divisor
  outb(0x40, high); // Envia o byte superior do divisor

  system.initialize_syscalls();
  kprintf(" ");
  kprintf("Kernel initialized, switching to ring 3 and starting context switch...", 2);
  STI;
  system.change_ring(3);
  while(true);

}
