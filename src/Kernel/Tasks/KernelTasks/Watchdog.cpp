#include "../../Utils/Base.h"
#include "../../Drivers/VIDEO/preload.h"

namespace KernelTask {
  void Watchdog() {
label:
    for(int i = 0; i<99999999; i++);
    kprintf("Kernel: Watchdog task running");
    goto label;
  }
}
