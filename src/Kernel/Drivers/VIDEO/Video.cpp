inline void outb(unsigned short port, unsigned char value) {
  asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

inline unsigned char inb(unsigned short port) {
  unsigned char ret;
  asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

const int SCALE_FACTOR = 2;
unsigned char cursor[6 * SCALE_FACTOR][2] = {
    {0 * SCALE_FACTOR, 3 * SCALE_FACTOR},  // Linha 0, Coluna 3
    {1 * SCALE_FACTOR, 2 * SCALE_FACTOR},  // Linha 1, Coluna 2
    {1 * SCALE_FACTOR, 4 * SCALE_FACTOR},  // Linha 1, Coluna 4
    {2 * SCALE_FACTOR, 1 * SCALE_FACTOR},  // Linha 2, Coluna 1
    {2 * SCALE_FACTOR, 5 * SCALE_FACTOR},  // Linha 2, Coluna 5
    {3 * SCALE_FACTOR, 0 * SCALE_FACTOR},  // Linha 3, Coluna 0
    {3 * SCALE_FACTOR, 6 * SCALE_FACTOR},  // Linha 3, Coluna 6
    {4 * SCALE_FACTOR, 3 * SCALE_FACTOR}   // Linha 4, Coluna 3
};


#include "../../DefaultConfig.h"

unsigned int cursor_location[] = {30,30};

#ifndef GRAPHICAL_MODE
volatile unsigned char* video_addr = (volatile unsigned char*) 0xB8000;
#define DEFAULT_VIDEO_HEIGHT 25
#define DEFAULT_VIDEO_WIDTH 80
#else
volatile unsigned char* video_addr = (volatile unsigned char* ) 0xA0000;
#define DEFAULT_VIDEO_HEIGHT 768
#define DEFAULT_VIDEO_WIDTH 1024
#endif


volatile unsigned int video_height = (volatile unsigned int) 0;
volatile unsigned int video_width = (volatile unsigned int) 0;

unsigned char GenericWindow[30][95][3];


namespace Text{
char* last_char = (char*)0xB8000;

void NewLine() {
  //video_height = (volatile unsigned int)1 + video_height;
  video_height+=1;
  video_width = (volatile unsigned int)0;
}

void Write(const char* string, char color = 0x07) {
  for(int i = 0; string[i]!=0; i++) {
     video_addr[(video_height * DEFAULT_VIDEO_WIDTH + video_width) * 2] = string[i];
     video_addr[(video_height * DEFAULT_VIDEO_WIDTH + video_width) * 2 + 1] = color;
     video_width += 1;
     if(video_width == 80) {
        video_width = 0;
        video_height += 1;
        if(video_height == DEFAULT_VIDEO_HEIGHT) {
          video_height = 0;
          video_width = 0;
        }
     }
  }
}
void Writeln(const char* string, char color = 0x07) {
  Write(string, color);
  NewLine();
}



void text_clear() {
  for(int i = 0; i<3200; i++) {
    *(video_addr+i) = 0;
  }
  video_height = 0;
  video_width = 0;
}
}



namespace Graphics {
void clear_video(char color) {
  for(int i = 0; i!=64000; i++) {
    volatile unsigned char* buf = (volatile unsigned char *)(video_addr+i);
    *buf = color;
  }
}


void put_pixel(int x, int y, char r, char g, char b) {
  volatile unsigned char* buf = (volatile unsigned char*) (video_addr+y*DEFAULT_VIDEO_WIDTH+x);
  *buf = r;
  *(buf+1) = g;
  *(buf+2) = b;
}

void draw_window() {
  volatile unsigned char* buf = (volatile unsigned char*)video_addr;
  for(int coluna = 0; coluna<30; coluna++) {
    for(int linha = 0; linha<70; linha++) {
      put_pixel(linha, coluna, GenericWindow[linha][coluna][0], GenericWindow[linha][coluna][1], GenericWindow[linha][coluna][1]);
    }
  }
}

void put_char(int x, int y, char c, int color) {
  //unsigned char* bitmap = font[0];
}

}
