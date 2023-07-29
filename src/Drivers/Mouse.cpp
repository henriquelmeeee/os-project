#include "../Utils/Base.h"

namespace Drivers {
  namespace Mouse {
    extern "C" void mouse_interrupt(struct State *s) {
      CLI;
      __asm__ volatile("hlt");
      dbg("mouse!");
      while(true);
    }
  }
}
