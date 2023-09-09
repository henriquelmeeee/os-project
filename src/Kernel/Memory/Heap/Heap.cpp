#include "../../Utils/Base.h"
#include "../Base_Mem.h"

#include "../../Core/panic.h"

#include "Heap.h"

// TODO nem todas variáveis abaixo estão sendo utilizadas
// estou fazendo uma implementação mais simples do kmalloc(), sem uso de arenas
// porque senão, se torna complexo atoa
// e a prioridade agora é ter alocação dinâmica funcionando



PhysicalRegion regions[512] = {0,0};
u16 g_arenas_count = 0;

#if 0
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
#endif

u64 number_of_chunks;
u64 number_of_free_chunks;

#define KERNEL_HEAP_START 500000000

void* kmalloc(u32 size = 32) {
  dbg("kmalloc()-> alocando %dB\n", size);
  Chunk* current_chunk = (Chunk*)KERNEL_HEAP_START;
  for(;;) {
    if((current_chunk->freed) && (current_chunk->size >= size) && (current_chunk->exists)) {
      current_chunk->freed = false;
      --number_of_free_chunks;
      ++number_of_chunks;
      dbg("kmalloc()-> encontrado chunk livre: 0x%p\n", (void*)((char*)current_chunk+sizeof(Chunk)));
      return (void*) ((char*)current_chunk+sizeof(Chunk));
    }
    if(!(current_chunk->exists)) {
      ++number_of_chunks;
      current_chunk->size = size;
      current_chunk->freed = false;
      current_chunk->exists = true;
      dbg("kmalloc()-> encontrado endereço disponível: 0x%p\n", (void*)((char*)current_chunk+sizeof(Chunk)));
      return (void*) ((char*)current_chunk+sizeof(Chunk));
    }
    // TODO cuidar de overflow 
    current_chunk = (Chunk*) ( ((char*)current_chunk) + current_chunk->size);
  }
}

bool kfree(void* chunk_addr) {

  Chunk* chunk = (Chunk*) ((char*)chunk_addr - sizeof(Chunk));
  if(!(chunk->freed)) {
    if(!(chunk->exists)) {
      dbg("kfree()-> AVISO: kfree() em %p mas essa chunk não existe!\n", chunk_addr);
      // TODO talvez um stack trace para o dbg() ?
    } else {
      chunk->freed = true;
      ++number_of_free_chunks;
      --number_of_chunks;
    }
  } else {
    throw_panic(0, "Double free on kernel-heap detected");
  }
  return true;
}

void dump_kernel_heap() {
  dbg("Dump da Heap\n");
  dbg("Endereço\t\tTamanho\n");
  Chunk* current_chunk = (Chunk*)KERNEL_HEAP_START;
  while(current_chunk->exists) {
    dbg("0x%p\t\t%d\n", (void*) (((char*)current_chunk)+sizeof(Chunk)), current_chunk->size);
    current_chunk = (Chunk*) (((char*)current_chunk) + current_chunk->size);
  }
}
