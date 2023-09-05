//#pragma once
#ifndef HEAP_H
#define HEAP_H

#define HEADER_SIZE 16

struct Chunk {
  unsigned int size;
  bool exists;
  bool freed;
  struct Chunk* next_chunk;
  char a; // TODO só pra completar 16 bytes
  char b; // TODO só pra completar 16 bytes
};

extern unsigned long number_of_chunks;

void inline append_chunk(struct Chunk* addr);

extern "C" void* kmalloc(unsigned int size);

signed int kfind_chunk_by_addr(struct Chunk* chunk_addr);

bool kfree(void* chunk_addr);
#endif
