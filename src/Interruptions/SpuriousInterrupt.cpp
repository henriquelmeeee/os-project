#include "../Utils/Base.h"
#include "../panic.h"
#include "../Memory/Base_Mem.h"

char buf[128];

struct SpuriousInterrupt {
//TODO
};

extern "C" __attribute__((interrupt)) void i_spurious(SpuriousInterrupt *s) {
  CLI;
  dbg("Interrupts::i_spurious()-> interrupção desconhcida acionada\n");
  __asm__ volatile("hlt");
  /*for(unsigned long i = 0; i<s->int_no; i++) {
    dbg("a");
  }
  dbg("fim");
  while(true);*/
  /*
  if(s.err_code == 0) {
    CLI;
    throw_panic((unsigned int)0, "The CPU generated an unknown exception that cannot be handled");
  }*/
  //dbg("Interrupts::i_spurious()-> interrupção de código "); dbg(buf); dbg("\n");
  //STI;
}
