#include "../Utils/Base.h"

static inline void insw(unsigned short port, unsigned short *dest, unsigned int count) {
    asm volatile ("cld; rep insw" :
                  "=D" (dest), "=c" (count) :
                  "d" (port), "0" (dest), "1" (count) :
                  "memory", "cc");
}

typedef struct {
    u16 base;
    u16 ctrl;
    u16 bmide;
    u8 nien :1;
} ide_channel_registers_t;

void _read_from_sector(ide_channel_registers_t* channel, u8 slave, u32 lba, u8 sector_count, u16* buffer) {
    dbg("starting read\n");

    // Polling para verificar se o drive está pronto
    while(inb(channel->base + 7) & 0x80);

    // Desativa todas as interrupções para a controladora
    outb(channel->ctrl, 0);

    // Configuração de parâmetros para o comando de leitura
    outb(channel->base + 2, sector_count);
    outb(channel->base + 3, (u8) lba);
    outb(channel->base + 4, (u8)(lba >> 8));
    outb(channel->base + 5, (u8)(lba >> 16));
    outb(channel->base + 6, 0xE0 | (slave << 4) | ((lba >> 24) & 0x0F));

    // Envio do comando READ SECTORS (0x20)
    outb(channel->base + 7, 0x20);

    // Polling para verificar se o dado está pronto para ser lido
    while (!(inb(channel->base + 7) & 0x08));

    // Leitura dos dados
    insw(channel->base, buffer, sector_count * 256);

    dbg("disco pronto\n");
}

void read_from_sector(const char* buffer, u64 sector) {
    ide_channel_registers_t channel;
    dbg("called read_from_sector()\n");
    channel.base = 0x1F0;
    channel.ctrl = 0x3F6;

    _read_from_sector(&channel, 0, sector, 1, (u16*)buffer);
}



#if 0
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
    u8 status = inb(channel->base + 7);
    if(status & 0x01) {
      dbg("Disk error status: %d", inb(channel->base + 1));
      throw_panic(0, "Disk error");
    }
    while(inb(channel->base + 7) & 0x80);
    // Configuração inicial
    outb(channel->ctrl, 0);  // Desativa todas as interrupções para a controladora
    outb(channel->base + 2, sector_count);  // Contagem de setores

    // Envio dos bits de LBA
    outb(channel->base + 3, (u8) lba);
    outb(channel->base + 4, (u8)(lba >> 8));
    outb(channel->base + 5, (u8)(lba >> 16));
    outb(channel->base + 6, 0x40 | (slave << 4) | ((lba >> 24) & 0x0F)); // Especifica o drive e os bits altos de LBA

    outb(channel->base + 3, (u8)(lba >> 24));
    outb(channel->base + 4, (u8)(lba >> 32));
    outb(channel->base + 5, (u8)(lba >> 40));

    // Comando READ SECTORS EXT
    outb(channel->base + 7, 0x24);

    // Polling até que o bit DRQ seja definido
    while (!(inb(channel->base + 7) & 0x8));

    // Leitura dos dados
    insw(channel->base, buffer, sector_count * 256);
    status = inb(channel->base + 7);
    if(status & 0x01) {
      dbg("Disk error status: %d", inb(channel->base + 1));
      throw_panic(0, "Disk error");
    }

    // Espera o disco estar pronto
    dbg("disco pronto\n");
    
    // Ler dados
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
#endif
