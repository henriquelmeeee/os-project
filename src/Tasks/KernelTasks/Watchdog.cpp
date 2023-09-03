#include "../../Utils/Base.h"
#include "../../Drivers/VIDEO/preload.h"

namespace KernelTask {
  void Watchdog() {
    Text::Writeln("Kernel: Watchdog task running");
    while(true);
  }
}
