#pragma once

#ifndef _MEMORY
#define _MEMORY

#include "Base_Mem.h"
#include "../Utils/Base.h"
#include "../Tasks/Process.h"

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

  public:
    VMObject(u64 virtual_page, u64 physical_page = 0) : m_virtual_page(virtual_page) {
      dbg("Kernel: Novo VMOBject criado\nA página virtual é %d\n", (int)virtual_page);
      // TODO talvez setar as permissões logo aqui
      this->m_physical_page = physical_page;
    }

    bool map(Process* process) {
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


      if(!((*pdpt_entry) & PG_PRESENT)) {
        dbg("Page Directory %d indisponível, criando um novo\n", pd_index);
        // PDPT e PML4 já são pré-alocados fixamente
        *pml4_entry = *pml4_entry | PG_PRESENT;
        *pdpt_entry = *pdpt_entry | PG_PRESENT;
        pd_entry = (u64*) kmalloc(512 * sizeof(u64));
        pt_entry = (u64*) kmalloc(512 * sizeof(u64));
        
        *pd_entry = *pt_entry & ~0xFFF; // obtém o endereço do PT sem considerar as flags
        *pt_entry = (v_page * PAGE_SIZE) & ~0xFFF; // obtém o endereço da página virtual sem considerar as flags

        // TODO flags para pd_entry e pt_entry q alocamos agora, se pa pro pml4 e pdpt tbm
        m_pt_location = pt_entry;
      } else {
        // TODO get PD pointer and PT pointer
        if((*pt_entry) & PG_PRESENT)
          return false;
        *pt_entry = *pt_entry | PG_PRESENT;
        // TODO permissões
        m_pt_location = pt_entry;
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
    Memory::Vector<VMObject*> m_vm_objs;
    Process* m_process;
    u32 m_size;
  public:
    Region() {
      dbg("Nova região criada\n");
    }

    bool map(VMObject *vm) {
      if(vm == nullptr) {
        throw_panic(0, "Passed nullptr to 'map_new_vm()'");
      }
      
      m_vm_objs.append(vm);
      m_size += 1; // 1 page

      vm->map(m_process);

      dbg("Alocado novo VMObject para Region\n");
    }
};

#endif
