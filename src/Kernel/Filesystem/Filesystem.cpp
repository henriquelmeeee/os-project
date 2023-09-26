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

FILE* FS::fopen(const char* path) {
  dbg("Abrindo arquivo no kernel: %s", path);
  // TODO por enquanto, lemos apenas no root_inode!
  // TODO e vamos ignorar também blocos de ponteiro duplo/triplo
  /*ext2_dir_entry*/ char current_dir_entry[BLOCK_SIZE];
  ext2_inode found_inode;
  for(int i = 0; i<12; i++) {
    __read_block((void*)&current_dir_entry, m_root_inode.i_block[i]+3);
    ext2_dir_entry* _current_dir_entry = (ext2_dir_entry*)current_dir_entry;
    const char* name = (const char*) (&(_current_dir_entry->name));
    dbg("name: %s", name);
    if(kstrcmp(name, path) == 0) {
      dbg("path encontrado!");
      while(true);
      break;
    }
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
  
}
