#pragma once

#ifndef __FS
#define __FS

#define EXT2_PARTITION_START 1000
#define EXT2_SUPERBLOCK_START 2

#include "../Utils/Base.h"
#include "../Drivers/Disk.h"

#define BLOCK_SIZE 4096

struct ext2_super_block {
    u32 s_inodes_count;
    u32 s_blocks_count;
    u32 s_r_blocks_count;
    u32 s_free_blocks_count;
    u32 s_free_inodes_count;
    u32 s_first_data_block;
    u32 s_log_block_size;
    i32 s_log_frag_size;
    u32 s_blocks_per_group;
    u32 s_frags_per_group;
    u32 s_inodes_per_group;
    u32 s_mtime;
    u32 s_wtime;
    u16 s_mnt_count;
    i16 s_max_mnt_count;
    u16 s_magic;
    // ... e mais campos
    volatile u8 s_padding[BLOCK_SIZE];
} __attribute__((packed));

struct ext2_inode {
    u16 i_mode;
    u16 i_uid;
    u32 i_size;
    u32 i_atime;
    u32 i_ctime;
    u32 i_mtime;
    u32 i_dtime;
    u16 i_gid;
    u16 i_links_count;
    u32 i_blocks;
    u32 i_flags;
    u32 i_osd1;
    u32 i_block[15];
    u32 i_generation;
    u32 i_file_acl;
    u32 i_dir_acl;
    u32 i_faddr;
    u8  i_osd2[12];
    volatile char padding[BLOCK_SIZE];
} __attribute__((packed));

struct ext2_dir_entry {
    u32 inode;
    u16 rec_len;
    u8  name_len;
    u8  file_type;
    volatile char name[BLOCK_SIZE];  // Tamanho variável
} __attribute__((packed));

struct ext2_group_desc {
  u32 bg_block_bitmap;
  u32 bg_inode_bitmap;
  u32 bg_inode_table; // bloco onde a tabela de inodes começa neste grupo de blocos
  u16 bg_free_blocks_count;
  u16 bg_free_inodes_count;
  u16 bg_used_dirs_count;
  u16 bg_pad;
  u32 bg_reserved[3];
  volatile char padding[BLOCK_SIZE];
} __attribute__((packed));

inline u32 block_to_sector(u32 block, u32 entry = 1000) {
    dbg("block %d to sector: %d\nm_block_size: %d\n", block, ((block*4096)/512) + entry, 4096);
    return ((block * 4096) / 512) + entry;
}

class FILE {
  public:
    const char* m_absolute_path;
    enum {
      Directory,
      RegularFile,
    } m_file_type;

    unsigned char* m_raw_data;

    FILE(unsigned char* raw_data) : m_raw_data(raw_data) {

    }
};


class FS {
  public:
    ext2_inode m_root_inode;
    ext2_super_block m_sb;
    ext2_group_desc m_root_inode_group_desc;

    u32 m_inode_size = 128;

    void __read_block(void* write_back, u64 block, u32 group_block_index=0) {
      // vamos ignorar o "group_block_index" por enquanto
      char* _write_back = (char*)write_back;
      u32 block_sector_start = EXT2_PARTITION_START;
      u32 block_sector = block_sector_start + (block * BLOCK_SIZE / 512);
      read_from_sector(_write_back, block_sector); // hard-coded para 4096
      read_from_sector(_write_back+512, block_sector+1);
      read_from_sector(_write_back+1024, block_sector+2);
      read_from_sector(_write_back+1536, block_sector+3);
      read_from_sector(_write_back+2048, block_sector+4);
      read_from_sector(_write_back+2560, block_sector+5);
      read_from_sector(_write_back+3072, block_sector+6);
      read_from_sector(_write_back+3584, block_sector+7);
      return;
    }

    ext2_dir_entry __found_entry_in_dirs(ext2_inode inode, const char* entry);

    unsigned char* __read_regular_file_data(ext2_inode inode) {
      // TODO FIXME temporariamente iremos alocar fixamente um buffer
      // que cobre todos 12 blocos usando o i_size do file
      //unsigned char* buffer = (unsigned char*)kmalloc(BLOCK_SIZE * 12);
      unsigned char* buffer = (unsigned  char*)kmalloc(inode.i_size);
      dbg("__read_regular_file: size: %d", inode.i_size);
      for(int i = 0; i<12; i++) {
        if(inode.i_block[i] == 0)
          return buffer;
        __read_block(buffer+i*BLOCK_SIZE, inode.i_block[i]);
        dbg("__read_regular_file: block: %d", inode.i_block[i]);
        //dbg("__read_regular_file_data: ++i\ni anterior: %d", i);
      }
      dbg("Ext2FS: Aviso: parecem existir blocos indiretos");
      return buffer;
    }

    void __read_inode(void* write_back, u32 inode_index, ext2_group_desc ___gp_desc={0}/*deprecated*/) {
      --inode_index;

      u32 inodes_per_block = BLOCK_SIZE / 256; // 256bytes = 1 inode
      u32 inode_group = inode_index / inodes_per_block;
#if 0 
      ext2_group_desc gp_desc;
      __read_block((void*)&gp_desc, m_group_desc_table_block + inode_group);
#endif
      u32 inode_table = ___gp_desc.bg_inode_table;
      u32 inode_block = inode_table  + ((inode_index) / inodes_per_block);

      char read_buffer[BLOCK_SIZE];
      // inode_block - 1 porque o __read_block ignora o primeiro bloco por padrão TODO FIXME tirar isso! tirar o +8 do read_block
      __read_block((void*)read_buffer, inode_block);

      u32 offset_within_block = (inode_index % inodes_per_block) * 256;
      __builtin_memcpy((char*)write_back, read_buffer + offset_within_block, 256);
      dbg("ext2fs: inode lido\ninode_table: %d\ninode_block: %d\noffset: %d", inode_table, inode_block, offset_within_block);
    }
    
    void __read_fs_metadata() {
      dbg("ext2fs: lendo metadados");
      u16 sb_sector = EXT2_PARTITION_START + 2;

      read_from_sector((char*)&m_sb, sb_sector);
      read_from_sector(((char*)&m_sb)+512, sb_sector+1);

      if(m_sb.s_magic != 61267) {
        throw_panic(0, "magic number invalido ext2fs");
      }

      u32 first_group_desc = m_sb.s_first_data_block;
      
      __read_block((void*)&m_root_inode_group_desc, first_group_desc+1);
      if(m_root_inode_group_desc.bg_free_blocks_count == 0) {
        dbg("m_root_inode_group_desc.bg_free_blocks_count == 0");
        while(true);
      }

      u32 inode_table_start = m_root_inode_group_desc.bg_inode_table;
      if(inode_table_start == 0) {
        dbg("inode_table_start 0");
        while(true);
      }
      dbg("inode_table_start %d", inode_table_start);
      u32 root_inode_index = 2;
      __read_inode((void*)&m_root_inode, root_inode_index, m_root_inode_group_desc);
      dbg("Ext2FS: %d inodes, %d blocos", m_sb.s_inodes_count, m_sb.s_blocks_count);
      dbg("m_root_inode.i_blocks = %d",m_root_inode.i_blocks);
    }

    FS() {
      __read_fs_metadata();
      return;
    }

    void init() {__read_fs_metadata();}

    typedef void (*each_dir_entry_CallbackFunc) (u32, ext2_dir_entry);

    void for_each_dir_entry(each_dir_entry_CallbackFunc callback, ext2_inode inode) {
      for(int i = 0; inode.i_block[i] != 0; i++) {
        ext2_dir_entry direntry;
        __read_block((void*)&direntry, inode.i_block[i]);
        callback(i, direntry);
      }
    }

    FILE* fopen(const char* path);
    Memory::Vector<char*> list_dir(const char* path);
};

#endif
