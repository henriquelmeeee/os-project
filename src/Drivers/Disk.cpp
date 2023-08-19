#include "../Utils/Base.h"
#include "../panic.h"

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

#define DISK_MODE "LBA28" // LBA48 = 48 bits sector, disk more larger

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
  while((inw(STATUS_PORT) & 0x80) != 0) {}
}

inline void wait_for_disk_controller_r() {
  dbg("wait_for_disk_controller_r()-> Esperando disco...\n");
  while((inw(STATUS_PORT) & 0x80) != 0) {} // BSY
  unsigned char status;
  //while(!(status & 0x08)) status = inb(STATUS_PORT);
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

void read_from_sector(char* buffer, unsigned int sector) {
  wait_for_disk_controller_r();

  char tmp_buf[512];
  itos((sector>>8)&0xFF, tmp_buf);
  dbg("CYLINDER_LOW_PORT: ");
  dbg(tmp_buf);
  dbg("\n");

  outt(DRIVE_HEAD_PORT, (sector >> 24) | 0xE0);
  outt(SECTOR_COUNT_PORT, (unsigned char)1);
  outt(SECTOR_NUMBER_PORT, sector & 0xFF);
  outt(CYLINDER_LOW_PORT, (sector >> 8) & 0xFF);
  outt(CYLINDER_HIGH_PORT, (sector >> 16) & 0xFF);
  outt(COMMAND_PORT, STATUS_READINGSECTOR);
  dbg("read_from_sector()-> Iniciando leitura de disco\n");
  for(int i = 0; i<256; i+=2){
    short data_buf = inw(DATA_PORT);
    if(data_buf == 0) {
      dbg("..");
    } else {
    dbg("!!");
    }
    buffer[i] = (char)(data_buf&0xFF);
    buffer[i+1] = (char)(data_buf >> 8);
    unsigned short status = inw(STATUS_PORT);
    if(status & 0x01) {
      throw_panic(0, "Disk error");
    }
  }
  dbg("\n");
}
