#ifndef VIDEO
#define VIDEO
namespace Text{
void Write(const char* string, char color = 0x07);
void NewLine();
void text_clear();
}
namespace Graphics{

extern unsigned int cursor_location[2];
void clear_video(char color);
void put_pixel(int x, int y, char color);
void put_char(int x, int y, char c, char color);
void draw_window();
}
#endif
