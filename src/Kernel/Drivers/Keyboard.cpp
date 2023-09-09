#include "../Utils/Base.h"
#include "../Core/panic.h"
#include "../Memory/Base_Mem.h"
#include "Keyboard.h"
struct KeyboardInterrupt {
  u64 rip;
  u64 cs;
  u64 rflags;
  u64 rsp;
  u64 ss;
};
namespace Drivers{
namespace Keyboard {

  //unsigned int keys[255];

  /*unsigned char scancodes[] = {
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U',
    'I', 'O', 'P', '[', ']', '\0', '\0', 'A',
    'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';',
    '\'', '`', '\0', '\\', 'Z', 'X', 'C', 'V', 'B',
    'N', 'M', ',', '.', '/', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0'
  };*/

  /*

  bool caps = false;
  bool shift = false;

  void handle_caps() {
    // ...
    STI; // retorna as interrupções
  }

  void clear_key_list() {
    for(short x = 0; x < 255; x++) {
      keys[x] = 0;
    }
  }
  int last_key = 0;
  void continue_handle(unsigned int scancode) {
    dbg("continue_handle()-> Rotina alcançada\n");
    if(scancodes[scancode] == 0)
      return;
    if(keys[254] != 0) 
      clear_key_list();
    unsigned short key = scancodes[scancode];
    if(!caps)
      key += 32;

    if(key == '\\' && shift) {
      keys[last_key] = '|';
      return;
    }
    keys[last_key] = scancodes[scancode];
    ++last_key;
    if(last_key == 255)
      clear_key_list();
  }

  #define READ_KEYBOARD_STATUS 0x64 // há mesmo algum scancode para ler?
  */
  /*
   * -> Toda interrupção de teclado precisa de um EOI, enquanto não houver,
   *    interrupções subjacentes ficarão numa pool da PIC
   *    esse EOI é marcado pelo Kernel quando todos os programas recebem a nova tecla
   *    corretamente.
  */    

  void/* NO_CALLER_SAVED_REGISTERS*/ keyboard_interrupt_key(KeyboardInterrupt *s) {
    CLI;
    __asm__ volatile("hlt");

#if 0
    dbg("keyboard_interrupt_key()\n");
    unsigned char status;
    unsigned char scancode;

    status = inb(READ_KEYBOARD_STATUS);
    if(status & 0x1) {
      dbg("keyboard_interrupt_key()-> Teclado disponível\n");
      scancode = inb(0x60);
    
    //scancode -= 14; // ignore control keys
    switch (scancode) { // handle special keys
        case 58:
          handle_caps();
          break;
        case 0x2A:
          shift = true;
          break;
        case 0x36:
          shift = true;
          break;
        case 0xAA:
          shift = false;
          break;
        case 0xB6:
          shift = false;
          break;
        default:
          //continue_handle(scancode);
          break;
      }
    }
    status = inb(READ_KEYBOARD_STATUS);
    if(status & 0x1) {
      dbg("mais uma interrupcao");
    }
    dbg("enviando eoi\n");
    //outb(0x60, 0xFA);
    outb(0x20, 0x20);
    outb(0xA0, 0x20);
    IRET;
    throw_panic(0, "Unable to return from interruption handler");
#endif
  }

}
}
