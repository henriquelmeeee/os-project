#include "HAL.h"

namespace HAL {

void System::initialize_syscalls() {
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

bool System::change_to_kernel_addr_space() {
  if(kernel_pagination_already_created) {
    write_cr3((u64)__kernel_pml4);
    return true;
  } else {
    __kernel_pml4 = (void*) kPML4;
    kernel_pagination_already_created = true;

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
    
    asm volatile(
        "mov %0, %%cr3;"
        :
        : "r" (kPML4)
        : "rax", "memory"
    );
    return true;

  }
}

void System::write_cr3(u64 value) {
  dbg("[System::write_cr3()] TODO");
  while(true);
}

u64 read_cr3() {
  dbg("System::read_cr3() TODO");
  while(true);
}

} // namespace HAL 
