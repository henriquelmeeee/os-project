#pragma once

#ifndef FS_H
#define FS_H

#include "../Utils/Base.h"
#include "../Memory/Base_Mem.h"
#include "../Drivers/Disk.h"
#include "../Drivers/VIDEO/preload.h"

#define EXT2_PARTITION_START 1000
#define EXT2_SUPERBLOCK_START 2

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
    volatile u8 s_padding[940]; // Espaço para preencher até 1024 bytes
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
    volatile char padding[512 - sizeof(u16)*6 - sizeof(u32)*28 - sizeof(u8)*12];
} __attribute__((packed));

struct ext2_dir_entry {
    u32 inode;
    u16 rec_len;
    u8  name_len;
    u8  file_type;
    volatile char name[512 - 8 - 8 - 16 - 32];  // Tamanho variável
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
  volatile char padding[512-256 + 512]; // FIXME padding apenas com 512-256 tá dando overflow
} __attribute__((packed));

class FS {
  public:
    ext2_super_block m_sb;
    ext2_dir_entry m_dir_entry;
    ext2_inode m_root_inode;
    ext2_inode m_root_inode_table;
    ext2_group_desc m_group_desc;

    u32 m_entry = EXT2_PARTITION_START;

    u32 root_inode_table_sector;
    u32 inode_table_start_block;

    u32 m_inode_size = 128;
    u32 m_block_size = 4096;

    u32 m_first_block_group_descriptor_sector;
    
    inline u32 block_to_sector(u32 block) {
      dbg("block %d to sector: %d\nm_block_size: %d\n", block, ((block*m_block_size)/512) + m_entry, m_block_size);
      return ((block * m_block_size) / 512) + m_entry;
    }

    bool read_inode(u32 inode_number, ext2_inode* inode) {
      u32 offset_within_table = (inode_number - 1) * m_inode_size;
      u32 sector_to_read = root_inode_table_sector + (offset_within_table/512);
      u32 offset_within_sector = offset_within_table % 512;

      char buffer[512];
      read_from_sector(buffer, sector_to_read);

      __builtin_memcpy((char*)inode, (char*) (buffer+offset_within_sector), sizeof(ext2_inode));

      return true;
    }

    FS(u32 entry_sector = EXT2_PARTITION_START) {
      dbg("Criando novo sistema de arquivos Ext2FS\n");
      
      this->m_entry = entry_sector;
      u32 superblock_sector = entry_sector + 2;

      dbg("superblock_sector: %d\nm_entry: %d\n", superblock_sector, m_entry);

      read_from_sector((char*)&m_sb, superblock_sector);
      read_from_sector(((char*)&m_sb)+512, superblock_sector+1);
      dbg("magic sb: %d\n", m_sb.s_magic);
      if(m_sb.s_magic != 61267) {
        throw_panic(0, "Invalid magic number in ext2fs");
      } 
      dbg("m_sb.s_first_data_block: %d\nm_sb.s_inodes_count: %d\n", m_sb.s_first_data_block, m_sb.s_inodes_count);

      m_first_block_group_descriptor_sector = m_sb.s_first_data_block + 1;
      read_from_sector((char*)&m_group_desc, block_to_sector(m_first_block_group_descriptor_sector));


      dbg("Calculando setor para 'm_root_inode'\n");

      m_inode_size = 128;
      m_block_size = 4096;

      root_inode_table_sector = 0;
      inode_table_start_block = m_group_desc.bg_inode_table; // bloco para o inode table
      root_inode_table_sector = (inode_table_start_block * 4096) / 512;
      root_inode_table_sector += 1000;

      read_from_sector((char*)&m_root_inode, root_inode_table_sector);
      m_root_inode_table = m_root_inode;

      u32 root_inode_index = 2;

      char* root_inode_tmp = (char*) &m_root_inode;
      root_inode_tmp+=(root_inode_index*m_inode_size);
      m_root_inode = *((ext2_inode*) root_inode_tmp);

      dbg("Sistema de arquivos montado com sucesso!\n");
      dbg("M_ENTRY %d\n", m_entry);
      dump_root_files();
      list_directory("/diretorio");
      
    }

    bool read_inode_block(char* dst, u32 block) {
      // TODO cache em memória baseado no bloco ja lido
      read_from_sector(dst, block_to_sector(block));
      return true;
    }

    void dump_root_files() {
      Text::Writeln("Ext2FS: Listing /", 0xb);
      char buffer[m_block_size];
      read_from_sector(buffer, block_to_sector(m_root_inode.i_block[0]));

      u32 offset = 0;
      while(offset < m_block_size) {
        ext2_dir_entry* entry = (ext2_dir_entry*)(buffer+offset);
        if(entry->rec_len > 0) {
          Text::Write("File: ", 0xb);
          Text::Writeln((const char*)entry->name);
          offset += entry->rec_len;
        } else {
          break;
        }
      }
    }

    void list_directory(const char* path) {
      char directories_to_list[6][32] = {{0}};
      int x = -1;
      int y = 0;
      char c;
      int dirs_to_list = 0;
      for(int i = 0; path[i]!=0; i++) {
        c = path[i];
        if(c == '/') {
          x++;
          y = 0;
          ++dirs_to_list;
        } else {
          directories_to_list[x][y] = c;
          ++y;
        }
      }
      if(c == '/') --dirs_to_list;

      dbg("directory /%s\n", (char*)directories_to_list[0]);
      dbg("dirs_to_list %d\n", dirs_to_list);

      ext2_inode current_inode = m_root_inode;
      char buffer[m_block_size];

      u32 actual_dir = 0;

      while(actual_dir < dirs_to_list) {
        read_from_sector(buffer, block_to_sector(current_inode.i_block[0]));

        u32 offset = 0;
        while(offset < m_block_size) {
          ext2_dir_entry* entry = (ext2_dir_entry*)(buffer + offset);

          if(entry->rec_len > 0) {

            if(kstrncmp((const char*)entry->name, (const char*)directories_to_list[actual_dir], 32) == 0) {
              ++actual_dir;
              read_inode(entry->inode, &current_inode);

              read_from_sector(buffer, block_to_sector(current_inode.i_block[0]));

              offset = 0;

              if(actual_dir == dirs_to_list) {
                dbg("diretorio final encontrado\n");
                __asm__ volatile("hlt");
              }

            }
          }
          offset += entry->rec_len;

        }
      }
    }

    void* open(const char* path) {}


#if 0
      for(int direct_pointer = 0; direct_pointer < 12; direct_pointer++) {
        if(m_root_inode.i_block[direct_pointer] == 0) {
          continue;
        } else {
          ext2_dir_entry buffer[m_block_size];
          read_inode_block((char*)&buffer, m_root_inode.i_block[direct_pointer]);
          int offset = 0;

          while(offset < m_block_size) {
            ext2_dir_entry* dir_entry = (ext2_dir_entry*)(buffer+offset);
            dbg("file name: %s\n", dir_entry->name);
            offset += dir_entry->rec_len;
          }

        }
      }
#endif

};
#if 0
class FS {
  private:
    ext2_super_block sb;
    ext2_dir_entry dir_entry; // for root inode
    ext2_inode root_inode;
    ext2_group_desc group_desc;

    u32 root_inode_location = 0;
    u32 first_data_block = 0;
    u32 block_size = 4096;

    u32 m_inode_size = 128;
    u32 m_entry_sector = 0;

    bool calculate_inode_table_location() {
      u32 inode_table_start_block = group_desc.bg_inode_table;
#if 0
      if(m_entry_sector < 512) {
        throw_panic(0, "Ext2FS: Invalid 'm_entry_sector', expected >512, get <512");
      }
#endif
      root_inode_location = (inode_table_start_block * 4096) / 512;
      dbg("group_desc.bg_inode_table: %d\n", group_desc.bg_inode_table);
      dbg("inode_table sector: %d\n", root_inode_location);
      dbg("block_size %d\n", block_size /*sb.s_log_block_size*/);
      //root_inode_location = (root_inode_location + (/*m_entry_sector*/1000 * 512)) / 512;
      root_inode_location+=1000;
      dbg("inode_table sector + offset: %d\n", root_inode_location);
      return true;
    }

    bool calculate_bg_location() {
      u32 group_descriptor_block = sb.s_first_data_block + 1;
      u32 group_descriptor_offset = (group_descriptor_block * block_size) / 512;
      dbg("group_descriptor_block: %d\ngroup_descriptor_offset sector: %d\n", group_descriptor_block, group_descriptor_offset+m_entry_sector);
      read_from_sector((char*)&group_desc, group_descriptor_offset+m_entry_sector); 
      return true;
    }

    bool read_disk_block(u32 block_group_index, u32 block_number, void* buffer) {
      u32 offset = (block_number * block_size) + m_entry_sector;
      dbg("Ext2FS::read_disk_block() offset = %d\n", offset);
      for(int i = 0; i < block_size; i+=512) {
        read_from_sector(((char*) buffer)+i, (offset+i)/512);
      }
      return true;
    }

  public:

    FS(u32 entry_sector = EXT2_PARTITION_START) {
      /*
       Primeiro bloco (0-4096): superbloco (começa no offset 1024)
       Segundo bloco (4096-8096): struct para o primeiro grupo de blocos
       Cada entrada no inode_table tem 128 bytes de tamanho
       e o root_inode normalmente é a segunda entrada nesta tabela
      */

      this->m_inode_size = 128;

      if(entry_sector == EXT2_PARTITION_START)
        dbg("Montando sistema de arquivos principal (ext2fs)\n");
      else
        dbg("Montando novo sistema de arquivos em %d\n", entry_sector);
      this->m_entry_sector = entry_sector;
      dbg("m_entry_sector: %d\n", m_entry_sector);

      read_from_sector((char*)&sb, entry_sector+2); // superblock
      this->block_size = 1024 << (sb.s_log_block_size);
      dbg("block_size %d\n", block_size /*sb.s_log_block_size*/);
      read_from_sector(((char*)&sb)+512, entry_sector+3); // superblock

      calculate_bg_location();
      calculate_inode_table_location();

      // Agora que temos o setor da tabela de inodes,
      // precisamos reajustar "root_inode" para conter os valores de first_inode_table[1]

      read_from_sector((char*)&root_inode, root_inode_location);
      u32 root_inode_index = 2;
      dbg("m_inode_size: %d\n", m_inode_size);
      u32 root_inode_offset = (root_inode_index) * 128;
      

      char* root_inode_as_ptr = (char*) &root_inode_location;
      root_inode_as_ptr+=root_inode_offset;
      ext2_inode* tmp = (ext2_inode*) root_inode_as_ptr;
      root_inode = (ext2_inode) *tmp; 

      if(root_inode.i_mode == 0) {
        throw_panic(0, "i_mode invalid (=0)");
      }
      for(int i = 0;;i++) {
        if(root_inode.i_block[i] != 0) {
          dbg("arquivo encontrado\nroot_inode.i_block[%d] = %d\n", i, root_inode.i_block[i]);
          __asm__ volatile("hlt");
        }
        if(i == 12) {
          throw_panic(0, "todo nenhum arquivo encontrado em root_inode");
        }
      }

      //first_data_block = root_inode.i_block[0];

      //read_disk_block(0, first_data_block, &dir_entry);

    }

    void* open(const char* path) {
      if(path[0] != '/' && false) {
        throw_panic(0, "invalid path todo remover esse panic e adicionar algo melhor tipo localizacao relativa");
      }

      if(root_inode.i_blocks < 1) {
        dbg("i_blocks < 1\n");
        return nullptr;
      }

      ext2_inode tmp = root_inode;
      for(int i = 0; i < 12; i++) {
        if(root_inode.i_block[i] != 0) {
          ext2_dir_entry block_data[block_size];
          read_disk_block(0, root_inode.i_block[i], (char*)block_data);
          dbg("entry name: %s\n", block_data->name);
          return nullptr;
        }
      }
      
      dbg("arquivo não encontrado (hard coded para um unico bloco para teste)\n");



      return nullptr;
    }
};
#endif
// OLD custom filesystem
#if 0
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
      Text::Writeln(count);

      for(int i = 0; i<total_inodes_amount; i++) {
        // HARDCODED !!!
        char inode_buffer[512];
        read_from_sector(inode_buffer, 201+i);
        Inode inode = {*((unsigned int*)inode_buffer), "teste"};
        Text::Write("-> File found: ");
        Text::Write(inode.name);
        __asm__ volatile("hlt");
        //inodes.append(inode);
      }

      Text::Writeln("Filesystem: Inodes mapped", 2);
    }
};
#endif
#endif
