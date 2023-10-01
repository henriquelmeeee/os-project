#pragma once

#include "../kstd/stdlib.h"
#include "../Utils/Base.h"

struct Elf64_Ehdr {
  u8 e_ident[16];     // Identificador (magic number)
  u16 e_tpe;          // Por enquanto, aceitamos apenas executáveis
  u16 e_machine;
  u32 e_version;
  u64 e_entry;
  u64 e_phoff;        // Início do Program Header
  u64 e_shoff;        // Início dos section headers
  u32 e_flags;
  u16 e_ehsize;       // Tamanho do Elf Header
  u16 e_phentsize;    // Tamanho de uma única entrada no PHDR
  u16 e_phnum;        // Número de entradas no PHDR
  u16 e_shentsize;    // Tamanho de uma única entrada no SHDR
  u16 e_shnum;        // Número de entradas no SHDR
  u16 e_shstrndx;     // Índice da seção de strings 
                      // no cabeçalho de seção
} __attribute__((packed));

struct Elf64_Phdr {
  u32 p_type;         // Tipo de segmento
  u32 p_flags;
  u64 p_offset;       // Offset no arquivo para o segmento
  u64 p_vaddr;        // Endereço virtual esperado do segmento na memória
  u64 p_paddr;        // Irrelevante
  u64 p_filesz;       // Tamanho do segmento no arquivo
  u64 p_memsz;        // Tamanho do segmento na memória
  u64 p_align;
} __attribute__((packed));

class ElfImage {
  private:
  public:
    Elf64_Ehdr* m_elf_header;
    ElfImage() {}
    ElfImage(void* binary){
      dbg("ElfImage criado");
      m_elf_header = (Elf64_Ehdr*)binary;
      dbg("m_elf_header->e_phoff: %d", m_elf_header->e_phoff);
      
    }
    template<typename Func, typename... Args>
      void for_each_program_header(Func&& callback, Args... args) {
        Elf64_Phdr* current_phdr;
        u32 amount_of_program_headers = m_elf_header->e_phnum;
        u32 ph_offset = m_elf_header->e_phoff;

        dbg("amount_of_program_headers: %d", amount_of_program_headers);
        dbg("ph_offset: %d", ph_offset);

        current_phdr = (Elf64_Phdr*) ((char*)m_elf_header)+ph_offset;
        for(int i = 0; i<amount_of_program_headers; i++) {
          callback(current_phdr, args...);
          current_phdr = (Elf64_Phdr*) ((char*)current_phdr) + (m_elf_header->e_phentsize);
          return;
        }
        // Exemplo de uso:
        // Functor<int, int> callback(functionHere);
        // for_each_program_header(callback, 1, 2);
      }
};
