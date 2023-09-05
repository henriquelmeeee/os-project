#pragma once
#ifndef HEAP_H
#define HEAP_H

#define HEADER_SIZE 16

#include "../../Utils/Base.h"
#include "../Base_Mem.h"

struct PhysicalRegion {
  u32 start_address;
  u32 end_address;
};

// TODO mudar kmmap() para outro arquivo em Memory/
// TODO fazer esse array de regiões de outro modo, tentando "burlar" o uso de Vector q depende da heap
PhysicalRegion regions[512] = {0,0};

PhysicalRegion kmmap(u64 size = 32) {
  for(int i = 0; i<512; i++) {
    if(regions[i].start_address != 0) {
      // TODO precisamos de um algoritmo melhor para cá, mas por enquanto, esse está bom
      // porque ainda não usaremos mais de uma arena
      // e por enquanto vamos começar no endereço fixo 500MB mesmo
      regions[i].start_address = 500000000;
      regions[i].end_address = regions[i].start_address + size;
      return regions[i];
    }
  }
  throw_panic(0, "TEMPORARIO TODO: nenhuma região disponível");
  return {};
}

struct ArenaHeader {
  PhysicalRegion region;
  u16 offset_first_chunk;
  u64 size;
};

struct Chunk {
  u32 size;
  bool exists;
  bool freed;
  Chunk* next_chunk;
  char a; // TODO só pra completar 16 bytes
  char b; // TODO só pra completar 16 bytes
};

class Arena {
  private:
    ArenaHeader m_start = {};
    bool m_full = false;
    
  public:
    Arena() {
      m_start.region = kmmap();
      dbg("Kernel: Heap: Created new arena @ %p\n", &m_start.region);
      m_start.size = m_start.region.end_address - m_start.region.start_address;
    }

    bool is_full() {
      return m_full;
    }

    Chunk* a_malloc(u32 size) {
      // Iterate over all chunks at this arena
      for(u64 i = (m_start.region.start_address)+sizeof(ArenaHeader); i!=m_start.region.end_address;) {
        Chunk* chunk = (Chunk*)i;
        if(chunk->exists && !(chunk->freed)) {
          i+=chunk->size;
          continue;
        }
        if(chunk->exists && chunk->freed)
          if(chunk->size >= size) {
            chunk->freed = false;
            return chunk;
          }
        if(!(chunk->exists)) {
          chunk->size = size;
          chunk->freed = false;
          return chunk;
        }
      }
      dbg("Warning: no free chunk found\n");
      return nullptr;
    }

};

extern unsigned long number_of_chunks;

Arena arenas[32];

bool initialize() {
  dbg("Kernel: Initializing heap\n");
  arenas[0] = Arena();
  return true;
}

Chunk* kmalloc(u32 size = 32) {
  i8 arena_to_use = -1;
  for(int i = 0; i<32; i++)
    if(!arenas[i].is_full()) {
      arena_to_use = i;
      break;
    }

  if(arena_to_use == -1) {
    throw_panic(0, "Disponible arena not found\n");
    return nullptr;
  }
  ++number_of_chunks;
  return arenas[arena_to_use].a_malloc(size);
}


//void inline append_chunk(struct Chunk* addr);

//signed int kfind_chunk_by_addr(struct Chunk* chunk_addr);

//bool kfree(void* chunk_addr);
#endif
