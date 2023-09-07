#include "../Tasks/Process.h"
#include "Memory.h"

bool VMObject::map(Process* process) {
    if(m_physical_page < KERNEL_END)
        return false;
      u64 v_page = m_virtual_page;
      u16 pml4_index = (v_page >> 39) & 0x1FF;
      u16 pdpt_index = (v_page >> 30) & 0x1FF;
      u16 pd_index = (v_page >> 21) & 0x1FF;
      u16 pt_index = (v_page >> 12) & 0x1FF;
      
      u64* pml4_entry = (process->pml4)+pml4_index;
      u64* pdpt_entry = (process->pdpt)+pdpt_index;
      u64* pd_entry;
      u64* pt_entry;

      // Agora precisamos achar ou alocar ponteiros para PD e PT

      if(!((*pdpt_entry) & PG_PRESENT)) {
        dbg("Page Directory %d indisponível, criando um novo\n", pd_index);
        // PDPT e PML4 já são pré-alocados fixamente
        *pml4_entry = *pml4_entry | PG_PRESENT;
        *pdpt_entry = *pdpt_entry | PG_PRESENT;
        pd_entry = (u64*) kmalloc(512 * sizeof(u64));
        pt_entry = (u64*) kmalloc(512 * sizeof(u64));
        
        *pd_entry = *pt_entry & ~0xFFF; // obtém o endereço do PT sem considerar as flags
        *pt_entry = (v_page * PAGE_SIZE) & ~0xFFF; // obtém o endereço da página virtual sem considerar as flags

        *pd_entry = *pd_entry | PG_PRESENT;
        *pt_entry = *pt_entry | PG_PRESENT;
      } else {
        pd_entry = (u64*) ( (*pdpt_entry) & ~0xFFF);
        if(! ((*pd_entry) | PG_PRESENT) )
          pt_entry = (u64*) kmalloc(512 * sizeof(u64));
        else
          pt_entry = (u64*) ( (*pd_entry) & ~0xFFF);

        if((*pt_entry) & PG_PRESENT)
          return false;

        *pt_entry = *pt_entry | PG_PRESENT;
      }
    m_pt_location = pt_entry;

    // TODO flags para PDPT, PD, PT
    return true;

}