#include "../Utils/Base.h"
#include "Heap/Heap.h"
#include "Base_Mem.h"
#include "../Tasks/Process.h"
#if 0
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
}

#define START_FOR_KMMAP 1 GB
Memory::PhysicalRegion kmmap(u64 initial_size) {
  // TODO alinhar size para cima, para alinhamento
  initial_size /= PAGE_SIZE;
  Memory::PhysicalRegion temp;

  for(int page=0;page<initial_size;page++) {
    temp.AllocateNewPage();
    dbg("Nova página alocada para PhysicalRegion\n");
  }
  return temp;
}
#endif
