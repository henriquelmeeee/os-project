#pragma once

#ifndef FS_H
#define FS_H
#include "../Utils/Base.h"
namespace FS {
  bool map_files_on_boot();

  Binary* LoadBinary(const char *fs_binary_location);
}
#endif
