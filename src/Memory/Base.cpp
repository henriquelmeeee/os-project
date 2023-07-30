#include "../Utils/Base.h"
#include "Heap/Heap.h"
#include "Base_Mem.h"
#include "../Tasks/Process.h"

// DIRETIVAS TEMPORÁRIAS
#define TOTAL_RAM_IN_PAGE_SCALE 2048

namespace Memory {

  i16 FindPhysicalPageAvailable(bool put_in_ppage_vector = false) {
    /*for(int pPage = 512;pPage<TOTAL_RAM_IN_PAGE_SCALE;pPage++) {
      if(!(Utils::pPages.has(pPage))) {
        if(put_in_ppage_vector)
          Utils::pPages.append(pPage);
        return pPage;
      }
    }*/
    return -ENOMEM;
  }

  PhysicalPage kmmap(u64 size, u64 start_virtual_address) {
    // TODO alinhar size para cima, para alinhamento
    size = size/PAGE_SIZE;
    Vector<PhysicalPage> physical_pages_found;
    for(u64 page=0;page<size;page++) {
      // TODO append physical_pages_found
      // TODO mapear baseado no start_virtual_address
      // TODO add ppages_in_use
    }

    return physical_pages_found[0];

  }
}
