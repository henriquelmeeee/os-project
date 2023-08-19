#pragma once

#ifndef _HAL
#define _HAL

#include "../Utils/Base.h"
#include "../Memory/Base_Mem.h"
#include "../Drivers/VIDEO/preload.h"

#define PORT (char)0x3f8 /* COM1 register, for control */

namespace HAL {
  class System {
    public:
      unsigned long long TOTAL_RAM;

      System() {
        Text::NewLine();
        Text::Writeln("HAL: Looking for system information...", 0xe);
        this->TOTAL_RAM = 2 GB;
        unsigned long long TOTAL_RAM_BUFFER = this->TOTAL_RAM;
        char buffer[32];
        Text::Write("HAL: ", 0xe);
        
        // Hard-coded

        if(TOTAL_RAM_BUFFER < 1024) {
          itos(TOTAL_RAM_BUFFER, buffer);
          Text::Write(buffer);
          Text::Write("B");
        } else if ((TOTAL_RAM_BUFFER >= 1024) && (TOTAL_RAM_BUFFER < 1000000)) {
          itos(TOTAL_RAM_BUFFER / 1024, buffer);
          Text::Write(buffer);
          Text::Write("KB");
        } else {
          itos(TOTAL_RAM_BUFFER / 1000000, buffer);
          Text::Write(buffer);
          Text::Write("MB");
        }
        Text::Writeln(" of RAM found", 0xe);
      }

      bool init_serial_for_dbg() {
        outb(PORT + 1, (char)0x00);    // Disable all interrupts
        outb(PORT + 3, (char)0x80);    // Enable DLAB (set baud rate divisor)
        outb(PORT + 0, (char)0x03);    // Set divisor to 3 (lo byte) 38400 baud
        outb(PORT + 1, (char)0x00);    //                  (hi byte)
        outb(PORT + 3, (char)0x03);    // 8 bits, no parity, one stop bit
        outb(PORT + 2, (char)0xC7);    // Enable FIFO, clear them, with 14-byte threshold
        outb(PORT + 4, (char)0x0B);    // IRQs enabled, RTS/DSR set

        return true;
      }
  };
};

extern HAL::System system;

#endif
