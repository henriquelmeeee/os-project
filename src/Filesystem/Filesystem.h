#pragma once

#ifndef FS_H
#define FS_H

#include "../Utils/Base.h"
#include "../Memory/Base_Mem.h"
#include "../Drivers/Disk.h"
#include "../Drivers/VIDEO/preload.h"


struct Binary {
  //u16 magic_number;
  u32 text_section_offset;
  u32 data_section_offset;
  //u32 text_section_size;
  //u32 data_section_size;
}; 

struct Superblock {
  u16 sectors[32];
};

struct Inode {
  u32 superblock;
  char name[32];
  char reserved[476];
} __attribute__((packed));

class FS {
  public:
    Memory::Vector<Inode> inodes;
    u32 total_inodes_amount;
    Superblock superblock;

    Inode inode_find_by_name(const char* name) {
      for(int i = 0; i<total_inodes_amount; i++) {
        //if(__builtin_strcmp(inodes[i].name, name) == 0)
          //return inodes[i];
      }

      return {};
    }

    FS() {
      read_from_sector((char*)&total_inodes_amount, 200);
      Text::Write("Filesystem: Inode count read is ", 2);
      char count[16];
      itos(total_inodes_amount, count);
      Text::Write(count);
      Text::NewLine();

      for(int i = 0; i<total_inodes_amount; i++) {
        Inode inode = {};
        read_from_sector((char*)&inode, 200+i); // FIXME 201+1 instead of 200+i
        Text::Write("-> File found: ");
        Text::Write(inode.name);
        __asm__ volatile("hlt");
        //inodes.append(inode);
      }

      Text::Write("Filesystem: Inodes mapped", 2);
      Text::NewLine();
    }
};

#endif
