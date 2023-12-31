#pragma once

//#include "../Memory/Memory.h"
//#include "../Memory/Heap/Heap.h"

//#include "../Memory/Memory.h"

#include "../Memory/Heap/Heap.h"

#include "../Core/API/Elf.h"
#include "../Core/API/Debug.h"

template<typename Lambda>
class Functor {
  public:
    Lambda lambda;
    Functor(Lambda lambda) : lambda(lambda) {}

    template <typename... Args>
      auto operator()(Args... args) -> decltype(lambda(args...)) {
        return lambda(args...);
      }
};

template<typename Lambda>
Functor<Lambda> MakeFunctor(Lambda&& lambda) {
  return Functor<Lambda>(lambda);
}

#if 0
namespace GenericSpinlock {
  bool is_locked = false;
  void lock() {
    while(is_locked); // TODO tornar isso atômico
    is_locked = true;
  }
  void unlock() {
    is_locked = false;
  }
}

namespace Spinlock {
  enum LockType {
    Disk,
    Memory,
    Global
  };

  bool is_disk_locked = false;
  bool is_memory_locked = false;
  bool is_global_locked = false;

  void lock(LockType type = Global) {
    if(type == Global) {
      while(is_memory_locked || is_disk_locked || is_global_locked);
      is_global_locked = true;
      return;
    } else if (type == Memory) {
      while(is_global_locked || is_memory_locked);
      is_memory_locked = true;
      return;
    } else if (type == Disk) {
      while(is_global_locked || is_disk_locked);
      is_disk_locked = true;
      return;
    }
  }

  void unlock(LockType type) {
    if(type == Global)
      is_global_locked = false;
    else if (type == Memory)
      is_memory_locked = false;
    else if (type == Disk)
      is_disk_locked = false;
    return;
  }
} // namespace Spinlock
#endif




#if 0
void* operator new(unsigned long size) {
  void* to_ret = kmalloc(size);
  return to_ret;
}
TODO colocar o operator new aqui na stdlib
#endif


#if 0
template<typename RetType, typename... Args>
class Functor {
  public:
    RetType (*func)(Args...);
    Functor(RetType (*func)(Args...)) : func(func) {}
    RetType operator()(Args... args) {
      //kprintf("Callback chamado");
      return func(args...);
    }
};
#endif
#if 0

namespace Memory {
template <typename V>
class Vector {
  private:
    V* chunk;
    u32 capacity;
    u32 last_element_index;
    char buf[128];
  public:
    Vector() {
      this->chunk = (V*) kmalloc(sizeof(V)*32);
      this->capacity = 32;
      this->last_element_index = 0;
      //dbg("Vector::constructor-> novo vetor  construído\n");
    }

    V& operator[](u32 index) {
      if(index >= this->capacity-1) {
        //dbg("Vector::operator[]-> fora da capacidade (index=");
        //itos(index, this->buf);
        //dbg(buf);
        //dbg(")\n");
        do {
          V* new_chunk = (V*) kmalloc((capacity+32)*sizeof(V));
          for(int i = 0; i<this->capacity; i++) {
            //Utils::kmemcpy(new_chunk[i], this->chunk[i], sizeof(V));
          }
          //kfree(this->chunk);
          this->chunk = new_chunk;
          this->capacity += 32;
        } while(index>=this->capacity);
      }
      //if(index>last_element_index) {
        //last_element_index = index+1;
      //}
      return this->chunk[index];
    };

    bool append(V value) {
      // TODO checar this->capacity
      //this->chunk+(last_element_index*sizeof(V)) = value;
      ++last_element_index;
      return true;
    }

    bool has(V value) {
      for(int i = 0; i<last_element_index; i++) {
        if(chunk[i] == value)
          return true;
        return false;
      }
    }
};

struct PhysicalPage {
  void *proc_owning;
  u64 page_number;
};

/*
class PhysicalRegion {
  public:
    Vector<PhysicalPage> pages;
    u32 index_of_pages;

    u64 start_address;
    u64 end_address;

    PhysicalRegion() {
      //dbg("Memory::PhysicalRegion-> criado\n");
      this->index_of_pages=0;
    }

    void AllocateNewPage() {
      pages[0] = {(void*)0,512}; // temp value, temos que achar uma pagina disponivel
      // TODO add this page in kPD
      start_address = 512*4096;
      ++this->index_of_pages;
    }
};*/
extern Vector<Region*> physical_pages_in_use;

}

namespace Spinlock {
  volatile bool is_available = true;

  void lock() {
    while(__sync_lock_test_and_set(&is_available, false)) {asm volatile ("pause");}
    return;
  }

  void unlock() {
    __sync_lock_release(&is_available);
    return;
  }
};

#endif
