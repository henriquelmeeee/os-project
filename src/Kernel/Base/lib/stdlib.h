#define print(string) \
  __asm__ volatile( \
      "pusha;" \
      "movl %1, %%eax;" /* edi 1 == syscall id 1 == Driver Video print */ \
      "movl %0, %%edi;" \
      "int $0x80;" \
      "popa;" \
      : \
      : "r" (string) \
      : "eax", "edi" \
      ) \
