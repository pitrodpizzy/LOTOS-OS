#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>  // <-- DODAJ TĘ LINIĘ#define KERNEL_H

extern unsigned int* framebuffer;
extern unsigned int framebuffer_pitch;
extern unsigned int framebuffer_width;
extern unsigned int framebuffer_height;
void load_bmp(int x, int y);
void put_pixel(int x, int y, unsigned int color);
void draw_string(int x, int y, const char* text, unsigned int color);
void clear_screen(unsigned int color);
void console_put_char(char c);
void console_print(const char* text);

unsigned char keyboard_has_key(void);
unsigned char get_scancode(void);


void play_music(void);
void beep(unsigned int frequency);

extern int console_x;
extern int console_y;
void run_3d_demo(int32_t pos_x, int32_t pos_y);
void shutdown(void);
void run_3d_demo2(int32_t pos_x, int32_t pos_y);
#endif
