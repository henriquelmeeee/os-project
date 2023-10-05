#pragma once

#ifndef ___DEBUG
#define ___DEBUG

//#define __KMALLOC_DBG

#include "../../kstd/stdlib.h"

inline void ___memory_dump(void* addr, int bytes_to_print = 20) {
  dbg("Memory Dump\nEndereço\t\tValor (signed)\t\tValor (unsigned)\t\tValor (char)");
  for(int offset = 0; offset<bytes_to_print; offset++) {
    char* __addr = (char*)addr;
    __addr+=offset;
    dbg("%p\t\t%d\t\t...\t\t%c", (void*)__addr, (int)*__addr, *__addr);
  }
}

#endif
