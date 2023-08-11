#pragma once

#ifndef FS_H
#define FS_H
#include "../Utils/Base.h"
struct Binary {
  //u16 magic_number;
  u32 text_section_offset;
  u32 data_section_offset;
  //u32 text_section_size;
  //u32 data_section_size;
}; 
namespace FS {
  bool map_files_on_boot();

  Binary* LoadBinary(const char *fs_binary_location);
}
#endif
