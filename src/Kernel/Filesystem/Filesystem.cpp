#include "../Utils/Base.h"
#include "Filesystem.h"

#if 0
ext2_inode __found_dir_in_inode(ext2_inode inode) {
  ext2_dir_entry current_entry;
  for(int i = 0; i<12; i++) {
    __read_block()
  }
}
#endif

ext2_dir_entry FS::__found_entry_in_dirs(ext2_inode inode, const char* entry) {
  for(int block_index = 0; block_index<12; block_index++) {
    u32 offset = 0;
    char dir_entry_buffer[BLOCK_SIZE];
    __read_block((void*)dir_entry_buffer, inode.i_block[block_index]-1);
    ext2_dir_entry to_ret = {};
    while(offset < BLOCK_SIZE) {
      ext2_dir_entry* current_entry = (ext2_dir_entry*)(dir_entry_buffer+offset);
      if(kstrcmp((const char*)current_entry->name, entry) == 0)
        return *current_entry;
      offset += current_entry->rec_len;
    }
  }
  return {0};
}

#define EXT2_REGULAR_FILE 1

FILE* FS::fopen(const char* path) {
  dbg("Ext2FS: Abrindo arquivo no kernel: %s", path);

  ext2_inode current_inode = m_root_inode;
  Memory::Vector<const char*> entries;
  entries[0] = "initd"; // temporariamente hard-coded
  entries[1] = nullptr;

  for(int i = 0; entries[i] != nullptr; i++) {
    ext2_dir_entry entry = __found_entry_in_dirs(current_inode, entries[i]);
    if(entry.inode == 0) {
      dbg("Ext2FS: arquivo %s não encontrado (entry.inode==0)", entries[i]);
      return 0;
    }
    dbg("Ext2FS: entries[%d] encontrado", i);
    if(entry.file_type == EXT2_REGULAR_FILE) {
      dbg("Ext2FS: era o último componente do path; encontrado arquivo %s", entries[i]);
      unsigned char* __raw_data = __read_regular_file_data(current_inode);
      FILE* to_ret = (FILE*)kmalloc(sizeof(FILE));
      __builtin_memcpy((char*)to_ret->m_raw_data, (char*)__raw_data, BLOCK_SIZE*12);
      to_ret->m_raw_data = __raw_data;
      //while(true); // temporário
      return to_ret;
    } else {
      // TODO checar por outros tipos de arquivos
      __read_inode(&current_inode, entry.inode, m_root_inode_group_desc);
      // FIXME: é realmente necessário achar o group desc de um inode index?
      // não seria possível fazer isso diretamente no __read_inode apenas calculando baseado no inode index,
      // que não é relativo ao seu grupo de blocos mas sim ao sistema em geral?
      continue;
    }
  }
  dbg("nada encontrado");
  return nullptr;
  //kfree((void*)entries[0]);

  }

// UM unico bloco no i_block pode ter mais de um dir_entry
// isso seria considerado no for_each_dir_entry()

Memory::Vector<char*> FS::list_dir(const char* path) {
  dbg("Ext2FS: Listando %s", path);
  Memory::Vector<const char*> entries;
  Memory::Vector<char*> to_ret;
  entries[0] = "diretorio";
  entries[1] = nullptr;

  ext2_inode current_inode = m_root_inode;
  ext2_dir_entry current_dir_entry;
  // TODO n seria melhor o "to_ret" ter os inodes dos arquivos encontrados ao inves de seus nomes? fazer
  
  // primeiro, precisamos encontrar o inode do ultimo componente do path
  for(int i = 0; entries[i] != nullptr; i++) {
    current_dir_entry = __found_entry_in_dirs(current_inode, entries[i]);
    if(current_dir_entry.inode == 0) {
      dbg("Ext2FS: pasta %s não encontrada", entries[i]);
      while(true);
    }
    __read_inode((void*)&current_inode, current_dir_entry.inode, m_root_inode_group_desc);
  }

  u32 offset = 0;
  for(int x = 0; x<12; x++) {
// Aqui nós reusamos o "current_dir_entry"
    // TODO for_each_dir_entry()
    __read_block((void*)&current_dir_entry, current_inode.i_block[x]-1);
    while(offset < BLOCK_SIZE) {
      ext2_dir_entry* _current_dir_entry = (ext2_dir_entry*) (((char*)&current_dir_entry) + offset);

      if(_current_dir_entry->inode == 0) {
        dbg("ultimo dir_entry no bloco %d encontrado", x);
        while(true);
      } else {
        dbg("entrada encontrada: %s", _current_dir_entry->name);
        offset+=_current_dir_entry->rec_len;
      }
    }

  }

  dbg("finalizado");
  while(true);
}

#if 0
  ext2_inode current_inode = m_root_inode;
  char path_entry_buffer[32]; // FIXME TODO remover buffer overflow depois
  int x = 0;
  for(int i = 0; path[i] != '\0'; i++) {
    char current_char = path[i];
    if(current_char == '/') {
      x = 0;
      current_inode = __found_dir_in_inode(current_inode);
      for(int z = 0; z<32; z++)
        path_entry_buffer[z] = 0;
    }
    path_entry_buffer[x] = current_char;
    x++;
  }
#endif
  

