#include "../Utils/Base.h"
// temp disk driver
static inline void insw(unsigned short port, unsigned short *dest, unsigned int count) {
    asm volatile ("cld; rep insw" :
                  "=D" (dest), "=c" (count) :
                  "d" (port), "0" (dest), "1" (count) :
                  "memory", "cc");
}

// Estrutura para a comunicação com o drive via PIO
typedef struct {
    u16 base;
    u16 ctrl;
    u16 bmide;
    u8 nien :1;
} ide_channel_registers_t;

// Função que lê um setor do disco em um buffer
void _read_from_sector(ide_channel_registers_t* channel, u8 slave, u64 lba, u8 sector_count, u16* buffer) {
    // Enviar informações de configuração ao canal IDE
    dbg("starting read\n");

    outb(channel->base  + 6, 0x40 | (slave << 4) | ((lba >> 24) & 0x0F)); // bits 24-27 do LBA
    outb(channel->base  + 2, sector_count);
    outb(channel->base  + 3, (u8)  lba);       // LBA bits 0-7
    outb(channel->base  + 4, (u8) (lba >> 8)); // LBA bits 8-15
    outb(channel->base  + 5, (u8) (lba >> 16));// LBA bits 16-23
    outb(channel->base  + 3, (u8) (lba >> 24));// LBA bits 24-31
    outb(channel->base  + 4, (u8) (lba >> 32));// LBA bits 32-39
    outb(channel->base  + 5, (u8) (lba >> 40));// LBA bits 40-47
    outb(channel->base  + 7, 0x24); // Comando READ SECTORS EXT
    
    u8 status = inb(channel->base + 7);
    if(status & 0x01) {
      dbg("Disk error status: %d", inb(channel->base + 1));
      throw_panic(0, "Disk error");
    }

    dbg("starting read 2\n");
    // Espera o disco estar pronto
    while (!(inb(channel->base + 7) & 0x08));
    dbg("disco pronto\n");
    
    // Ler dados
    insw(channel->base, buffer, sector_count * 256);
}

void read_from_sector(const char* buffer, u64 sector) {
    ide_channel_registers_t channel;
    dbg("called read_from_sector()\n");
    channel.base = 0x1F0;  // Endereço base usual para o primeiro canal IDE
    channel.ctrl = 0x3F6;  // Endereço usual do controle ido primeiro canal IDE

    // Lê o primeiro setor em LBA48
    _read_from_sector(&channel, 0, sector, 1, (u16*)buffer);

    // O buffer agora contém os dados do primeiro setor
    // Siga com a lógica necessária

}

