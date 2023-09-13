#pragma once

#ifndef _MEMORY
#define _MEMORY

#include "Base_Mem.h"
#include "../Utils/Base.h"
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

class Region {
  private:
    Memory::Vector<VMObject*> m_vm_objs;
    Process* m_process;
    u32 m_size;
  public:
    Region(Process* process) : m_process(process) {
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

      return true;
    }

    bool map_all(u32 virtual_page_start) {
      return true;
    }
};

#endif
