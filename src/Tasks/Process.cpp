//#include "Process.h"

#include "../Drivers/Disk.h"
#include "../Filesystem/Filesystem.h"
#include "../Drivers/VIDEO/preload.h"
#include "../Utils/Base.h"
#include "../Memory/Base_Mem.h"
#include "../panic.h"
#include "Process.h"
namespace Process{
  unsigned long long TOTAL_RAM = 0;

  #define vPage i16

  u64 amount_of_procs = 0;
  Memory::Vector<Process::SysProc> procs;

  SysProc::SysProc(const char *p_name, Binary *pbin, u32 pid) {

      if(pbin->magic_number != BINARY_MAGIC_NUMBER)
        return;
      
      //Utils::kmemcpy(name, p_name, 16);

      // TODO arredondar pra cima pra alinhar dados text_section_size e data_section_size
      //TextSection = (void)Memory::kmmap(pbin->text_section_size, 5);
      //DataSection = (void*)kmmap(pbin->data_section_size, 5);

      // TODO pegar as paginas alocadas de kmmap e jogar na tabela de páginas 
      // TODO configurar a tabela de paginas de acordo com o layout da memoria virtual dos processos
      
      /*
       * É aqui que a diversão começa
       * nós precisamos alocar a região de código, de dados e da stack na Heap
       * e então pegar o endereço resultante e colocar em cada página
      */

      dbg("Process::constructor-> Inicializando páginas para processo novo\n");
      throw_panic(0, "teste");
      this->pid = pid;
    };
    bool init() {
      // -----
      // Inicializando primeiro processo
      // -----
      procs = Memory::Vector<Process::SysProc>();
      dbg("Process::init()-> Vetor de processos inicializado\n");
      //TODO resolver problema em que uma chamada à memcpy() acontece se eu fizer atribuição direta
      //procs[0] = SysProc("init", "init", (u32)0);

      dbg("Process::init()-> Finalizado com sucesso\n");
      // procs.append(SysProc(const_cast<char*>("init"), const_cast<char*>("init"), (u32)0)); // Inicializa um SysProc
      // pages_in_use.append(Page((SysProc*)&(procs[0]), (u64)0)); // Cria uma página para o novo SysProc
      return true;
}; 


};

Memory::Vector<Process::SysProc> procs;
Process::SysProc* proc_current = 0;

class Page {
  public:
    Process::SysProc* proc;
      u64 page_number;
      u64 base_addr;

      Page(Process::SysProc* p, u64 b_addr) {
        this->proc = p;
        this->page_number = b_addr / PAGE_SIZE;
        this->base_addr = b_addr;
      };
};



u32 getNewPid() {
  return 1;
}

bool CreateProcess(const char* name, u16 privilege, const char* fs_binary_location) {
  if(privilege != 0){ throw_panic(0, "CreateProcess::privilege>0: not yet implemented");}

  Binary *bin = FS::LoadBinary(fs_binary_location);
  if(bin->magic_number != BINARY_MAGIC_NUMBER)
    return false;

  Process::SysProc new_process = Process::SysProc(name, bin, getNewPid());

  if ( (new_process.TextSection == 0) || (new_process.DataSection == 0) )
    return false;

  kfree(bin);

  return true;
}

