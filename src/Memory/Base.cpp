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

  u16 AllocateNewPage(u16 vPage) {
    // TODO precisamos checar se há uma tabela PDPT quando formos fazer PML4[i]
    // TODO big lock
    if(vPage < 512) // não queremos mexer com as páginas do Kernel
      return -EINVAL;
    //
    // Aqui nós pegamos o valor para a PDPT da página virtual atual:
    //struct PageEntry *vPageLocation = &( (* (Process::CurrentProcess->PML4[vPage/511]) )[vPage%511] );
 
    i16 pPage = FindPhysicalPageAvailable(true); 
    if(pPage == -ENOMEM) {
      throw_panic(0, "Out of memory");}

    //vPageLocation->flag_bits.present = 1;
    //vPageLocation->flag_bits.page_ppn = pPage;
    
    // TODO UpdatePageTable()

    return pPage;
  }
}
