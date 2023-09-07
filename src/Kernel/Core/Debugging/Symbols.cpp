#include "Symbols.h"

namespace Symbols {
Symbol symbol_table[] = {
  {"kmain", reinterpret(kmain)},
  {"null", nullptr},
};

Symbol* sym_lookup(void* address) {
  for(int i = 0; Symbols::symbol_table[i].address != nullptr; i++) {
    if(Symbols::symbol_table[i].address == address)
      return &Symbols::symbol_table[i];
  }
  return nullptr;
}
}
