#pragma once

#include "../../Utils/Base.h"

class Processor {
  private:
  public:
    bool is_bsp = false;
    u32 m_id = 0;
    Processor(u32 id) : m_id(id) {}
    Processor(bool bsp, u32 id) : is_bsp(bsp), m_id(id) {

    }




};

Processor* current_processor() {
  //return system->bsp; // TODO refazer função quando formos suportar multiprocessadores
  return nullptr;
}
