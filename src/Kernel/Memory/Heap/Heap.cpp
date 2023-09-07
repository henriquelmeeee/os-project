#include "../../Utils/Base.h"
#include "../Base_Mem.h"

#include "../../Core/panic.h"

#include "Heap.h"

PhysicalRegion regions[512] = {0,0};
Arena* arenas;
u16 g_arenas_count = 0;

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
  dbg("kmaalloc!\n");
  i8 arena_to_use = -1;
  for(int i = 0; i<32; i++)
    if(! (arenas[i].is_full()) ) {
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

void dump_kernel_heap() {
  // Precisamos iterar sobre todas as arenas através da lista encadeada
  // seguindo ponteiros "next"
  // para evitar problemas, também não será usado um vetor para as arenas encontradas
  // TODO FIXME esse bgl de ArenaHeader* ta meio confuso, ele fica dentro de um objeto Arena 
  // e deveria ter um ponteiro para o inicio da arena de fato, e nao um offset
  ArenaHeader* actual_arena = &(arenas->m_header);
  Chunk* actual_chunk = (Chunk*) ( (actual_arena)+(arenas->m_header.offset_first_chunk) );
  dbg("Kernel Heap Dump\n");
  dbg("Size\t\tAddress\t\tArena\n");
#if 0
  while(true) {
    if(!(actual_chunk->exists)) {
      actual_arena = actual_arena->m_header.next;
      if(actual_arena != nullptr)
        actual_chunk = (Chunk*)actual_arena + actual_arena->m_header.offset_first_chunk;
      else
        break;
      continue;
    }
    dbg("%dB\t\t(todo ptr)\t\t(todo ptr)", actual_chunk->size);
    Chunk* fd = actual_chunk + actual_chunk->size;


  }
#endif
}
