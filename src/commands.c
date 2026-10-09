#include "kernel.h"
#include "commands.h"
#include "games.h"
static void command_help(void);
static void command_clear(void);
static void command_about(void);
static void command_version(void);
static void command_echo(void);
static void command_date(void);
static void command_memory(void);
static void command_cpu(void);
static void command_reboot(void);
static void command_shutdown(void);
static void command_music(void);
static void command_notepad(void);
static void command_logo(void);
static void command_color(void);
static void command_games(void);
static void command_saper(void);
static void command_snake(void);
static void command_tetris(void);
static void command_pong(void);
void command_mario(void);
void command_3d(void) {
    // Poprawiony draw_string (wymaga x, y, tekstu oraz koloru)
    draw_string(10, 10, "Uruchamianie animacji 3D...", 0x00FFFFFF);
    
    run_3d_demo(160, 100);
}
void command_3d2(void) {
    // Poprawiony draw_string (wymaga x, y, tekstu oraz koloru)
    draw_string(10, 10, "Uruchamianie animacji 3D...", 0x00FFFFFF);
    
    run_3d_demo2(0, 0);
}

static Command commands[] =
{
    {"help",     "Lista komend",                 command_help},
    {"clear",    "Wyczysc ekran i przywroc tlo", command_clear},
    {"about",    "Informacje o LOTOS",           command_about},
    {"version",  "Wersja systemu",               command_version},
  //  {"echo",     "Wyswietl tekst",               command_echo},
    //{"date",     "Data/czas systemowy",           command_date},
    {"memory",   "Informacje o pamieci",         command_memory},
    {"cpu",      "Informacje o procesorze",      command_cpu},
    {"reboot",   "Restart",                      command_reboot},
    {"shutdown", "Wylaczenie",                  command_shutdown},
    {"music",    "Muzyka",                       command_music},
   // {"notepad",  "LOTOS Notepad",                command_notepad},
    {"logo",     "Wyswietl logo",                command_logo},
   // {"color",    "Zmiana koloru konsoli",        command_color}
    {"games", "LOTOS Game Center", command_games},
    {"saper", "Gra Saper", command_saper},
    {"snake",  "Gra Snake",  command_snake},
{"tetris", "Gra Tetris", command_tetris},
{"pong",   "Gra Pong",   command_pong},
{"notepad",  "LOTOS Notepad", command_notepad},
{"mario", "Gra Mario", command_mario},
{"3d", "3D", command_3d},
{"3d2", "23d", command_3d2},
};
#define COMMAND_COUNT (sizeof(commands) / sizeof(commands[0]))

int string_equals(const char* a, const char* b)
{
    int i = 0;

    while (a[i] && b[i])
    {
        if (a[i] != b[i])
            return 0;

        i++;
    }

    return a[i] == b[i];
}
void command_mario(void)
{
    game_mario();
}
static void command_help(void)
{
    int y = console_y + 16;

    draw_string(10, y, "LOTOS COMMANDS:", 0x00FFFFFF);
    y += 16;

    draw_string(10, y, "help       - lista komend", 0x00FFFFFF); y += 16;
    draw_string(10, y, "clear      - wyczysc ekran i przywroc tlo", 0x00FFFFFF); y += 16;
    draw_string(10, y, "about      - informacje o LOTOS", 0x00FFFFFF); y += 16;
    draw_string(10, y, "version    - wersja systemu", 0x00FFFFFF); y += 16;
    //draw_string(10, y, "echo       - wyswietl tekst", 0x00FFFFFF); y += 16;
    //draw_string(10, y, "date       - data/czas systemowy", 0x00FFFFFF); y += 16;
    draw_string(10, y, "memory     - informacje o pamieci", 0x00FFFFFF); y += 16;
    draw_string(10, y, "cpu        - informacje o procesorze", 0x00FFFFFF); y += 16;
    draw_string(10, y, "reboot     - restart", 0x00FFFFFF); y += 16;
    draw_string(10, y, "shutdown   - wylaczenie", 0x00FFFFFF); y += 16;
    draw_string(10, y, "music      - muzyka", 0x00FFFFFF); y += 16;
   // draw_string(10, y, "notepad    - LOTOS Notepad", 0x00FFFFFF); y += 16;
    draw_string(10, y, "logo       - wyswietl logo", 0x00FFFFFF); y += 16;
    //draw_string(10, y, "color      - zmiana koloru konsoli", 0x00FFFFFF); y += 16;
draw_string(10, y, "games      - LOTOS Game Center", 0x00FFFFFF); y += 16;
draw_string(10, y, "3d         - animacja 3D", 0x10FFFDFF); y += 16;
draw_string(10, y, "3d2        - 2 animacja 3D", 0x10FFFDFF); y += 16;
    console_x = 10;
    console_y = y;
}

static void command_clear(void)
{
    clear_screen(0x000000);
    load_bmp(0, 0);

}

static void command_music(void)
{
    play_music();
}

void execute_command(const char* command)
{
    for (unsigned int i = 0; i < COMMAND_COUNT; i++)
    {
        if (string_equals(command, commands[i].name))
        {
            commands[i].function();
            return;
        }
    }

    draw_string(10, 40, "Unknown command", 0x00FFFFFF);
}
static void command_about(void)
{
    draw_string(10, console_y + 16,
        "LOTOS OS - wlasny system operacyjny",
        0x00FFFFFF);

    console_y += 16;
}

static void command_version(void)
{
    draw_string(10, console_y + 16, "LOTOS OS version 0.5.1 - SNAKE AND TETRIS", 0x00FFFFFF);

    console_y += 16;
}

static void command_memory(void)
{
    draw_string(10, console_y + 16,
        "Memory manager: LOTOS",
        0x00FFFFFF);

    console_y += 16;
}

static void command_cpu(void)
{
    draw_string(10, console_y + 16,
        "CPU: x86",
        0x00FFFFFF);

    console_y += 16;
}

static void command_reboot(void)
{
    draw_string(10, console_y + 16,
        "Restarting LOTOS...",
        0x00FFFFFF);

    console_y += 16;

    while (1)
    {
        __asm__ volatile ("cli");
        __asm__ volatile ("hlt");
    }
}
void command_shutdown(void) {
    draw_string(10, 10, "Wylaczanie systemu...", 0x00FF0000);
    shutdown();
}

static void command_logo(void)
{
    load_bmp(0, 0);

    console_y += 16;
}
static void command_games(void)
{
    games_start();
}
static void command_saper(void)
{
    game_minesweeper();
}
static void command_snake(void)
{
    game_snake();
}

static void command_tetris(void)
{
    game_tetris();
}

static void command_pong(void)
{
    game_pong();
}
void command_notepad(void)
{
    char text[4096];
    int pos = 0;

    clear_screen(0x00101010);

    draw_string(20, 20, "LOTOS NOTEPAD", 0x00FFFFFF);
    draw_string(20, 45, "ESC - WYJSCIE | BACKSPACE - USUN", 0x00808080);

    while (1)
    {
        if (!keyboard_has_key())
            continue;

        unsigned char sc = get_scancode();

        /* puszczenie klawisza */
        if (sc & 0x80)
            continue;

        /* ESC */
        if (sc == 0x01)
        {
            clear_screen(0x00000000);
            return;
        }

        /* BACKSPACE */
        if (sc == 0x0E)
        {
            if (pos > 0)
            {
                pos--;
                text[pos] = '\0';
            }
        }

        /*
         * ENTER
         * Na razie dodajemy znak nowej linii.
         */
        else if (sc == 0x1C)
        {
            if (pos < 4095)
            {
                text[pos++] = '\n';
                text[pos] = '\0';
            }
        }

        /*
         * SPACJA
         */
        else if (sc == 0x39)
        {
            if (pos < 4095)
            {
                text[pos++] = ' ';
                text[pos] = '\0';
            }
        }

        /*
         * LITERY
         */
        else
        {
            char c = 0;

            if (sc == 0x10) c = 'q';
            if (sc == 0x11) c = 'w';
            if (sc == 0x12) c = 'e';
            if (sc == 0x13) c = 'r';
            if (sc == 0x14) c = 't';
            if (sc == 0x15) c = 'y';
            if (sc == 0x16) c = 'u';
            if (sc == 0x17) c = 'i';
            if (sc == 0x18) c = 'o';
            if (sc == 0x19) c = 'p';

            if (sc == 0x1E) c = 'a';
            if (sc == 0x1F) c = 's';
            if (sc == 0x20) c = 'd';
            if (sc == 0x21) c = 'f';
            if (sc == 0x22) c = 'g';
            if (sc == 0x23) c = 'h';
            if (sc == 0x24) c = 'j';
            if (sc == 0x25) c = 'k';
            if (sc == 0x26) c = 'l';

            if (sc == 0x2C) c = 'z';
            if (sc == 0x2D) c = 'x';
            if (sc == 0x2E) c = 'c';
            if (sc == 0x2F) c = 'v';
            if (sc == 0x30) c = 'b';
            if (sc == 0x31) c = 'n';
            if (sc == 0x32) c = 'm';

            if (c != 0 && pos < 4095)
            {
                text[pos++] = c;
                text[pos] = '\0';
            }
        }

        /*
         * RYSOWANIE TEKSTU
         */
        clear_screen(0x00101010);

        draw_string(
            20,
            20,
            "LOTOS NOTEPAD",
            0x00FFFFFF
        );

        draw_string(
            20,
            45,
            "ESC - WYJSCIE | BACKSPACE - USUN",
            0x00808080
        );

        draw_string(
            20,
            80,
            text,
            0x00FFFFFF
        );
    }
}
