#include "usb.h"
#include "commands.h"
void console_print(const char* text);

unsigned char keyboard_has_key(void);
unsigned char get_scancode(void);

void usb_init(void);
void usb_test(void);
unsigned int* framebuffer = 0;
unsigned int framebuffer_pitch = 0;
unsigned int framebuffer_width = 0;
unsigned int framebuffer_height = 0;
extern unsigned char _binary_logo_bmp_start[];
extern unsigned char _binary_logo_bmp_end[];
void put_pixel(int x, int y, unsigned int color);
void load_bmp(int x, int y)
{
    unsigned char* bmp = _binary_logo_bmp_start;

    if (bmp[0] != 'B' || bmp[1] != 'M')
        return;

    unsigned int pixel_offset =
        bmp[10] |
        (bmp[11] << 8) |
        (bmp[12] << 16) |
        (bmp[13] << 24);

    int width =
        bmp[18] |
        (bmp[19] << 8) |
        (bmp[20] << 16) |
        (bmp[21] << 24);

    int height =
        bmp[22] |
        (bmp[23] << 8) |
        (bmp[24] << 16) |
        (bmp[25] << 24);

    unsigned short bpp =
        bmp[28] |
        (bmp[29] << 8);

    if (bpp != 24)
        return;

    unsigned int row_size =
        (width * 3 + 3) & ~3;

    unsigned char* pixels = bmp + pixel_offset;

    if (framebuffer_width == 0 || framebuffer_height == 0)
        return;

    for (unsigned int screen_y = 0;
         screen_y < framebuffer_height;
         screen_y++)
    {
        int src_y =
            (screen_y * height) / framebuffer_height;

        for (unsigned int screen_x = 0;
             screen_x < framebuffer_width;
             screen_x++)
        {
            int src_x =
                (screen_x * width) / framebuffer_width;

            unsigned char* pixel =
                pixels +
                (height - 1 - src_y) * row_size +
                src_x * 3;

            unsigned char blue = pixel[0];
            unsigned char green = pixel[1];
            unsigned char red = pixel[2];

            unsigned int color =
                ((unsigned int)red << 16) |
                ((unsigned int)green << 8) |
                blue;

            put_pixel(x + screen_x, y + screen_y, color);
        }
    }
}
#define COMMAND_MAX 128

char command[COMMAND_MAX];
int command_pos = 0;
void console_put_char(char c);
void draw_string(int x, int y, const char* text, unsigned int color);
void clear_screen(unsigned int color);
void put_pixel(int x, int y, unsigned int color);


void console_input(char c)
{
    if (command_pos >= COMMAND_MAX - 1)
        return;

    command[command_pos] = c;
    command_pos++;

    command[command_pos] = '\0';
if (c >= 'a' && c <= 'z')
    c = c - 'a' + 'A';
    console_put_char(c);
}









unsigned char font[128][8];
void init_font()
{
/* a */
font['a'][0] = 0x00;
font['a'][1] = 0x00;
font['a'][2] = 0x3C;
font['a'][3] = 0x02;
font['a'][4] = 0x3E;
font['a'][5] = 0x42;
font['a'][6] = 0x3E;

/* b */
font['b'][0] = 0x40;
font['b'][1] = 0x40;
font['b'][2] = 0x5C;
font['b'][3] = 0x62;
font['b'][4] = 0x42;
font['b'][5] = 0x62;
font['b'][6] = 0x5C;

/* c */
font['c'][0] = 0x00;
font['c'][1] = 0x00;
font['c'][2] = 0x3C;
font['c'][3] = 0x42;
font['c'][4] = 0x40;
font['c'][5] = 0x42;
font['c'][6] = 0x3C;

/* d */
font['d'][0] = 0x02;
font['d'][1] = 0x02;
font['d'][2] = 0x3A;
font['d'][3] = 0x46;
font['d'][4] = 0x42;
font['d'][5] = 0x46;
font['d'][6] = 0x3A;

/* e */
font['e'][0] = 0x00;
font['e'][1] = 0x00;
font['e'][2] = 0x3C;
font['e'][3] = 0x42;
font['e'][4] = 0x7E;
font['e'][5] = 0x40;
font['e'][6] = 0x3C;

/* f */
font['f'][0] = 0x0C;
font['f'][1] = 0x10;
font['f'][2] = 0x3C;
font['f'][3] = 0x10;
font['f'][4] = 0x10;
font['f'][5] = 0x10;
font['f'][6] = 0x10;

/* g */
font['g'][0] = 0x00;
font['g'][1] = 0x00;
font['g'][2] = 0x3A;
font['g'][3] = 0x46;
font['g'][4] = 0x46;
font['g'][5] = 0x3A;
font['g'][6] = 0x06;
font['g'][7] = 0x3C;

/* h */
font['h'][0] = 0x40;
font['h'][1] = 0x40;
font['h'][2] = 0x5C;
font['h'][3] = 0x62;
font['h'][4] = 0x42;
font['h'][5] = 0x42;
font['h'][6] = 0x42;

/* i */
font['i'][0] = 0x18;
font['i'][1] = 0x00;
font['i'][2] = 0x38;
font['i'][3] = 0x18;
font['i'][4] = 0x18;
font['i'][5] = 0x18;
font['i'][6] = 0x3C;

/* j */
font['j'][0] = 0x06;
font['j'][1] = 0x00;
font['j'][2] = 0x1E;
font['j'][3] = 0x06;
font['j'][4] = 0x06;
font['j'][5] = 0x46;
font['j'][6] = 0x3C;

/* k */
font['k'][0] = 0x40;
font['k'][1] = 0x40;
font['k'][2] = 0x44;
font['k'][3] = 0x48;
font['k'][4] = 0x70;
font['k'][5] = 0x48;
font['k'][6] = 0x44;

/* l */
font['l'][0] = 0x38;
font['l'][1] = 0x18;
font['l'][2] = 0x18;
font['l'][3] = 0x18;
font['l'][4] = 0x18;
font['l'][5] = 0x18;
font['l'][6] = 0x3C;

/* m */
font['m'][0] = 0x00;
font['m'][1] = 0x00;
font['m'][2] = 0x6C;
font['m'][3] = 0x52;
font['m'][4] = 0x52;
font['m'][5] = 0x52;
font['m'][6] = 0x52;

/* n */
font['n'][0] = 0x00;
font['n'][1] = 0x00;
font['n'][2] = 0x5C;
font['n'][3] = 0x62;
font['n'][4] = 0x42;
font['n'][5] = 0x42;
font['n'][6] = 0x42;

/* o */
font['o'][0] = 0x00;
font['o'][1] = 0x00;
font['o'][2] = 0x3C;
font['o'][3] = 0x42;
font['o'][4] = 0x42;
font['o'][5] = 0x42;
font['o'][6] = 0x3C;

/* p */
font['p'][0] = 0x00;
font['p'][1] = 0x00;
font['p'][2] = 0x5C;
font['p'][3] = 0x62;
font['p'][4] = 0x62;
font['p'][5] = 0x5C;
font['p'][6] = 0x40;
font['p'][7] = 0x40;

/* q */
font['q'][0] = 0x00;
font['q'][1] = 0x00;
font['q'][2] = 0x3A;
font['q'][3] = 0x46;
font['q'][4] = 0x46;
font['q'][5] = 0x3A;
font['q'][6] = 0x02;
font['q'][7] = 0x02;

/* r */
font['r'][0] = 0x00;
font['r'][1] = 0x00;
font['r'][2] = 0x5C;
font['r'][3] = 0x62;
font['r'][4] = 0x40;
font['r'][5] = 0x40;
font['r'][6] = 0x40;

/* s */
font['s'][0] = 0x00;
font['s'][1] = 0x00;
font['s'][2] = 0x3E;
font['s'][3] = 0x40;
font['s'][4] = 0x3C;
font['s'][5] = 0x02;
font['s'][6] = 0x7C;

/* t */
font['t'][0] = 0x10;
font['t'][1] = 0x10;
font['t'][2] = 0x7C;
font['t'][3] = 0x10;
font['t'][4] = 0x10;
font['t'][5] = 0x12;
font['t'][6] = 0x0C;

/* u */
font['u'][0] = 0x00;
font['u'][1] = 0x00;
font['u'][2] = 0x42;
font['u'][3] = 0x42;
font['u'][4] = 0x42;
font['u'][5] = 0x46;
font['u'][6] = 0x3A;

/* v */
font['v'][0] = 0x00;
font['v'][1] = 0x00;
font['v'][2] = 0x42;
font['v'][3] = 0x42;
font['v'][4] = 0x42;
font['v'][5] = 0x24;
font['v'][6] = 0x18;

/* w */
font['w'][0] = 0x00;
font['w'][1] = 0x00;
font['w'][2] = 0x42;
font['w'][3] = 0x42;
font['w'][4] = 0x5A;
font['w'][5] = 0x5A;
font['w'][6] = 0x24;

/* x */
font['x'][0] = 0x00;
font['x'][1] = 0x00;
font['x'][2] = 0x42;
font['x'][3] = 0x24;
font['x'][4] = 0x18;
font['x'][5] = 0x24;
font['x'][6] = 0x42;

/* y */
font['y'][0] = 0x00;
font['y'][1] = 0x00;
font['y'][2] = 0x42;
font['y'][3] = 0x42;
font['y'][4] = 0x46;
font['y'][5] = 0x3A;
font['y'][6] = 0x02;
font['y'][7] = 0x3C;

/* z */
font['z'][0] = 0x00;
font['z'][1] = 0x00;
font['z'][2] = 0x7E;
font['z'][3] = 0x04;
font['z'][4] = 0x18;
font['z'][5] = 0x20;
font['z'][6] = 0x7E;


    /* A */
    font['A'][0] = 0x18;
    font['A'][1] = 0x24;
    font['A'][2] = 0x42;
    font['A'][3] = 0x7E;
    font['A'][4] = 0x42;
    font['A'][5] = 0x42;
    font['A'][6] = 0x42;

    /* B */
    font['B'][0] = 0x7C;
    font['B'][1] = 0x42;
    font['B'][2] = 0x42;
    font['B'][3] = 0x7C;
    font['B'][4] = 0x42;
    font['B'][5] = 0x42;
    font['B'][6] = 0x7C;

    /* C */
    font['C'][0] = 0x3C;
    font['C'][1] = 0x42;
    font['C'][2] = 0x40;
    font['C'][3] = 0x40;
    font['C'][4] = 0x40;
    font['C'][5] = 0x42;
    font['C'][6] = 0x3C;

    /* D */
    font['D'][0] = 0x78;
    font['D'][1] = 0x44;
    font['D'][2] = 0x42;
    font['D'][3] = 0x42;
    font['D'][4] = 0x42;
    font['D'][5] = 0x44;
    font['D'][6] = 0x78;

    /* E */
    font['E'][0] = 0x7E;
    font['E'][1] = 0x40;
    font['E'][2] = 0x40;
    font['E'][3] = 0x7C;
    font['E'][4] = 0x40;
    font['E'][5] = 0x40;
    font['E'][6] = 0x7E;

    /* F */
    font['F'][0] = 0x7E;
    font['F'][1] = 0x40;
    font['F'][2] = 0x40;
    font['F'][3] = 0x7C;
    font['F'][4] = 0x40;
    font['F'][5] = 0x40;
    font['F'][6] = 0x40;

    /* G */
    font['G'][0] = 0x3C;
    font['G'][1] = 0x42;
    font['G'][2] = 0x40;
    font['G'][3] = 0x4E;
    font['G'][4] = 0x42;
    font['G'][5] = 0x42;
    font['G'][6] = 0x3C;

    /* H */
    font['H'][0] = 0x42;
    font['H'][1] = 0x42;
    font['H'][2] = 0x42;
    font['H'][3] = 0x7E;
    font['H'][4] = 0x42;
    font['H'][5] = 0x42;
    font['H'][6] = 0x42;

    /* I */
    font['I'][0] = 0x7E;
    font['I'][1] = 0x18;
    font['I'][2] = 0x18;
    font['I'][3] = 0x18;
    font['I'][4] = 0x18;
    font['I'][5] = 0x18;
    font['I'][6] = 0x7E;

    /* J */
    font['J'][0] = 0x1E;
    font['J'][1] = 0x04;
    font['J'][2] = 0x04;
    font['J'][3] = 0x04;
    font['J'][4] = 0x44;
    font['J'][5] = 0x44;
    font['J'][6] = 0x38;

    /* K */
    font['K'][0] = 0x42;
    font['K'][1] = 0x44;
    font['K'][2] = 0x48;
    font['K'][3] = 0x70;
    font['K'][4] = 0x48;
    font['K'][5] = 0x44;
    font['K'][6] = 0x42;

    /* L */
    font['L'][0] = 0x40;
    font['L'][1] = 0x40;
    font['L'][2] = 0x40;
    font['L'][3] = 0x40;
    font['L'][4] = 0x40;
    font['L'][5] = 0x40;
    font['L'][6] = 0x7E;

    /* M */
    font['M'][0] = 0x42;
    font['M'][1] = 0x66;
    font['M'][2] = 0x5A;
    font['M'][3] = 0x5A;
    font['M'][4] = 0x42;
    font['M'][5] = 0x42;
    font['M'][6] = 0x42;

    /* N */
    font['N'][0] = 0x42;
    font['N'][1] = 0x62;
    font['N'][2] = 0x52;
    font['N'][3] = 0x4A;
    font['N'][4] = 0x46;
    font['N'][5] = 0x42;
    font['N'][6] = 0x42;

    /* O */
    font['O'][0] = 0x3C;
    font['O'][1] = 0x42;
    font['O'][2] = 0x42;
    font['O'][3] = 0x42;
    font['O'][4] = 0x42;
    font['O'][5] = 0x42;
    font['O'][6] = 0x3C;

    /* P */
    font['P'][0] = 0x7C;
    font['P'][1] = 0x42;
    font['P'][2] = 0x42;
    font['P'][3] = 0x7C;
    font['P'][4] = 0x40;
    font['P'][5] = 0x40;
    font['P'][6] = 0x40;

    /* Q */
    font['Q'][0] = 0x3C;
    font['Q'][1] = 0x42;
    font['Q'][2] = 0x42;
    font['Q'][3] = 0x42;
    font['Q'][4] = 0x4A;
    font['Q'][5] = 0x44;
    font['Q'][6] = 0x3A;

    /* R */
    font['R'][0] = 0x7C;
    font['R'][1] = 0x42;
    font['R'][2] = 0x42;
    font['R'][3] = 0x7C;
    font['R'][4] = 0x48;
    font['R'][5] = 0x44;
    font['R'][6] = 0x42;

    /* S */
    font['S'][0] = 0x3C;
    font['S'][1] = 0x40;
    font['S'][2] = 0x40;
    font['S'][3] = 0x3C;
    font['S'][4] = 0x02;
    font['S'][5] = 0x02;
    font['S'][6] = 0x7C;

    /* T */
    font['T'][0] = 0x7E;
    font['T'][1] = 0x18;
    font['T'][2] = 0x18;
    font['T'][3] = 0x18;
    font['T'][4] = 0x18;
    font['T'][5] = 0x18;
    font['T'][6] = 0x18;

    /* U */
    font['U'][0] = 0x42;
    font['U'][1] = 0x42;
    font['U'][2] = 0x42;
    font['U'][3] = 0x42;
    font['U'][4] = 0x42;
    font['U'][5] = 0x42;
    font['U'][6] = 0x3C;

    /* V */
    font['V'][0] = 0x42;
    font['V'][1] = 0x42;
    font['V'][2] = 0x42;
    font['V'][3] = 0x42;
    font['V'][4] = 0x42;
    font['V'][5] = 0x24;
    font['V'][6] = 0x18;

    /* W */
    font['W'][0] = 0x42;
    font['W'][1] = 0x42;
    font['W'][2] = 0x42;
    font['W'][3] = 0x5A;
    font['W'][4] = 0x5A;
    font['W'][5] = 0x66;
    font['W'][6] = 0x42;

    /* X */
    font['X'][0] = 0x42;
    font['X'][1] = 0x42;
    font['X'][2] = 0x24;
    font['X'][3] = 0x18;
    font['X'][4] = 0x24;
    font['X'][5] = 0x42;
    font['X'][6] = 0x42;

    /* Y */
    font['Y'][0] = 0x42;
    font['Y'][1] = 0x42;
    font['Y'][2] = 0x24;
    font['Y'][3] = 0x18;
    font['Y'][4] = 0x18;
    font['Y'][5] = 0x18;
    font['Y'][6] = 0x18;

    /* Z */
    font['Z'][0] = 0x7E;
    font['Z'][1] = 0x02;
    font['Z'][2] = 0x04;
    font['Z'][3] = 0x18;
    font['Z'][4] = 0x20;
    font['Z'][5] = 0x40;
    font['Z'][6] = 0x7E;

    /* 0 */
    font['0'][0] = 0x3C;
    font['0'][1] = 0x46;
    font['0'][2] = 0x4A;
    font['0'][3] = 0x52;
    font['0'][4] = 0x62;
    font['0'][5] = 0x42;
    font['0'][6] = 0x3C;

    /* 1 */
    font['1'][0] = 0x18;
    font['1'][1] = 0x38;
    font['1'][2] = 0x18;
    font['1'][3] = 0x18;
    font['1'][4] = 0x18;
    font['1'][5] = 0x18;
    font['1'][6] = 0x7E;

    /* 2 */
    font['2'][0] = 0x3C;
    font['2'][1] = 0x42;
    font['2'][2] = 0x02;
    font['2'][3] = 0x0C;
    font['2'][4] = 0x30;
    font['2'][5] = 0x40;
    font['2'][6] = 0x7E;

    /* 3 */
    font['3'][0] = 0x3C;
    font['3'][1] = 0x42;
    font['3'][2] = 0x02;
    font['3'][3] = 0x1C;
    font['3'][4] = 0x02;
    font['3'][5] = 0x42;
    font['3'][6] = 0x3C;

    /* 4 */
    font['4'][0] = 0x0C;
    font['4'][1] = 0x14;
    font['4'][2] = 0x24;
    font['4'][3] = 0x44;
    font['4'][4] = 0x7E;
    font['4'][5] = 0x04;
    font['4'][6] = 0x04;

    /* 5 */
    font['5'][0] = 0x7E;
    font['5'][1] = 0x40;
    font['5'][2] = 0x40;
    font['5'][3] = 0x7C;
    font['5'][4] = 0x02;
    font['5'][5] = 0x42;
    font['5'][6] = 0x3C;

    /* 6 */
    font['6'][0] = 0x3C;
    font['6'][1] = 0x40;
    font['6'][2] = 0x40;
    font['6'][3] = 0x7C;
    font['6'][4] = 0x42;
    font['6'][5] = 0x42;
    font['6'][6] = 0x3C;

    /* 7 */
    font['7'][0] = 0x7E;
    font['7'][1] = 0x02;
    font['7'][2] = 0x04;
    font['7'][3] = 0x08;
    font['7'][4] = 0x10;
    font['7'][5] = 0x10;
    font['7'][6] = 0x10;

    /* 8 */
    font['8'][0] = 0x3C;
    font['8'][1] = 0x42;
    font['8'][2] = 0x42;
    font['8'][3] = 0x3C;
    font['8'][4] = 0x42;
    font['8'][5] = 0x42;
    font['8'][6] = 0x3C;

    /* 9 */
    font['9'][0] = 0x3C;
    font['9'][1] = 0x42;
    font['9'][2] = 0x42;
    font['9'][3] = 0x3E;
    font['9'][4] = 0x02;
    font['9'][5] = 0x02;
    font['9'][6] = 0x3C;

    /* spacja */
    font[' '][0] = 0;
    font[' '][1] = 0;
    font[' '][2] = 0;
    font[' '][3] = 0;
    font[' '][4] = 0;
    font[' '][5] = 0;
    font[' '][6] = 0;
    font[' '][7] = 0;

    /* podstawowe znaki */
    font['-'][3] = 0x7E;

    font['.'][6] = 0x18;

    font['!'][0] = 0x18;
    font['!'][1] = 0x18;
    font['!'][2] = 0x18;
    font['!'][3] = 0x18;
    font['!'][4] = 0x18;
    font['!'][6] = 0x18;

    font[':'][2] = 0x18;
    font[':'][5] = 0x18;
}



void draw_char(int x, int y, char c, unsigned int color)
{
    for(int row = 0; row < 8; row++)
    {
        for(int col = 0; col < 8; col++)
        {
            if(font[(int)c][row] & (1 << (7-col)))
            {
                put_pixel(x + col, y + row, color);
            }
        }
    }
}
void draw_string(int x, int y, const char* text, unsigned int color)
{
    int i = 0;

    while(text[i])
    {
        draw_char(
            x + i * 8,
            y,
            text[i],
            color
        );

        i++;
    }
}








void draw_full_square(int x, int y, int size, unsigned int color)
{
    for (int yy = 0; yy < size; yy++)
    {
        for (int xx = 0; xx < size; xx++)
        {
            put_pixel(x + xx, y + yy, color);
        }
    }
}
void draw_line(int x0, int y0, int x1, int y1, unsigned int color)
{
    int dx = x1 - x0;
    int dy = y1 - y0;

    int steps;

    if (dx < 0)
        dx = -dx;

    if (dy < 0)
        dy = -dy;

    steps = dx > dy ? dx : dy;

    for (int i = 0; i <= steps; i++)
    {
        int x = x0 + (x1 - x0) * i / steps;
        int y = y0 + (y1 - y0) * i / steps;

        put_pixel(x, y, color);
    }
}
void draw_square(int x, int y, int size, unsigned int color)
{
    for(int i = 0; i < size; i++)
    {
        put_pixel(x+i, y, color);
        put_pixel(x+i, y+size, color);

        put_pixel(x, y+i, color);
        put_pixel(x+size, y+i, color);
    }
}
int console_x = 10;
int console_y = 10;

unsigned int console_color = 0x0000FF00;
typedef signed int int32_t;
typedef unsigned int uint32_t;

void put_pixel(int x, int y, unsigned int color) {
    if (framebuffer == 0)
        return;

    if (x < 0 || y < 0)
        return;

    if ((unsigned int)x >= framebuffer_width)
        return;

    if ((unsigned int)y >= framebuffer_height)
        return;

    unsigned int* pixel =
        (unsigned int*)((unsigned char*)framebuffer + y * framebuffer_pitch);

    pixel[x] = color;
}

/* Pomocnicza funkcja licząca wartość bezwzględną */
static inline int32_t kernel_abs(int32_t value) {
    return (value < 0) ? -value : value;
}

/**
 * Generuje piksele linii i zapisuje je do tablicy (x, y, kolor)
 */
int32_t line_pp(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color, int32_t out_array[][3], int32_t max_pixels) {
    if (out_array == (void*)0 || max_pixels <= 0) {
        return -1;
    }

    int32_t dx = kernel_abs(x2 - x1);
    int32_t sx = (x1 < x2) ? 1 : -1;
    int32_t dy = -kernel_abs(y2 - y1);
    int32_t sy = (y1 < y2) ? 1 : -1;
    int32_t err = dx + dy;
    int32_t e2;

    int32_t count = 0;

    while (1) {
        if (count >= max_pixels) {
            return -1;
        }

        out_array[count][0] = x1;
        out_array[count][1] = y1;
        out_array[count][2] = (int32_t)color;
        count++;

        if (x1 == x2 && y1 == y2) {
            break;
        }

        e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x1 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y1 += sy;
        }
    }

    return count;
}

/**
 * Wyznacza trójkąt i zapisuje punkty do tablicy out_array
 */
int32_t triangle_pp(int32_t x1, int32_t y1, 
                    int32_t x2, int32_t y2, 
                    int32_t x3, int32_t y3, 
                    uint32_t color,
                    int32_t out_array[][3], 
                    int32_t max_pixels) 
{
    if (out_array == (void*)0 || max_pixels <= 0) {
        return -1;
    }

    int32_t total_count = 0;

    // 1. Linia od P1 do P2
    int32_t count1 = line_pp(x1, y1, x2, y2, color, &out_array[total_count], max_pixels - total_count);
    if (count1 < 0) return -1;
    total_count += count1;

    // 2. Linia od P2 do P3
    int32_t count2 = line_pp(x2, y2, x3, y3, color, &out_array[total_count], max_pixels - total_count);
    if (count2 < 0) return -1;
    total_count += count2;

    // 3. Linia od P3 do P1 (zamknięcie trójkąta)
    int32_t count3 = line_pp(x3, y3, x1, y1, color, &out_array[total_count], max_pixels - total_count);
    if (count3 < 0) return -1;
    total_count += count3;

    return total_count;
}

/**
 * Funkcja pomocnicza – bezpośrednio rysuje wygenerowane punkty na ekranie
 */
void draw_triangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint32_t color) {
    int32_t buffer[2048][3];
    int32_t count = triangle_pp(x1, y1, x2, y2, x3, y3, color, buffer, 2048);

    if (count > 0) {
        for (int32_t i = 0; i < count; i++) {
            put_pixel(buffer[i][0], buffer[i][1], (uint32_t)buffer[i][2]);
        }
    }
}

void console_newline()
{
    console_x = 10;
    console_y += 10;
}

void console_put_char(char c)
{
    if (c == '\n')
    {
        console_newline();
        return;
    }

    draw_char(console_x, console_y, c, console_color);

    console_x += 8;

    if (console_x + 8 >= framebuffer_width)
    {
        console_newline();
    }

    if (console_y + 8 >= framebuffer_height)
    {
        console_y = 10;
    }
}

void console_print(const char* text)
{
    int i = 0;

    while (text[i])
    {
        console_put_char(text[i]);
        i++;
    }
}

void console_clear()
{
    clear_screen(0x00000000);

    console_x = 10;
    console_y = 10;
}






struct multiboot_info
{
    unsigned int total_size;
    unsigned int reserved;
};

struct multiboot_tag
{
    unsigned int type;
    unsigned int size;
};
struct multiboot_tag_framebuffer
{
    unsigned int type;
    unsigned int size;

    unsigned long long framebuffer_addr;

    unsigned int framebuffer_pitch;
    unsigned int framebuffer_width;
    unsigned int framebuffer_height;

    unsigned char framebuffer_bpp;
    unsigned char framebuffer_type;
    unsigned short reserved;
};
void print(const char* text, int post)
{
console_print(text);
}
static inline unsigned char inb(unsigned short port)
{
    unsigned char value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}
unsigned char keyboard_has_key()
{
    return inb(0x64) & 1;
}
unsigned char get_scancode()
{
    while ((inb(0x64) & 1) == 0);

    return inb(0x60);
}
char keymap[128] =
{
    [0x02]='1',
    [0x03]='2',
    [0x04]='3',
    [0x05]='4',
    [0x06]='5',
    [0x07]='6',
    [0x08]='7',
    [0x09]='8',
    [0x0A]='9',
    [0x0B]='0',

    [0x10]='q',
    [0x11]='w',
    [0x12]='e',
    [0x13]='r',
    [0x14]='t',
    [0x15]='y',
    [0x16]='u',
    [0x17]='i',
    [0x18]='o',
    [0x19]='p',

    [0x1E]='a',
    [0x1F]='s',
    [0x20]='d',
    [0x21]='f',
    [0x22]='g',
    [0x23]='h',
    [0x24]='j',
    [0x25]='k',
    [0x26]='l',

    [0x2C]='z',
    [0x2D]='x',
    [0x2E]='c',
    [0x2F]='v',
    [0x30]='b',
    [0x31]='n',
    [0x32]='m',

    [0x39]=' '
};
void print_int(int n, int pos)
{
    char buf[12];
    int i = 0;

    if (n == 0)
    {
 //       vga[pos] = (0x0F << 8) | '0';
        return;
    }

    while (n > 0)
    {
        buf[i++] = '0' + (n % 10);
        n /= 10;
    }

    while (i > 0)
    {
        i--;
   //     vga[pos++] = (0x0F << 8) | buf[i];
    }
}
void clear_screen(unsigned int color)
{
    for (unsigned int y = 0; y < framebuffer_height; y++)
    {
        for (unsigned int x = 0; x < framebuffer_width; x++)
        {
            put_pixel(x, y, color);
        }
    }
}
void beep(unsigned int frequency)
{
    unsigned int divisor = 1193180 / frequency;

    __asm__ volatile (
        "movb $0xB6, %%al\n"
        "outb %%al, $0x43\n"
        "movb %b0, %%al\n"
        "outb %%al, $0x42\n"
        "movb %b1, %%al\n"
        "outb %%al, $0x42\n"
        :
        : "a"(divisor), "b"(divisor >> 8)
    );

    unsigned char tmp = inb(0x61);

    if ((tmp & 3) != 3)
        __asm__ volatile ("orb $3, %%al\noutb %%al, $0x61" : : "a"(tmp));
}
void play_music(void)
{
    beep(659);
    beep(659);
    beep(659);

    beep(523);
    beep(659);
    beep(784);

    beep(392);
    beep(523);
    beep(392);
    beep(330);

    beep(440);
    beep(494);
    beep(466);
    beep(440);

    beep(392);
    beep(659);
    beep(784);
    beep(880);

    beep(698);
    beep(784);
    beep(659);
    beep(523);
}
/* Pomocnicza funkcja do zamiany wartości */
static inline void swap_int(int32_t *a, int32_t *b) {
    int32_t temp = *a;
    *a = *b;
    *b = temp;
}

/**
 * Rysuje poziomy odcinek od x1 do x2 na wysokości y
 */
static void draw_horizontal_line(int32_t x1, int32_t x2, int32_t y, uint32_t color) {
    if (x1 > x2) {
        swap_int(&x1, &x2);
    }
    for (int32_t x = x1; x <= x2; x++) {
        put_pixel(x, y, color);
    }
}

/**
 * Rysuje WYPEŁNIONY trójkąt
 */
void draw_filled_triangle(int32_t x1, int32_t y1, 
                         int32_t x2, int32_t y2, 
                         int32_t x3, int32_t y3, 
                         uint32_t color) 
{
    // 1. Sortowanie wierzchołków według osi Y (y1 <= y2 <= y3)
    if (y1 > y2) { swap_int(&x1, &x2); swap_int(&y1, &y2); }
    if (y1 > y3) { swap_int(&x1, &x3); swap_int(&y1, &y3); }
    if (y2 > y3) { swap_int(&x2, &x3); swap_int(&y2, &y3); }

    // Jeśli trójkąt nie ma wysokości, nic nie rysujemy
    if (y1 == y3) return;

    // 2. Rysowanie górnej części trójkąta (od y1 do y2)
    for (int32_t y = y1; y <= y2; y++) {
        int32_t height1 = y2 - y1 + 1;
        int32_t height2 = y3 - y1 + 1;

        int32_t xa = x1 + (x2 - x1) * (y - y1) / (height1 ? height1 : 1);
        int32_t xb = x1 + (x3 - x1) * (y - y1) / (height2 ? height2 : 1);

        draw_horizontal_line(xa, xb, y, color);
    }

    // 3. Rysowanie dolnej części trójkąta (od y2 do y3)
    for (int32_t y = y2; y <= y3; y++) {
        int32_t height1 = y3 - y2 + 1;
        int32_t height2 = y3 - y1 + 1;

        int32_t xa = x2 + (x3 - x2) * (y - y2) / (height1 ? height1 : 1);
        int32_t xb = x1 + (x3 - x1) * (y - y1) / (height2 ? height2 : 1);

        draw_horizontal_line(xa, xb, y, color);
    }
}
typedef signed int int32_t;
typedef unsigned int uint32_t;

#define MAX_VERTICES 64
#define MAX_TRIANGLES 64
#define MAX_LINES 64

/* -------------------------------------------------------------------------- */
/* 1. TYPY DANYCH I STRUKTURY                                                 */
/* -------------------------------------------------------------------------- */

typedef struct {
    int32_t x, y, z;
} Point3D;

typedef struct {
    int32_t x, y;
} Point2D;

typedef struct {
    int32_t p1, p2, p3; // Indeksy do tablicy wierzchołków
    uint32_t color;
} TriangleIndices;

typedef struct {
    int32_t p1, p2;     // Indeksy do tablicy wierzchołków
    uint32_t color;
} LineIndices;

/* Główny pakiet danych obiektu 3D */
typedef struct {
    // Definicja obiektu
    Point3D local_vertices[MAX_VERTICES];   // Wierzchołki bazowe (niezmienne)
    Point3D transformed_vertices[MAX_VERTICES]; // Wierzchołki po obrocie i przesunięciu
    Point2D projected_vertices[MAX_VERTICES];   // Wierzchołki po rzutowaniu na 2D
    int32_t vertex_count;

    TriangleIndices triangles[MAX_TRIANGLES]; // Trójkąty
    int32_t triangle_count;

    LineIndices lines[MAX_LINES];             // Dodatkowe linie
    int32_t line_count;

    // Pozycja i rotacja w świecie (kąty w stopniach 0-359)
    int32_t rot_x, rot_y, rot_z;
    int32_t pos_x, pos_y, pos_z;
} Mesh3D;

/* -------------------------------------------------------------------------- */
/* 2. PROSTA TABLICA LUT DLA SINUS / COSINUS (skalowana x1024 dla int)        */
/* -------------------------------------------------------------------------- */

/* Wartości sin(deg) * 1024 dla deg = 0..90 (co 15 stopni dla oszczędności) */
static const int32_t sin_lut_90[7] = { 0, 265, 512, 724, 886, 989, 1024 };

/* Pomocnicza funkcja zwaracająca sin(deg) * 1024 dla deg 0-359 */
static int32_t kernel_sin_deg(int32_t deg) {
    deg = deg % 360;
    if (deg < 0) deg += 360;

    if (deg <= 90)  return sin_lut_90[(deg + 7) / 15];
    if (deg <= 180) return sin_lut_90[((180 - deg) + 7) / 15];
    if (deg <= 270) return -sin_lut_90[((deg - 180) + 7) / 15];
    return -sin_lut_90[((360 - deg) + 7) / 15];
}

static int32_t kernel_cos_deg(int32_t deg) {
    return kernel_sin_deg(deg + 90);
}

/* -------------------------------------------------------------------------- */
/* 3. LOGIKA SILNIKA 3D (OBRÓT, RZUTOWANIE, RENDEROWANIE)                      */
/* -------------------------------------------------------------------------- */

/**
 * Obraca i przesuwa wierzchołki obiektu oraz rzutuje je na ekran 2D
 */
void update_mesh_transform(Mesh3D *mesh) {
    int32_t sin_x = kernel_sin_deg(mesh->rot_x);
    int32_t cos_x = kernel_cos_deg(mesh->rot_x);
    int32_t sin_y = kernel_sin_deg(mesh->rot_y);
    int32_t cos_y = kernel_cos_deg(mesh->rot_y);
    int32_t sin_z = kernel_sin_deg(mesh->rot_z);
    int32_t cos_z = kernel_cos_deg(mesh->rot_z);

    int32_t center_x = (int32_t)framebuffer_width / 2;
    int32_t center_y = (int32_t)framebuffer_height / 2;
    int32_t focal_length = 300;

    for (int i = 0; i < mesh->vertex_count; i++) {
        int32_t x = mesh->local_vertices[i].x;
        int32_t y = mesh->local_vertices[i].y;
        int32_t z = mesh->local_vertices[i].z;

        // 1. Obrót wokół osi X
        int32_t y1 = (y * cos_x - z * sin_x) / 1024;
        int32_t z1 = (y * sin_x + z * cos_x) / 1024;

        // 2. Obrót wokół osi Y
        int32_t x2 = (x * cos_y + z1 * sin_y) / 1024;
        int32_t z2 = (-x * sin_y + z1 * cos_y) / 1024;

        // 3. Obrót wokół osi Z
        int32_t x3 = (x2 * cos_z - y1 * sin_z) / 1024;
        int32_t y3 = (x2 * sin_z + y1 * cos_z) / 1024;

        // 4. Przesunięcie w przestrzeni świata (World Translation)
        mesh->transformed_vertices[i].x = x3 + mesh->pos_x;
        mesh->transformed_vertices[i].y = y3 + mesh->pos_y;
        mesh->transformed_vertices[i].z = z2 + mesh->pos_z;

        // 5. Rzutowanie perspektywiczne na ekran 2D
        int32_t tz = mesh->transformed_vertices[i].z;
        if (tz <= 1) tz = 1; // Zabezpieczenie przed dzieleniem przez <= 0

        mesh->projected_vertices[i].x = center_x + (mesh->transformed_vertices[i].x * focal_length) / tz;
        mesh->projected_vertices[i].y = center_y - (mesh->transformed_vertices[i].y * focal_length) / tz;
    }
}

/**
 * Rysuje obiekt na ekranie na podstawie przetworzonych punktów
 */
void render_mesh(Mesh3D *mesh) {
    // Rysowanie trójkątów
    for (int i = 0; i < mesh->triangle_count; i++) {
        int p1 = mesh->triangles[i].p1;
        int p2 = mesh->triangles[i].p2;
        int p3 = mesh->triangles[i].p3;

        draw_filled_triangle(
            mesh->projected_vertices[p1].x, mesh->projected_vertices[p1].y,
            mesh->projected_vertices[p2].x, mesh->projected_vertices[p2].y,
            mesh->projected_vertices[p3].x, mesh->projected_vertices[p3].y,
            mesh->triangles[i].color
        );
    }

    // Rysowanie linii
    int32_t line_buf[1024][3];
    for (int i = 0; i < mesh->line_count; i++) {
        int p1 = mesh->lines[i].p1;
        int p2 = mesh->lines[i].p2;

        int32_t count = line_pp(
            mesh->projected_vertices[p1].x, mesh->projected_vertices[p1].y,
            mesh->projected_vertices[p2].x, mesh->projected_vertices[p2].y,
            mesh->lines[i].color,
            line_buf, 1024
        );

        if (count > 0) {
            for (int j = 0; j < count; j++) {
                put_pixel(line_buf[j][0], line_buf[j][1], (uint32_t)line_buf[j][2]);
            }
        }
    }
}



/**
 * Czyszczenie tylko wybranego prostokątnego obszaru na ekranie
 */
void clear_rect(int32_t min_x, int32_t min_y, int32_t max_x, int32_t max_y, uint32_t color) {
    if (framebuffer == 0) return;

    // Zabezpieczenie przed wyjściem poza granice ekranu (clipping)
    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if ((uint32_t)max_x >= framebuffer_width)  max_x = framebuffer_width - 1;
    if ((uint32_t)max_y >= framebuffer_height) max_y = framebuffer_height - 1;

    for (int32_t y = min_y; y <= max_y; y++) {
        for (int32_t x = min_x; x <= max_x; x++) {
            put_pixel(x, y, color);
        }
    }
}
typedef struct {
    int32_t min_x, min_y;
    int32_t max_x, max_y;
}BoundingBox;

static BoundingBox prev_box = { 0, 0, 0, 0 };

BoundingBox get_mesh_bounds(Mesh3D *mesh) {
    BoundingBox box;

    if (mesh->vertex_count == 0) {
        box.min_x = box.min_y = box.max_x = box.max_y = 0;
        return box;
    }

    box.min_x = mesh->projected_vertices[0].x;
    box.max_x = mesh->projected_vertices[0].x;
    box.min_y = mesh->projected_vertices[0].y;
    box.max_y = mesh->projected_vertices[0].y;

    for (int i = 1; i < mesh->vertex_count; i++) {
        int32_t x = mesh->projected_vertices[i].x;
        int32_t y = mesh->projected_vertices[i].y;

        if (x < box.min_x) box.min_x = x;
        if (x > box.max_x) box.max_x = x;
        if (y < box.min_y) box.min_y = y;
        if (y > box.max_y) box.max_y = y;
    }

    box.min_x -= 5;
    box.min_y -= 5;
    box.max_x += 5;
    box.max_y += 5;

    return box;
}

void draw_3d_frame(Mesh3D *mesh) {
    clear_rect(prev_box.min_x, prev_box.min_y, prev_box.max_x, prev_box.max_y, 0x00000000);

    mesh->rot_y = (mesh->rot_y + 3) % 360;
    mesh->rot_x = (mesh->rot_x + 1) % 360;

    update_mesh_transform(mesh);
    prev_box = get_mesh_bounds(mesh);
    render_mesh(mesh);
}
//to jest do nanimacji 3d        tutej nie tamMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
void run_3d_demo(int32_t pos_x, int32_t pos_y) {
    Mesh3D pyramid = {
        .local_vertices = {
         { -40, 80, -40}, //0
         { -40, 80, 40},//1
         { 40, 80, 40 },//2
         { 40, 80, -40 },//3
         { -40, 0, -40 },//4
         { -40, 0, 40 },//5
         { 40, 0, 40 },//6
         { 40, 0, -40 },//7
         { 0, 100, 0 }//8
        },
        .vertex_count = 9,

        .triangles = {
            { 0, 1, 2, 0x00FF0000 },
            { 0, 3, 2, 0x00FF0000 },
            { 4, 5, 6, 0x00FFFF00 },
            { 4, 7, 6, 0x00FFFF00 },

            { 0, 4, 1, 0x0000FF00 },
            { 5, 4, 1, 0x0000FF00 },
            { 2, 7, 3, 0x00FF00FF },
            { 2, 7, 6, 0x00FF00FF },

            { 0, 7, 4, 0x00FF8800 },
            { 0, 7, 3, 0x00FF8800 },
            { 1, 6, 5, 0x000000FF },
            { 1, 6, 2, 0x000000FF },

            { 8, 0, 1, 0x00888800 },
            { 8, 1, 2, 0x000000FF },
            { 8, 2, 3, 0x00888800 },
            { 8, 3, 0, 0x000000FF }
        },
        .triangle_count = 16,

        .lines = {
        { 0, 6, 0x00000000 }
        },
        .line_count = 1,

        .rot_x = 0,
        .rot_y = 0,
        .rot_z = 0,
        .pos_x = pos_x,
        .pos_y = pos_y,
        .pos_z = 600
    };

    while (1) {
        if (inb(0x60) == 0x01) {
            clear_rect(prev_box.min_x, prev_box.min_y, prev_box.max_x, prev_box.max_y, 0x00000000);
            break;
        }

        draw_3d_frame(&pyramid);

        for (volatile int i = 0; i < 1000000; i++);
    }
}
void run_3d_demo2(int32_t pos_x, int32_t pos_y) {
    Mesh3D p2 = {
        .local_vertices = {
        { 0, 0, 0 }
        },
        .vertex_count = 14,

        .triangles = {
            { 0, 1, 2, 0x00FF0000 },
            { 0, 3, 2, 0x00FF0000 },
            { 4, 5, 6, 0x00FFFF00 },
            { 4, 7, 6, 0x00FFFF00 },

            { 0, 4, 1, 0x0000FF00 },
            { 5, 4, 1, 0x0000FF00 },
            { 2, 7, 3, 0x00FF00FF },
            { 2, 7, 6, 0x00FF00FF },

            { 0, 7, 4, 0x00FF8800 },
            { 0, 7, 3, 0x00FF8800 },
            { 1, 6, 5, 0x000000FF },
            { 1, 6, 2, 0x000000FF },

            { 8, 0, 1, 0x00888800 },
            { 8, 1, 2, 0x000000FF },
            { 8, 2, 3, 0x00888800 },
            { 8, 3, 0, 0x000000FF }
        },
        .triangle_count = 22 ,

        .lines = {
        { 0, 6, 0x00000000 }
        },
        .line_count = 1,

        .rot_x = 0,
        .rot_y = 0,
        .rot_z = 0,
        .pos_x = pos_x,
        .pos_y = pos_y,
        .pos_z = 600
    };

    while (1) {
        if (inb(0x60) == 0x01) {
            clear_rect(prev_box.min_x, prev_box.min_y, prev_box.max_x, prev_box.max_y, 0x00000000);
            break;
        }

        draw_3d_frame(&p2);

        for (volatile int i = 0; i < 1000000; i++);
    }


}
/* Pomocnicza funkcja do zapisu słowa (16-bit) do portu I/O */
static inline void outw(unsigned short port, unsigned short val) {
    __asm__ __volatile__ ("outw %0, %1" : : "a"(val), "Nd"(port));
}

/* Funkcja wyłączająca system w QEMU */
void shutdown(void) {
    // Wysyła sygnał ACPI Shutdown do domyślnego portu QEMU
    outw(0x604, 0x2000);

    // Zapasowe wyłączenie dla starszych wersji QEMU / Bochs
    outw(0xB004, 0x2000);

    // Jeśli wyłączenie z jakiegoś powodu się nie powiedzie, zatrzymaj CPU
    while (1) {
        __asm__ __volatile__ ("cli; hlt");
    }
}

void kernel_main(unsigned int magic, void* mbi)


{
if (magic != 0x36D76289)
{
    while (1)
    {
        __asm__("hlt");
    }
}
struct multiboot_info* info = (struct multiboot_info*)mbi;

struct multiboot_tag* tag =
    (struct multiboot_tag*)((char*)mbi + 8);
while (tag->type != 0)
{
if (tag->type == 8)
{
    struct multiboot_tag_framebuffer* fb =
        (struct multiboot_tag_framebuffer*)tag;

    framebuffer = (unsigned int*)(unsigned long)fb->framebuffer_addr;

    framebuffer_pitch = fb->framebuffer_pitch;
    framebuffer_width = fb->framebuffer_width;
    framebuffer_height = fb->framebuffer_height;
}
    // tutaj później będziemy analizować tagi


    tag = (struct multiboot_tag*)
    (
        (char*)tag +
        ((tag->size + 7) & ~7)
    );
}
init_font();
put_pixel(300, 300, 0x00FF0000);
clear_screen(0x000000);   // czarny
put_pixel(302, 300, 0x0000FF00);
clear_screen(0x000000);   // niebieski
draw_line(400,400,500,400,0x00FF0000);
draw_full_square(500,500,100,0x00FFFF00);
draw_char(100, 100, 'A', 0xFFFFFFFF);
put_pixel(300, 300, 0xFF000000);
draw_string(100, 10, "LOTOS OS", 0x00FFFFFF);
draw_string(100, 10, "OOOOOOOO", 0x00108040);

load_bmp(0, 0);

console_print("LOTOS OS wersja-[0.6.0]\n");
console_print("        FIRST 3D       \n");
console_print("-----------------------\n");
draw_filled_triangle(100, 100, 200, 200, 400, 50, 0x00FF0011);
draw_full_square(800, 300, 10, 0x10105060);
while (1)
{
    unsigned char sc = get_scancode();

    // Ignoruj puszczenie klawisza
    if (sc & 0x80)
        continue;

    // ENTER
if (sc == 0x1C)
{
    command[command_pos] = '\0';

    execute_command(command);

    command_pos = 0;

    console_x = 10;
    console_y += 10;

    continue;
}
    char c = keymap[sc];

    if (c)
    {
        console_input(c);
    }
}
}
