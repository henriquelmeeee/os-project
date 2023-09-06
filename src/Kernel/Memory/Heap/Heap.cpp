#include "../../Utils/Base.h"
#include "../Base_Mem.h"

#include "../../panic.h"

#include "Heap.h"

PhysicalRegion regions[512] = {0,0};
Arena arenas[32];

PhysicalRegion kmmap(u64 size) {
  for(int i = 0; i<512; i++)
    if(regions[i].start_address == 0) {
      // TODO precisamos de um algoritmo melhor pra cá, mas por enquanto, esse está bom
      // porque ainda não usaremos mais de uma arena
      // e por enquanto vamos começar no endereço fixo 500MB mesmo
      regions[i].start_address = 500000000;
      regions[i].end_address = regions[i].start_address + size;
      return regions[i];
    }

  throw_panic(0, "TEMPORARIO TODO: NENHUMA REGIAO DISPONIVEL");
  return {};
}

u64 number_of_chunks;

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

bool kfree(void* chunk_addr) {
  throw_panic(0, "kfree() not implemented yet");
  return false;
}

