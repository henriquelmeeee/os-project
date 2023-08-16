#ifndef KEYBOARD_DRIVER
#define KEYBOARD_DRIVER
#include "../Interruptions/Stack.h"
#include "../Utils/Base.h"
namespace Drivers {
  namespace Keyboard {
    struct KeyboardStack {
      
    };
    void NO_CALLER_SAVED_REGISTERS keyboard_interrupt_key(struct KeyboardStack *s);    
    extern unsigned int keys[255];
  }
}
#endif
