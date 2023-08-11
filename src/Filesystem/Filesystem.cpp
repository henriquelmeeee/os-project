#include "../Drivers/Disk.h"
#include "../Utils/Base.h"
#include "../Memory/Base_Mem.h"
#include "../Memory/Heap/Heap.h"
#include "Filesystem.h"

#define FIRST_SECTOR_FOR_METADATA 300
#define FIRST_SECTOR_FOR_USER_DATA 400
#define MAX_SECTORS 96

struct GenericFile { // essa é a mesma estrutura presente no disco METADATA
  unsigned int sectors[MAX_SECTORS];
  char* name;
};

Memory::Vector<struct GenericFile> files;
unsigned int number_of_files = 0;

namespace FS{

  /*
void append_to_sector_array(struct GenericFile* file, unsigned long sector) {
  for(int i = 0; i<MAX_SECTORS; i++) {
    if(file->sectors[i] == 0) {
      file->sectors[i] = sector;
      break;
    }
  }
}

void add_file_metadata(struct GenericFile file) {
  // TODO adicionar novo arquivo nos setores de metadata
}*/

bool map_files_on_boot() {
  for(int i = FIRST_SECTOR_FOR_METADATA; ; i++) {
    GenericFile file_buffer; 
    read_from_sector((char*)&file_buffer, (u32)i);
    if(file_buffer.sectors[0] != 0) { // caso existir um metadado de arquivo nesse setor
      dbg("map_files_on_boot()-> Arquivo encontrado\n");
      files[number_of_files] = file_buffer;
      ++number_of_files;

    } else { // TODO checar se não estamos no setor 400 já.
      if(number_of_files == 0)
        return false;
      break;
    }
  }
  return true;
}

GenericFile get_file_metadata(const char *fs_binary_location) {
  for(int _files = 0; _files<number_of_files; _files++) {
    //if(kstrcmp(files[_files].name, fs_binary_location)) {
    if(true) {  
      return files[_files];
    }
  }
  return {};
}

Binary* buf;

Binary* LoadBinary(const char *fs_binary_location) {
  //GenericFile file_metadata = get_file_metadata(fs_binary_location);
  // TODO ler todos setores usando o file_metadata, essa função é responsável por colocar o binário no kernel temporariamente 
  // TODO usar a heap ao invés de variavel global
  read_from_sector((char*)buf, 500);
  return buf;
}
/*bool create_file(char *bytes, unsigned long long file_size, char *name) {
  unsigned int last_sector = 0;
  if(number_of_files != 0) {
    struct GenericFile last_file = files[number_of_files-1];
    for(int i = 0; ; i++) {
      if(last_file.sectors[i] == 0)
        last_sector = (last_file.sectors[--i]) + 1; // iremos usar o próximo setor
    }
  } else {
    last_sector = FIRST_SECTOR_FOR_USER_DATA; // iremos usar o primeiro setor, já que nem ele está sendo utilizado (primeiro arquivo provavelmente)
  }
  struct GenericFile file_buffer = {};

  short *bytes_buffer = bytes; // original byte address
  unsigned int actual_sector = 0;
  
  for(int sector = 0; (file_size / 512) <= sector; sector++) {
    Drivers::Disk::write_to_sector(bytes, (last_sector + sector));
    bytes+=512;
    actual_sector = sector+last_sector;
    append_to_sector_array(&file_buffer, actual_sector);
  }
  kmemcpy(file_buffer->name, name, 16);
  ++number_of_files;
  files[number_of_files] = file_buffer;
}}*/
}
