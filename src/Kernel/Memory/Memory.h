#pragma once

#ifndef _MEMORY
#define _MEMORY

#include "Base_Mem.h"
#include "../Utils/Base.h"
#include "../Tasks/Process.h"

class Process {
  public:
    u64* pml4;
    u64* pdpt;
    u64* pd;
    u64* pt;
}; // TODO remover esse CLASS PROCESS pq ele será feito em Tasks/Process.h de maneira adequada

#define KERNEL_END 1000000

#define PG_PRESENT 0x1
#define PG_WRITABLE 0x2
#define PG_READABLE 0 // ?? qual q é?
#define PG_EXECUTABLE 0 // ?? qual q é?

struct PagePermissions {
  u8 read = 0;
  u8 write = 0;
  u8 execute = 0;
};

class VMObject {
  // Se m_physical_page == 0, então o VMObject ainda não foi alocado
  // não é recomendável criar um VMObject sem um m_physical_page, porque ele ainda estará no vetor 
  // de páginas físicas que as syscalls usarão para procurar por páginas físicas disponíveis
  private:
    u64 m_virtual_page;
    u64 m_physical_page;
    u64* m_pt_location = nullptr;
    PagePermissions m_permissions;

    Process* m_process;
  public:
    VMObject(u64 virtual_page, u64 physical_page = 0) : m_virtual_page(virtual_page) {
      dbg("Kernel: Novo VMOBject criado\nA página virtual é %d\n", (int)virtual_page);
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

      m_pt_location = pt;

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

    inline void check_if_pt_is_valid() {
      if(m_pt_location == nullptr) {
        throw_panic(0, "Tried to manipulate a nullptr page table entry");
      }
    }

    void set_writable() {
      check_if_pt_is_valid();
      *m_pt_location = *m_pt_location | PG_WRITABLE;
    }

    void set_readable() {
      check_if_pt_is_valid();
      *m_pt_location = *m_pt_location | PG_READABLE;
    }

    void set_executable() {
      check_if_pt_is_valid();
      *m_pt_location = *m_pt_location | PG_EXECUTABLE;
    }
};

class Region {
  private:
  public:
    Region() {
      dbg("Nova região criada\n");
 
    }
};

#endif
