#pragma once

#ifndef PROCESSOR
#define PROCESSOR

//#include "Memory/Base_Mem.h"

#if 0
enum DeviceType {
  PS2,
  Unknown,
};

class Device {
  public:
    virtual DeviceType get_dev_type() {return DeviceType::Unknown;};
    Device() {}
};

class PS2 : public Device {
  public:
    virtual DeviceType get_dev_type() {return DeviceType::PS2;};

    PS2() {}
};

struct KeyboardInterrupt {
  u64 rip;
  u64 cs;
  u64 flags;
  u64 rsp;
  u64 ss;
  u8 scancode;
};

class Keyboard : public PS2 {
  public:
    

    Keyboard() {}
};

#include "Utils/Base.h"

inline u64 read_msr(u32 msr) {
  u32 low, high;
  asm volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
  return ((u64)high << 32) | low;
}

inline void write_msr(u32 msr, u64 value) {
  u32 low = (u32) value;
  u32 high = (u32) (value >> 32);
  asm volatile("wrmsr" : : "a"(low), "d"(high), "c"(msr));
}

class Processor {
  public:
    u32 m_identifier;

    u64 m_local_apic_addr;


    Processor(u32 identifier) \
      : m_identifier(identifier) {}
    
} extern *processor;

#endif
#endif
