#include "../Utils/Base.h"
#include "../Core/panic.h"

// ATA (Advanced Technology Attachment) protocol for PATA/SATA disks

#define us (unsigned short)

#define DATA_PORT us 0x1F0 // primary ATA interface
#define ERROR_PORT us 0x1F1
#define SECTOR_COUNT_PORT us 0x1F2 // how many sectors we will read?
#define SECTOR_NUMBER_PORT us 0x1F3
#define CYLINDER_LOW_PORT us 0x1F4
#define CYLINDER_HIGH_PORT us 0x1F5
#define DRIVE_HEAD_PORT us 0x1F6
#define ALTERNATE_STATUS_PORT us 0x3F6
#define STATUS_PORT us 0x1F7
#define COMMAND_PORT us 0x1F7

#define STATUS_WRITINGSECTOR us 0x30
#define STATUS_READINGSECTOR us 0x20

#define DISK_MODE 48 // LBA48 = 48 bits sector, disk more larger

// out -> write data
// in -> read data

// TODO colocar isso em Utils/Base.h
// TODO "outt" é um out temporario, mas é um outb isso, tem q passar pra usar no Base.h
void outt(unsigned short port, unsigned char data) {
  __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

void outw(unsigned short port, unsigned short data) {
  __asm__ volatile("outw %0, %1" : : "a"(data), "Nd"(port));
}

unsigned short inw(unsigned short port) {
  unsigned short result;
  __asm__ volatile("inw %%edx, %%ax" : "=a" (result) : "d" (port));
  return result;
}

inline void wait_for_disk_controller_w() {
  while((inb(STATUS_PORT) & 0x80) != 0) {}
}

inline void wait_for_disk_controller_r() {
  dbg("wait_for_disk_controller_r()-> Esperando disco...\n");
  while((inb(STATUS_PORT) & 0x80) != 0) {} // BSY
  unsigned char status;
  do {
    status = inw(STATUS_PORT);
  } while (!(status & 0x80));

  if(!(status & 0x40)) {
    throw_panic(0, "Disk error: not ready");
  }
}

void write_to_sector(short* bytes, unsigned int sector) {
  wait_for_disk_controller_w();
  

  outt(DRIVE_HEAD_PORT, (sector >> 24) | 0xE0 ); // byte alto do setor para registrador de disco
  outt(SECTOR_COUNT_PORT, (unsigned short)1); // 1 setor a ser lido
  outt(SECTOR_NUMBER_PORT, sector); // byte baixo do setor para registrador de disco
  outt(CYLINDER_LOW_PORT, (sector >> 8) & 0xFF);
  outt(CYLINDER_HIGH_PORT, (sector >> 16) & 0xFF );

  outt(STATUS_PORT, STATUS_WRITINGSECTOR);
  
  for(int i = 0; i < 256; i+=2) {
    // ele envia 2 bytes por vez para a porta de dados
    outw(DATA_PORT, bytes[i]);
  }
}

// 2 bytes por vez para serem lidos de um setor
// buffer precisa ter 512 bytes

// cada vez que 2 bytes de um setor são lidos, o registrador
// que guarda os bytes do setor é atualizado pros próximos
// 2 bytes.

#define LBA48_LOW_PORT 0x1F3
#define LBA48_MID_PORT 0x1F4
#define LBA48_HIGH_PORT 0x1F5
#define LBA48_SECTOR_COUNT_PORT 0x1F2
#define LBA48_DRIVE_HEAD_PORT 0x1F6
#define LBA48_READING 0x24

void read_from_sector(char* buffer, unsigned long long sector) {
  wait_for_disk_controller_r();
  char tmp_buf[512];
  for(int i = 0; i<512; i++)
    tmp_buf[i] = 0;
  itos((sector>>8)&0xFF, tmp_buf);
  dbg("CYLINDER_LOW_PORT: ");
  dbg(tmp_buf);
  dbg("\n");
  
  // TODO FIXME utilizar HAL para facilitar

  if(DISK_MODE == 48) {
    unsigned short count = 1; 
    outb(COMMAND_PORT, 0xE7);
    // Bytes mais significativos:
    outb(LBA48_SECTOR_COUNT_PORT, 0);
    outb(LBA48_LOW_PORT, (sector >> 24) & 0xFF);
    outb(LBA48_MID_PORT, (sector >> 32) & 0xFF);
    outb(LBA48_HIGH_PORT, (sector >> 40) & 0xFF);
    outb(LBA48_DRIVE_HEAD_PORT, 0x40 | ((sector >> 24) & 0x0F));
    
    // Bytes menos significativos:
    outb(LBA48_SECTOR_COUNT_PORT, count & 0xFF);
    outb(LBA48_LOW_PORT, sector & 0xFF);
    outb(LBA48_MID_PORT, (sector >> 8) & 0xFF);
    outb(LBA48_HIGH_PORT, (sector >> 16) & 0xFF);
    outb(LBA48_DRIVE_HEAD_PORT, 0xE0 | ((sector >> 24) & 0x0F));

    outb(COMMAND_PORT, LBA48_READING);
    
    dbg("read_from_sector()-> Iniciando leitura de disco\n");
    for(int i = 0; i<256; i+=2){
      short data_buf = inw(DATA_PORT);
      if(data_buf == 0) {
        dbg("..");
      } else {
        dbg("!!");
      }
      //buffer[i] = (char)(data_buf&0xFF); TODO checar se o buffer ta lendo corretamente dps, agora é focar no erro de leitura do driver
      //buffer[i+1] = (char)(data_buf >> 8);
      unsigned short status = inb(STATUS_PORT);
      if(status & 0x01) {
        throw_panic(0, "Disk error");
      }
    }
  } else {
    // NOT IMPLEMENTED YET
  }
  wait_for_disk_controller_r();
  dbg("\n");
}
