#include "../../Utils/Base.h"
#include "../../panic.h"

#define HEAP_BASE_ADDRESS (unsigned long) (500*1024*1024) // 500MB
#define HEAP_MAX_ADDRESS (unsigned long) (1024*1024*1024) // 1GB

u64 number_of_chunks = 0;

void inline append_chunk(struct Chunk* addr) {
  //CHUNKS[number_of_chunks] = addr;
}

char itos_buffer[32];

extern "C" void* kmalloc(unsigned int size) {
  if(size > (TOTAL_RAM - mem_usage)) {
    return (void*)0;
  }
  char* addr = (char*)((unsigned long)HEAP_BASE_ADDRESS);
  struct Chunk* chunk_found;
  while(true) {
    struct Chunk* addr_buffer = (struct Chunk*)addr;
    if(( (unsigned long)addr_buffer>=HEAP_MAX_ADDRESS) || ((unsigned long)addr_buffer)+size>=HEAP_MAX_ADDRESS) {
          throw_panic(0, "Heap overflow detected"); // TODO heap region-based
    }
    if(addr_buffer->exists == 0) { // Caso não haja chunk aqui (byte 4 começando do zero)
      addr_buffer->exists = 1;
      addr_buffer->size = size;
      addr_buffer->freed = 0;
      chunk_found = (struct Chunk*)((unsigned long)addr_buffer);
      mem_usage+=size+HEADER_SIZE;
      break;
    } else {
      if((addr_buffer->freed) && (addr_buffer->size)>=size) { // TODO será q não precisa considerar o HEADER_SIZE?
        chunk_found = addr_buffer;
        addr_buffer->freed = 0;
        break;
      }
      addr+=addr_buffer->size+HEADER_SIZE;
    }
  }

  ++number_of_chunks;
  dbg("kmalloc()-> Nova chunk criada\n");
  dbg("kmalloc()-> Tamanho: ");
  itos((unsigned long)size, itos_buffer);
  dbg(itos_buffer);
  dbg(" (");
  itos((unsigned long)size+HEADER_SIZE, itos_buffer);
  dbg(itos_buffer);
  dbg(" se considerarmos o cabeçalho)");
  itos((unsigned long)addr, itos_buffer);
  dbgl;
  dbg("kmalloc()-> Endereço base: ");
  dbg(itos_buffer);
  dbgl;
  //return (void*)(addr + 8);
  return addr+HEADER_SIZE;
}

struct Chunk* kfind_chunk_by_size(u32 size) {
  for(struct Chunk* addr = (struct Chunk*)HEAP_BASE_ADDRESS;;) {
    if(!(addr->exists))
      dbg("kfind_chunk_by_size()-> Nenhuma chunk encontrada\n");
      break;

    if(addr->size == size) {
      dbg("kfind_chunk_by_size()-> Chunk encontrada\n");
      return addr;
    }

    addr+=addr->size;
    addr+=8;
  }
  return (struct Chunk*)0;
}

bool kfree(void* chunk_addr) {
  struct Chunk* chunk = (struct Chunk*) chunk_addr;
  chunk-=HEADER_SIZE;
  if(!(chunk->exists)) {
    dbg("kfree()-> Chunk inválida\n");
    return false;
  } else {
    itos((unsigned long)chunk, itos_buffer);
    dbg("kfree()-> Chunk ");
    dbg(itos_buffer);
    dbg(" liberada\n");
    chunk->freed = true;
    mem_usage-=(chunk->size-HEADER_SIZE);
    return true;
  }
}
