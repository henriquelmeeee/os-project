#pragma once

#ifndef BASE_H
#define BASE_H
#define CLI __asm__("cli")
#define STI __asm__("sti")

#define NO_CALLER_SAVED_REGISTERS __attribute__((no_caller_saved_registers))

#define u64 unsigned long
#define u32 unsigned int
#define u128 unsigned long long
#define i64 signed long
#define i32 signed int
#define u16 unsigned short
#define u8 unsigned char
#define i16 signed short
#define i8 signed char

#define IRET \
  outb(0x20, 0x20); \
  outb(0xA0, 0x20); \
  __asm__ volatile( \
      "add $8, %rsp;" \
      "pop %r15;" \
      "pop %r14;" \
      "pop %r13;" \
      "pop %r12;" \
      "pop %r11;" \
      "pop %r10;" \
      "pop %r9;" \
      "pop %r8;" \
      "pop %rbp;" \
      "pop %rdi;" \
      "pop %rsi;" \
      "pop %rdx;" \
      "pop %rcx;" \
      "pop %rbx;" \
      "pop %rax;" \
      "iretq;" \
      )

enum BootType {
  BIOS = 0,
  UEFI = 1,
};
struct BootloaderInfo {
  BootType boot_type;
} __attribute__((packed));

#include "../Core/panic.h"
#include "../Memory/Base_Mem.h"

// ERRORS
#define ENOMEM 1
#define EINVAL 2
#define ENOPT 3
#define ENOPD 4
#define ENOPDPT 5
#define ENOPML4 6

#define KB *1000
#define MB *1000000
#define GB *1000000000
#define TB *1000000000000

static inline int kstrcmp(const char *str1, const char *str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(unsigned char *)str1 - *(unsigned char *)str2;
}

static inline int kstrncmp(const char *str1, const char *str2, int n) {
    for (int i = 0; i < n; ++i) {
        if (str1[i] != str2[i]) {
            return static_cast<unsigned char>(str1[i]) - static_cast<unsigned char>(str2[i]);
        }
        if (!str1[i]) { // Check if end of string is reached.
            break;
        }
    }
    return 0;
}

struct VBEInfo {
  u16 flags;
  unsigned char windowA, windowB;
  u16 granularity;
  u16 windowSize;
  u16 segmentA, segmentB;
  u32 winFuncPtr;
  u16 pitch;

  u16 width, height;
  
  unsigned char wChar, yChar, planes, bpp, banks; // bpp = bits por pixel
  unsigned char memoryModel, bankSize, imagePages;
  unsigned char reserved0;

  unsigned char readMask, redPosition;
  unsigned char greenMask, greenPosition;
  unsigned char blueMask, bluePosition;
  unsigned char reservedMask, reservedPosition;
  unsigned char directColorAttributes;

  u32 physbase;
  u32 reserved1;
  u16 reserved2;
};

extern unsigned long long TOTAL_RAM;
extern unsigned long TOTAL_RAM_IN_PAGE_SCALE;


struct P {
  u64 Present;
  u64 Writable;
  u64 User;
  u64 WriteThrough;
  u64 CacheDisabled;
  u64 Accessed;
  u64 Dirty;
  u64 PAT;
  u64 Global;
  u64 Ignored : 3;
  u64 Frame : 40;
  u64 Available : 11;
  u64 NX;
} __attribute__((packed));

struct PT {
  struct P Pe[512];
};

struct PD {
  struct PT* PTe[512];
};

struct PDPT {
  struct PD* PDe[512];
};

struct PML4 {
  struct PDPT* PDPTe;
};

struct multiboot {
  // TODO
};

#define PAGE_SIZE 2*1024*1024

extern unsigned long long mem_usage;

extern "C" void *memcpy(void* dest, const void* src, int size);

/*static inline void outb(unsigned short port, unsigned char value) {
  __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}*/

#define outb(port, val) \
  __asm__ volatile("outb %0, %1" : : "a"(static_cast<unsigned char>(val)), "dN"(static_cast<unsigned short>(port)))

#define inb(port) ({\
  unsigned char result; \
  __asm__ volatile("inb %%dx, %%al" : "=a" (result) : "d" ((short)port)); result; })

/*#define dbg(x) do { \
  const char* str = (x); \
  for(int i = 0; str[i] != '\0'; ++i) \
    outb(0x3F8, str[i]); \
} while (0)*/

#include "../kstd/stdio.h"

//#define dbgl dbg("\n")



enum ErrorType {
  CRITICAL,
  IRRELEVANT,
  INFORMATIVE,
  SHOULD_CRASH,
  SHOULD_WARN,
};

#define throw_panic(err_code, msg) \
  unsigned long rip; \
  __asm__ volatile("leaq (%%rip), %0" : "=r" (rip)); \
  panic((unsigned int)0, const_cast<char*>(msg), rip); \

#define TRY(condition, error_type, msg) \
  if(!(condition)) { \
    if(error_type == 0) { \
      unsigned long rip; \
      __asm__ volatile("leaq (%%rip), %0" : "=r" (rip)); \
      panic((unsigned int)0, const_cast<char*>(msg), rip);\
    } \
  } \

#define ASSERT(condition, error_type, msg) \
  if(!(condition)) { \
    if(error_type == 0) { \
      unsigned long rip; \
      __asm__ volatile("leaq (%%rip), %0" : "=r" (rip)); \
      panic((unsigned int)0, const_cast<char*>(msg), rip);\
    } \
  } \

namespace Utils {

  extern Memory::Vector<i16> pPages;

  template <typename T>
  void append_to_array(T* array, T value, unsigned long size);

  template <typename T>
  bool kmemcpy(T* dest, T* orig, unsigned long size) {
    dest = (char*)dest;
    orig = (char*)orig;
    for(int i = 0; i<size; i++) {
      dest[0] = orig[0];
    }
    return true;
  }
}
bool itos(long value, char* to_ret);
bool itoh(long value, char* to_ret);

#include "../kstd/stdio.h"

void halt();
//extern Memory::PML4Entry kPML4[512];

void change_page_table(void* pml4_address);

class FS;
extern FS* g_fs;

#endif

