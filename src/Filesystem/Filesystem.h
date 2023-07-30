#ifndef FS_H
#define FS_H
#include "../Utils/Base.h"
namespace FS {

  struct Binary {
    u16 magic_number;
    u32 text_section_offset;
    u32 data_section_offset;
    u32 text_section_size;
    u32 data_section_size;
  };

  bool map_files_on_boot();

  Binary* LoadBinary(const char *fs_binary_location);
}
#endif
