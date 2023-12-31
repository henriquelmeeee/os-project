#pragma once

#ifndef _MEMORY
#define _MEMORY

#include "Base_Mem.h"
#include "../Utils/Base.h"
#include "../kstd/stdlib.h"



class Process;
class Region;

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
 // private:
  public:
    u64 m_virtual_page;
    u64 m_physical_page;
    u64* m_pt_location = nullptr;
    PagePermissions m_permissions;

  //public:
    VMObject(u64 virtual_page, u64 physical_page = 0) : m_virtual_page(virtual_page) {
      dbg("Kernel: Novo VMOBject criado\nA página virtual é %d\n", (int)virtual_page);
      // TODO talvez setar as permissões logo aqui
      this->m_physical_page = physical_page;
    }

    bool map(Process* process);

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

inline void* operator new(unsigned long size) {
  void *p = kmalloc(size);
  return p;
}

class Region {
  private:
    Process* m_process;
    u64 current_vaddr = 0;
    u32 m_size;
  public:
    Memory::Vector<VMObject*> m_vm_objs;
    Region(Process* process, u64 vaddr_base, u64 paddr_base = 0) : m_process(process) {
      dbg("Nova região criada");
      current_vaddr = vaddr_base;
      if(paddr_base != 0)
        m_vm_objs.append(new VMObject(vaddr_base, paddr_base));
      else
        m_vm_objs.append(new VMObject(vaddr_base, vaddr_base)); // aloca uma única página por enquanto
      // TODO mmap() caso paddr_base == 0
      
    }

    u64 base_addr_as_physical() {
      return m_vm_objs[0]->m_physical_page * PAGE_SIZE;
    }

    bool map();
};

u64 kmmap(u64);
bool kunmap(u64);

#endif
