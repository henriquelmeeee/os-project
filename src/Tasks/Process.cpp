#include "../Filesystem/Filesystem.h"
#include "../Drivers/Disk.h"
#include "../Drivers/VIDEO/preload.h"
#include "../Utils/Base.h"
#include "../Memory/Base_Mem.h"
#include "../panic.h"
//#include "Process.h"

namespace Process{
unsigned long long TOTAL_RAM = 0;

#define vPage i16

enum state {
  RUNNING,
  PAUSED,
  ZOMBIE,
};


u64 amount_of_procs = 0;

class SysProc {
  public:
    char name[17]; // 17 pois precisamos incluir null byte
    u64 *ptr;
    u128 cycles_started;
    u128 cycles_finished;
    u32 pid;
    enum state p_state;

    FrameBuffer *framebuffer;

    vPage TextSection;
    vPage StackSection;
    
    struct PT pt;
    struct PD pd;
    struct PDPT pdpt;
    struct PML4 pml4;

    vPage proc_instructions; // proc instructions vPage

    SysProc(const char *p_name, const char *fs_binary_location, u32 pid) {
      //Utils::kmemcpy(name, p_name, 16);
      //unsigned long binary_size = FS::get_file_size_by_path(fs_binary_location);
      //unsigned long *get_addr_by_page = (pti[CODE_SEGMENT_PAGE]) * 4096; // pegamos o endereço do primeiro byte do segmento de código
      //(*get_addr_by_page) = FileSystem::read_bytes(fs_binary_location, binary_size);
      

      // Precisamos configurar a tabela de páginas para esse novo processo usando kmalloc()
      this->pml4.PDPTe = &(this->pdpt); // criar uma entrada na PML4 para um diretório de diretórios
      for(int pdir = 0; pdir<512; pdir++) { // todos ponteiros de diretórios apontarão para o mesmo diretório
        (pdpt).PDe[pdir] = &(this->pd);
      }
      for(int dir = 0; dir<512; dir++) { // todos diretórios apontarão para a mesma tabela de páginas
        (pd).PTe[dir] = &(this->pt);
      }

      /*
       * É aqui que a diversão começa
       * nós precisamos alocar a região de código, de dados e da stack na Heap
       * e então pegar o endereço resultante e colocar em cada página
      */

      /*
      
      //TODO kmalloc_alligned
      this->TextSection = Memory::CreateNewPage(); // TODO tornar isso dinâmico baseado no tamanho da .text; (página 50, 100MB)
      this->StackSection = Memory::CreateNewPage(); // A stack começa com 2MB (página 499, 998MB)
      
      unsigned long StackSectionPhysicalAddr = ((unsigned long)StackSection / PAGE_SIZE);
      unsigned long TextSectionPhysicalAddr = ((unsigned long)TextSection / PAGE_SIZE);
      dbg("Process::constructor-> Inicializando páginas para processo novo\n");
      (pt).Pe[50] = (struct Memory::PageFlags) {
        .present = 1,
        .writeable = 0,
        .user_access = 1,
        .write_through = 0,
        .cache_disabled = 1,
        .accessed = 0,
        .size = 0,
        .page_ppn = TextSectionPhysicalAddr,
        .execution_disabled = 0,
      };
      (pt).Pe[499] = (struct P){
            .Present = 1,
            .Writable = 1,
            .User = 1,
            .WriteThrough = 0,
            .CacheDisabled = 0,
            .Accessed = 0,
            .Dirty = 0,
            .PAT = 0,
            .Global = 0,
            .Ignored = 0,
            .Frame = StackSectionPhysicalAddr,
            .Available = 1,
            .NX = 0,
      };
      char CodeBuffer[1 * PAGE_SIZE];
      read_from_sector(CodeBuffer, 400);
      Utils::kmemcpy((char*)TextSection, CodeBuffer, (unsigned long)1 * PAGE_SIZE);
      throw_panic(0, "teste");
      __asm__ volatile("jmp *%0\n" : : "r" (TextSection));
      */
      this->pid = pid;
    }
};

Memory::Vector<SysProc> procs;
SysProc* proc_current = 0;

class Page {
  public:
      SysProc* proc;
      u64 page_number;
      u64 base_addr;

      Page(SysProc* p, u64 b_addr) {
        this->proc = p;
        this->page_number = b_addr / PAGE_SIZE;
        this->base_addr = b_addr;
      };
};
Memory::Vector<Page> pages_in_use;

bool init() {
  // -----
  // Inicializando primeiro processo
  // -----
  procs = Memory::Vector<SysProc>();
  dbg("Process::init()-> Vetor de processos inicializado\n");
  //TODO resolver problema em que uma chamada à memcpy() acontece se eu fizer atribuição direta
  SysProc("init", "init", (u32)0);
  //procs[0] = SysProc("init", "init", (u32)0);

  dbg("Process::init()-> Finalizado com sucesso\n");
 // procs.append(SysProc(const_cast<char*>("init"), const_cast<char*>("init"), (u32)0)); // Inicializa um SysProc
 // pages_in_use.append(Page((SysProc*)&(procs[0]), (u64)0)); // Cria uma página para o novo SysProc
  return true;
};

bool CreateProcess(const char* name, u16 privilege) {
  if(privilege != 0){ throw_panic(0, "CreateProcess::privilege>0: not yet implemented");}
  return true;
}

}
