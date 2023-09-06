#include "../Memory/Memory.h"
#include "../Memory/Heap/Heap.h"

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


