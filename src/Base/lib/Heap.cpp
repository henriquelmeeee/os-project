#include <Utils/Base.h> // inclui kmemcpy()

#define HEAP_BASE_ADDRESS 0xC3000000
#define HEAP_MAX_ADDRESS 0xC4000000

struct Chunk {
  unsigned int size;
  char freed;
  struct Chunk* next_chunk;
};

struct Chunk* CHUNKS[255];
unsigned int number_of_chunks = 0;

void inline append_chunk(struct Chunk* addr) {
  CHUNKS[number_of_chunks] = addr;
}

void* malloc(unsigned int size) {
  struct Chunk new_chunk{size, 0, 0};
  struct Chunk* addr = number_of_chunks == 0 ? (struct Chunk*) HEAP_BASE_ADDRESS \
                       : (struct Chunk*)CHUNKS[number_of_chunks] + CHUNKS[number_of_chunks]->size + 8;
  if(number_of_chunks > 1) {
    CHUNKS[number_of_chunks-1]->next_chunk = addr;
  }
  kmemcpy(addr, &new_chunk, sizeof(struct Chunk));
  ++number_of_chunks;
  append_chunk(addr);

  return (void*)(addr + 8);
}

signed int find_chunk_by_addr(struct Chunk* chunk_addr) {
  for(int i = 0; i<number_of_chunks; i++) {
    if(CHUNKS[i] == chunk_addr)
      return i;
  }
  return -1;
}

bool free(void* chunk_addr) {
  signed int chunk_position = find_chunk_by_addr((struct Chunk*) chunk_addr - sizeof(struct Chunk));
  if(chunk_position == -1)
    return false;
  if( *( (char*) (chunk_addr) - 5) != 0) 
    return false;
  *((char*)chunk_addr-5) = 0xFF;
  struct Chunk* adjacent_chunk =  ((struct Chunk*)chunk_addr)->next_chunk;

  if(adjacent_chunk == 0 || find_chunk_by_addr(adjacent_chunk) == 0)
    return true;
  if(adjacent_chunk->freed > 0) {
    ((struct Chunk*)chunk_addr)->size += adjacent_chunk->size;
    // os metadados da chunk adjacente livre ainda estarão lá, mas se tornarão lixo
  }
  return true;
}
