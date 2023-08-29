#include "Tasks/Process.h" // namespace Process
#include "Utils/Base.h" // namespace Utils
#include "DefaultConfig.h"
#include "Drivers/VIDEO/Video.h"
#include "HAL/HAL.h"

#define hlt __asm__ volatile("hlt")
#define newline Text::NewLine();
#define write Text::Write

char buffer[1024];

void panic(unsigned int id, char* msg, u64 rip) {
  CLI;
//#ifdef GRAPHICAL_MODE
#ifdef fodase
outb(0x3C0, 0x30); outb(0x3C0, 0x21); // Desabilita Proteção de Atributos
outw(0x3C4, 0x0100); // Synchronous Reset for the Sequencer
outw(0x3C4, 0x0302); // Character Map Select
outw(0x3C4, 0x0407); // Memory Mode
outw(0x3C4, 0x0003); // Synchronous Reset for the Sequencer
outw(0x3D4, 0x0D06); // Miscellanous Register
outw(0x3D4, 0x0007); // Overflow Register
outw(0x3D4, 0x3E04); // Underline Location
outw(0x3D4, 0x4105); // Mode Control Register
outw(0x3D4, 0xEA10); // Start Address High
outw(0x3D4, 0x8F12); // Cell Height
outw(0x3D4, 0x0614); // Maximum Scan Line
outw(0x3D4, 0xE715); // Start Address Low
outw(0x3D4, 0x0616); // Location of the Cursor Start
outw(0x3D4, 0xE317); // Text-Mode Blinking
#endif
  Text::NewLine();
  dbg("Kernel panic was released\nError message is %s", msg);
  dbg("\n");
  write(("Kernel panic!"));
  system.dump_stack();
  newline;
  write(msg);
  newline;
  newline;
  itos(Process::amount_of_procs, buffer);
  write(buffer);
  write(" tasks");
  newline;
  itos(number_of_chunks, buffer);
  write(buffer); write(" kernel chunks"); newline;
  itoh(rip, buffer); write(("0x"), 0x4); write(buffer, 0x4); write((" - RIP"), 0x4); newline;
  dbg("Tasks dump:\n");
  for(unsigned int i = 0; i<Process::amount_of_procs; i++) {
    //dbg(Process::procs[i].name);
    dbg("\n");
    //write(Process::procs[i].name);
  }
  /*
  for(int i = 0; i<1023; i++) {
    if(Process::procs[i] == 0)
      break;
    write!(Process::procs[i]->name);
    switch (Process::procs[i]->p_state) {
      case 0:
        write!(" Running with PID "); break;
      case 1:
        write!(" Paused with PID "); break;
      default:
        write(" had no state with PID "); break;
    }
    write!(Utils::itos(Process::procs[i]->pid));
  }
  */
  newline;
  if(mem_usage < 1024) {
    itos(mem_usage, buffer);
    write(buffer);
    write("B RAM usage");
  } else if ((mem_usage >= 1024) && (mem_usage < 1000000)) {
    itos(mem_usage / 1024, buffer);
    write(buffer);
    write("KB RAM usage");
  } else {
    itos(mem_usage / 1000000, buffer);
    write(buffer);
    write("MB RAM usage");
  }
  newline;
  write("System halted", 15);
  hlt;
}
