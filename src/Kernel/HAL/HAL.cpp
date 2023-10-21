#include "HAL.h"

void HAL::initialize_syscalls() {
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
