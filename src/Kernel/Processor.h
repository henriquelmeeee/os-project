#pragma once

#ifndef PROCESSOR
#define PROCESSOR

#include "Memory/Base_Mem.h"

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

class CPU {
  public:
    unsigned short id;
    Memory::Vector<Device*> devices;

    unsigned long get_eax(unsigned long *to_ret) {
      return 1; // TODO FIXME implement that
    }



    CPU(unsigned short _id=0) : id(_id) {}
} extern *Processor;

#endif
