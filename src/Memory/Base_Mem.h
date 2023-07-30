#pragma once

#ifndef BASE_MEM
#define BASE_MEM

#include "Heap/Heap.h"

#define u64 unsigned long
#define u32 unsigned int
#define u128 unsigned long long
#define i64 signed long
#define i32 signed int
#define u16 unsigned short
#define u8 unsigned char
#define i16 signed short
#define i8 signed char

namespace Memory{

// Flags:
// present,writeable,user_access,write_through,cache_disabled,accessed,ignored,size,ignored,page_ppn,reserved,ignored,execution_disabled
// pml4 = size sempre 0
//
// IA32-e
struct PML4Flags {
    u64 present         : 1; // Bit 0 - Se 1, indica que a entrada está presente e válida.
    u64 read_write      : 1; // Bit 1 - Se 0, a página associada é somente leitura; se 1, é leitura e escrita.
    u64 user_supervisor : 1; // Bit 2 - Se 0, a página só pode ser acessada em modo supervisor (kernel); se 1, também em modo usuário.
    u64 write_through   : 1; // Bit 3 - Se 0, escritas na página não são escritas de volta no cache; se 1, são escritas de volta.
    u64 cache_disabled  : 1; // Bit 4 - Se 0, a página é cacheable; se 1, a página é não-cacheable.
    u64 accessed        : 1; // Bit 5 - Indica se a entrada foi acessada (geralmente é gerenciado pelo hardware).
    u64 ignored1        : 1; // Bits 6-8 - Reservados, não são usados no modo de paginação.
    u64 size       : 1; // Bits 9-11 - Bits disponíveis para uso do software (normalmente ignorados).
    u64 ignored2: 4;
    u64 physical_address: 36; // Bits 12-51 - Endereço base da tabela de páginas de nível 3 (PDP).
    u64 reserved        : 4; // Bits 52-62 - Reservados, não são usados no modo de paginação.
    u64 ignored3 : 11;
    u64 execute_dis:1;
} __attribute__((packed));

struct PDPTFlags {
    u64 present         : 1; // Bit 0 - Se 1, indica que a entrada está presente e válida.
    u64 read_write      : 1; // Bit 1 - Se 0, a página associada é somente leitura; se 1, é leitura e escrita.
    u64 user_supervisor : 1; // Bit 2 - Se 0, a página só pode ser acessada em modo supervisor (kernel); se 1, também em modo usuário.
    u64 write_through   : 1; // Bit 3 - Se 0, escritas na página não são escritas de volta no cache; se 1, são escritas de volta.
    u64 cache_disabled  : 1; // Bit 4 - Se 0, a página é cacheable; se 1, a página é não-cacheable.
    u64 accessed        : 1; // Bit 5 - Indica se a entrada foi acessada (geralmente é gerenciado pelo hardware).
    u64 ignored1        : 1; // Bit 6 - Reservado, não é usado no modo de paginação.
    u64 size        : 1; // Bit 7 - Reservado para uso futuro.
    u64 ignored2       : 4; // Bits 8-10 - Bits disponíveis para uso do software (normalmente ignorados).
    u64 physical_address: 36; // Bits 11-50 - Endereço base da tabela de páginas de nível 2 (PD).
    u64 reserved: 4; // Bits 51-61 - Reservados, não são usados no modo de paginação.
    u64 ignored3: 11; // Bit 62 - Se 0, a página pode ser executada por código; se 1, a execução é desabilitada (NX bit).
    u64 execute_disabled       : 1; // Bit 63 - Reservado para uso futuro.
} __attribute__((packed));

// Definição da estrutura para uma entrada na PD (Page Directory)
struct PDFlags {
    u64 present         : 1; // Bit 0 - Se 1, indica que a entrada está presente e válida.
    u64 read_write      : 1; // Bit 1 - Se 0, a página associada é somente leitura; se 1, é leitura e escrita.
    u64 user_supervisor : 1; // Bit 2 - Se 0, a página só pode ser acessada em modo supervisor (kernel); se 1, também em modo usuário.
    u64 write_through   : 1; // Bit 3 - Se 0, escritas na página não são escritas de volta no cache; se 1, são escritas de volta.
    u64 cache_disabled  : 1; // Bit 4 - Se 0, a página é cacheable; se 1, a página é não-cacheable.
    u64 accessed        : 1; // Bit 5 - Indica se a entrada foi acessada (geralmente é gerenciado pelo hardware).
    u64 ignored1        : 1; // Bit 6 - Reservado, não é usado no modo de paginação.
    u64 page_size            : 1; // Bit 7 - Se 0, a entrada aponta para uma tabela de páginas (PT); se 1, aponta para uma tabela de páginas grande (PS).
    u64 ignored2      : 4; // Bits 8-10 - Bits disponíveis para uso do software (normalmente ignorados).
    u64 physical_address: 36; // Bits 11-50 - Endereço base da tabela de páginas (PT) ou tabela de páginas grandes (PS).
    u64 reserved        : 4; // Bits 51-61 - Reservados, não são usados no modo de paginação.
    u64 ignored3: 11; // Bit 62 - Se 0, a página pode ser executada por código; se 1, a execução é desabilitada (NX bit).
    u64 execute_disabled       : 1; // Bit 63 - Reservado para uso futuro.
} __attribute__((packed));

// Definição da estrutura para uma entrada na PT (Page Table)
struct PTFlags {
    u64 present         : 1;
    u64 read_write      : 1;
    u64 user_supervisor : 1;
    u64 write_through   : 1;
    u64 cache_disabled  : 1;
    u64 accessed        : 1;
    u64 dirty           : 1;
    u64 pat             : 1;
    u64 global          : 1;
    u64 ignored2       : 3;
    u64 physical_address: 36;
    u64 reserved         : 4;
    u64 ignored3 : 7;
    u64 protection_key  : 4;   // campo de proteção de chave
    u64 execute_disabled: 1;
} __attribute__((packed));


union PML4Entry {
  unsigned long flags; // todos os bits
  PML4Flags flag_bits; // bit a bit
} __attribute__((__packed__));

union PDPTEntry {
  unsigned long flags;
  PDPTFlags flag_bits;
} __attribute__((__packed__));

union PDEntry {
  unsigned long flags;
  PDFlags flag_bits;
} __attribute__((__packed__));

union PTEntry {
  unsigned long flags;
  PTFlags flag_bits;
} __attribute__((__packed__));

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
          kfree(this->chunk);
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

extern Vector<PhysicalPage> ppages_in_use;

PhysicalPage kmmap(u64 size, u64 start_virtual_address);

}
#endif
