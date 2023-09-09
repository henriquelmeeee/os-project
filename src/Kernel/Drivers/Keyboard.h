#pragma once

#ifndef KEYBOARD_DRIVER
#define KEYBOARD_DRIVER
#include "../Utils/Base.h"
namespace Drivers {
  namespace Keyboard {
    struct KeyboardInterrupt {
      
    };
    void /*NO_CALLER_SAVED_REGISTERS*/ keyboard_interrupt_key(KeyboardInterrupt *s);    
    extern unsigned int keys[255];
  }
}
#endif
