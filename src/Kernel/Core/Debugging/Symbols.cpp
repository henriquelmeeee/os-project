#include "Symbols.h"
#include "../../Drivers/VIDEO/preload.h"

namespace Symbols {
Symbol symbol_table[] = {
  {"kentrypoint", reinterpret(kentrypoint)},
  {"kmain", reinterpret(kmain)},
  {"Text::Writeln", reinterpret(Text::Writeln)},
  {"null", nullptr},
};

Symbol* sym_lookup(void* address) {
  dbg("Symbol lookup for %p\nkmain() pointer is %p and Writeln() pointer is %p\n", address, kmain, Text::Writeln);
  for(int i = 0; Symbols::symbol_table[i].address != nullptr; i++) {
    dbg("Looking for: %s\n", Symbols::symbol_table[i].name);
    if(Symbols::symbol_table[i].address == address)
      return &Symbols::symbol_table[i];
  }
  return nullptr;
}
}
