#pragma once

#ifndef __SYMBOLS
#define __SYMBOLS

#include "../../Utils/Base.h"
//#include "../../HAL/HAL.h"

extern "C" void kmain(BootloaderInfo* info);
extern "C" __attribute__((sysv_abi)) void kentrypoint(BootloaderInfo* info);

namespace Symbols {

struct Symbol {
  const char* name;
  void* address;
};

#define reinterpret reinterpret_cast<void*>

extern Symbol symbol_table[];

Symbol* sym_lookup(void* address);

}

#endif
