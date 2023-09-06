#pragma once

#ifndef _MEMORY
#define _MEMORY

#include "Base_Mem.h"
#include "../Utils/Base.h"
#include "../Tasks/Process.h"

typedef u64 Process; // TODO FIXME temporário, precisamos de um objeto que representa um processo ainda

#define KERNEL_END 1000000

#define PG_PRESENT 1

struct PagePermissions {
  u8 read = 0;
  u8 write = 0;
  u8 execute = 0;
};

class VMObject {
  private:
    u64 m_virtual_page;
    u64 m_physical_page;
    PagePermissions m_permissions;

    Process* m_process;

    VMObject(u64 virtual_page, u64 physical_page = 0) : m_virtual_page(virtual_page) {
      dbg("Kernel: Created new VMOBject\nVirtual page is %d\n", (int)virtual_page);
      // TODO talvez setar as permissões logo aqui
      this->m_physical_page = physical_page;
    }

    bool map() {
      if(m_physical_page < KERNEL_END)
        return false;
      u64 v_page = m_virtual_page;
      u16 pml4_index = (v_page >> 39) & 0x1FF;
      u16 pdpt_index = (v_page >> 30) & 0x1FF;
      u16 pd_index = (v_page >> 21) & 0x1FF;
      u16 pt_index = (v_page >> 12) & 0x1FF;
      
      u64* pml4 = (m_process->pml4)+pml4_index;
      u64* pdpt = (m_process->pdpt)+pdpt_index;
      u64* pd = (m_process->pd)+pd_index;
      u64* pt = (m_process->pt)+pt_index;

      if(!((*pd) & PG_PRESENT)) {
        dbg("Page Directory %d indisponível\n", pd_index);
        *pml4 = *pml4 | PG_PRESENT;
        *pdpt = *pdpt | PG_PRESENT;
        *pd = *pd | PG_PRESENT;
      } else {
        if((*pt) & PG_PRESENT)
          return false;
        *pt = *pt | PG_PRESENT;
        // TODO permissões
      }
    return true;
    }
};


#endif
