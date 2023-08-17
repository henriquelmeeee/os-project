#pragma once

#ifndef PROCESSOR
#define PROCESSOR

class CPU {
  public:
    unsigned short id;

    unsigned long get_eax(unsigned long *to_ret) {
      return 1; // TODO FIXME implement that
    }

    CPU(unsigned short _id=0) : id(_id) {}
} extern *Processor;

#endif
