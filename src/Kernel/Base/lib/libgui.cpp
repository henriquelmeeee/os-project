#include "stdlib.h"
#include "../../Utils/Base.h"
#include "libgui.h"

#define RES_WIDTH 70
#define RES_HEIGHT 70

Window w;

char window_actions_bitmap[2][3] = {
  {0,0,0},
  {1,1,1},
};

namespace GUI {
  Window CreateWindow() {
    __asm__ volatile(
          "mov %%rax, $1;" // rax 1 == initialize window framebuffer
          "syscall;"
        );
    w = Window();
    for(int i = 0; i<2; i++) {
      char bitmap = window_actions_bitmap[i];
      w.framebuffer[bitmap[0]][window_actions_bitmap[1]] = bitmap[2];
    }
    return w;
  }
}
