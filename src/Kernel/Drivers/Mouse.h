#include "../Utils/Base.h"

namespace Drivers {
  namespace Mouse {
    extern "C" void mouse_interrupt(struct State *s);
  }
}
