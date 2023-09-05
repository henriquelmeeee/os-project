#pragma once

#ifndef HEAP_H
#define HEAP_H

#define HEADER_SIZE 16

#include "../../Utils/Base.h"
#include "../../kstd/stdio.h"
#include "../Base_Mem.h"

#include "../../panic.h"

struct PhysicalRegion {
  u32 start_address;
  u32 end_address;
};

// TODO mudar kmmap() para outro arquivo em Memory/
// TODO fazer esse array de regiões de outro modo, 
// tentando "burlar" o uso de Vector q depende da heap 
// (talvez com lista encadeada usando kmmap() como
// uma forma rudimentar de vetor dinamico)
// TODO colocar todas definições em Heap.cpp
// TODO spinlock (ou talvez algo mais aprimorado pq as rotinas podem demorar)

extern PhysicalRegion regions[512];

PhysicalRegion kmmap(u64 size=32);

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

extern Arena arenas[32];

bool initialize();

Chunk* kmalloc(u32 size);

//void inline append_chunk(struct Chunk* addr);

//signed int kfind_chunk_by_addr(struct Chunk* chunk_addr);

bool kfree(void* chunk_addr);
#endif
