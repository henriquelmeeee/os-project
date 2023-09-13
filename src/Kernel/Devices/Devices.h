#pragma once

#ifndef __DEVICES
#define __DEVICES

#include "../Utils/Base.h"
#include "../Memory/Memory.h"

#if 0

// TTYs não serão implementados por enquanto.

enum TTYMode {
  Cocked,
  Raw,
};

class VirtualConsole {
  private:
    u8* m_framebuffer;
    TTYMode mode = Raw;
    Memory::Vector<Process*> m_procs;

    VirtualConsole() {
      
    }

    bool change() {
      framebuffer = (u8*) 
    }


};
#endif
#endif

