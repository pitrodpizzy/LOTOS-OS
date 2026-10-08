#include "games.h"
#include "kernel.h"

/*
 * ============================================================
 * LOTOS GAMES
 * Snake / Pong / Tetris / Saper / Mario
 * ============================================================
 */

/* ============================================================
   WSPÓLNE FUNKCJE
   ============================================================ */

static unsigned int random_seed = 123456789;

static unsigned int game_random(void)
{
    random_seed = random_seed * 1103515245 + 12345;
    return (random_seed >> 16) & 0x7FFF;
}

static void game_rect(int x, int y, int w, int h, unsigned int color)
{
    int xx;
    int yy;

    if (w <= 0 || h <= 0)
        return;

    for (yy = 0; yy < h; yy++)
    {
        for (xx = 0; xx < w; xx++)
        {
            put_pixel(x + xx, y + yy, color);
        }
    }
}

static void game_border(
    int x,
    int y,
    int w,
    int h,
    int thickness,
    unsigned int color)
{
    game_rect(x, y, w, thickness, color);
    game_rect(x, y + h - thickness, w, thickness, color);
    game_rect(x, y, thickness, h, color);
    game_rect(x + w - thickness, y, thickness, h, color);
}

/* ============================================================
   SAPER - GRAFIKA MINY
   ============================================================ */

static void draw_mine_graphic(int x, int y, int size)
{
    unsigned int black = 0x00000000;
    unsigned int dark  = 0x00101010;
    unsigned int red   = 0x00FF2020;
    unsigned int white = 0x00FFFFFF;

    int c = size / 2;

    /* kolce */
    game_rect(x + c - 2, y + 2, 4, 8, black);
    game_rect(x + c - 2, y + size - 10, 4, 8, black);

    game_rect(x + 2, y + c - 2, 8, 4, black);
    game_rect(x + size - 10, y + c - 2, 8, 4, black);

    /* przekątne */
    game_rect(x + 7, y + 7, 6, 6, black);
    game_rect(x + size - 13, y + 7, 6, 6, black);
    game_rect(x + 7, y + size - 13, 6, 6, black);
    game_rect(x + size - 13, y + size - 13, 6, 6, black);

    /* główna kula */
    game_rect(x + 8, y + 8, size - 16, size - 16, black);

    /* cień */
    game_rect(x + 11, y + 11, size - 20, size - 20, dark);

    /* czerwony środek */
    game_rect(x + c - 6, y + c - 6, 12, 12, red);

    /* błysk */
    game_rect(x + c - 5, y + c - 5, 4, 4, white);
}

/* ============================================================
   SAPER - GRAFIKA FLAGI
   ============================================================ */

static void draw_flag_graphic(int x, int y, int size)
{
    unsigned int black = 0x00000000;
    unsigned int red   = 0x00FF0000;

    int pole_x = x + size / 2;

    /* maszt */
    game_rect(
        pole_x - 2,
        y + 5,
        4,
        size - 9,
        black
    );

    /* chorągiewka */
    game_rect(
        pole_x + 2,
        y + 6,
        size / 2 - 2,
        size / 4,
        red
    );

    game_rect(
        pole_x + 2,
        y + 6 + size / 4,
        size / 3,
        4,
        red
    );

    /* podstawa */
    game_rect(
        pole_x - size / 4,
        y + size - 7,
        size / 2,
        4,
        black
    );
}

/* ============================================================
   SAPER - CYFRY
   ============================================================ */

static unsigned int mine_number_color(int n)
{
    if (n == 1) return 0x000000FF;
    if (n == 2) return 0x00008000;
    if (n == 3) return 0x00FF0000;
    if (n == 4) return 0x00000080;
    if (n == 5) return 0x00800000;
    if (n == 6) return 0x00008080;
    if (n == 7) return 0x00000000;

    return 0x00606060;
}

/* ============================================================
   SAPER
   ============================================================ */

#define MS_MAX_ROWS 16
#define MS_MAX_COLS 30

static unsigned char ms_mines[MS_MAX_ROWS][MS_MAX_COLS];
static unsigned char ms_state[MS_MAX_ROWS][MS_MAX_COLS];

static int ms_rows;
static int ms_cols;
static int ms_total_mines;
static int ms_first_click;

static int ms_count_mines(int r, int c)
{
    int dr;
    int dc;
    int count = 0;

    for (dr = -1; dr <= 1; dr++)
    {
        for (dc = -1; dc <= 1; dc++)
        {
            int rr = r + dr;
            int cc = c + dc;

            if (rr < 0 || rr >= ms_rows)
                continue;

            if (cc < 0 || cc >= ms_cols)
                continue;

            if (ms_mines[rr][cc])
                count++;
        }
    }

    return count;
}

static void ms_clear(void)
{
    int r;
    int c;

    for (r = 0; r < MS_MAX_ROWS; r++)
    {
        for (c = 0; c < MS_MAX_COLS; c++)
        {
            ms_mines[r][c] = 0;
            ms_state[r][c] = 0;
        }
    }
}

static void ms_generate(int safe_r, int safe_c)
{
    int placed = 0;

    while (placed < ms_total_mines)
    {
        int r = game_random() % ms_rows;
        int c = game_random() % ms_cols;

        if (r == safe_r && c == safe_c)
            continue;

        if (ms_mines[r][c])
            continue;

        ms_mines[r][c] = 1;
        placed++;
    }
}

static void ms_reveal(int r, int c)
{
    int dr;
    int dc;

    if (r < 0 || r >= ms_rows)
        return;

    if (c < 0 || c >= ms_cols)
        return;

    if (ms_state[r][c] == 1)
        return;

    if (ms_state[r][c] == 2)
        return;

    if (ms_mines[r][c])
        return;

    ms_state[r][c] = 1;

    if (ms_count_mines(r, c) != 0)
        return;

    for (dr = -1; dr <= 1; dr++)
    {
        for (dc = -1; dc <= 1; dc++)
        {
            if (dr == 0 && dc == 0)
                continue;

            ms_reveal(r + dr, c + dc);
        }
    }
}

static int ms_won(void)
{
    int r;
    int c;

    for (r = 0; r < ms_rows; r++)
    {
        for (c = 0; c < ms_cols; c++)
        {
            if (!ms_mines[r][c] && ms_state[r][c] != 1)
                return 0;
        }
    }

    return 1;
}

static void ms_show_all(void)
{
    int r;
    int c;

    for (r = 0; r < ms_rows; r++)
    {
        for (c = 0; c < ms_cols; c++)
        {
            if (ms_mines[r][c])
                ms_state[r][c] = 1;
        }
    }
}

static void ms_draw_cell(
    int x,
    int y,
    int size,
    int r,
    int c,
    int selected)
{
    unsigned int covered = 0x00606060;
    unsigned int opened  = 0x00B8B8B8;
    unsigned int border  = 0x00303030;
    unsigned int cursor  = 0x00FFFF00;

    if (ms_state[r][c] == 1)
    {
        game_rect(
            x,
            y,
            size - 2,
            size - 2,
            opened
        );

        game_border(
            x,
            y,
            size - 2,
            size - 2,
            2,
            border
        );

        if (ms_mines[r][c])
        {
            draw_mine_graphic(
                x + 2,
                y + 2,
                size - 6
            );
        }
        else
        {
            int n = ms_count_mines(r, c);

            if (n > 0)
            {
                char text[2];

                text[0] = '0' + n;
                text[1] = '\0';

                draw_string(
                    x + size / 2 - 4,
                    y + size / 2 - 6,
                    text,
                    mine_number_color(n)
                );
            }
        }
    }
    else
    {
        game_rect(
            x,
            y,
            size - 2,
            size - 2,
            covered
        );

        game_border(
            x,
            y,
            size - 2,
            size - 2,
            2,
            0x00888888
        );

        if (ms_state[r][c] == 2)
        {
            draw_flag_graphic(
                x + 2,
                y + 2,
                size - 6
            );
        }
    }

    if (selected)
    {
        game_border(
            x,
            y,
            size - 2,
            size - 2,
            3,
            cursor
        );
    }
}

static void ms_draw_board(int cur_r, int cur_c)
{
    int r;
    int c;
    int cell;

    clear_screen(0x00202020);

    draw_string(
        30,
        20,
        "LOTOS SAPER",
        0x00FFFFFF
    );

    draw_string(
        30,
        45,
        "STRZALKI - RUCH",
        0x00FFFFFF
    );

    draw_string(
        30,
        65,
        "ENTER - ODKRYJ",
        0x00FFFFFF
    );

    draw_string(
        30,
        85,
        "F - FLAGA",
        0x00FFFFFF
    );

    draw_string(
        30,
        105,
        "ESC - WYJSCIE",
        0x00FFFFFF
    );

    /*
     * Dobieramy rozmiar komórki tak,
     * żeby plansza się mieściła.
     */
    if (ms_cols >= 30)
        cell = 22;
    else if (ms_cols >= 16)
        cell = 30;
    else
        cell = 42;

    int board_w = ms_cols * cell;
    int board_x = (framebuffer_width - board_w) / 2;
    int board_y = 135;

    for (r = 0; r < ms_rows; r++)
    {
        for (c = 0; c < ms_cols; c++)
        {
            ms_draw_cell(
                board_x + c * cell,
                board_y + r * cell,
                cell,
                r,
                c,
                (r == cur_r && c == cur_c)
            );
        }
    }
}

void game_minesweeper(void)
{
    int selected = 0;
    int cur_r;
    int cur_c;
    int status = 0;

    /*
     * MENU TRUDNOŚCI
     */

    while (1)
    {
        clear_screen(0x00101010);

        draw_string(
            300,
            80,
            "LOTOS SAPER",
            0x00FFFFFF
        );

        draw_string(
            300,
            115,
            "WYBIERZ POZIOM",
            0x00FFFF00
        );

        draw_string(
            300,
            160,
            "EASY  9x9  10 MIN",
            selected == 0 ? 0x00FFFF00 : 0x00FFFFFF
        );

        draw_string(
            300,
            195,
            "NORMAL  16x16  40 MIN",
            selected == 1 ? 0x00FFFF00 : 0x00FFFFFF
        );

        draw_string(
            300,
            230,
            "HARD  16x30  99 MIN",
            selected == 2 ? 0x00FFFF00 : 0x00FFFFFF
        );

        draw_string(
            300,
            285,
            "GORA/DOL - WYBOR",
            0x00FFFFFF
        );

        draw_string(
            300,
            305,
            "ENTER - START",
            0x00FFFFFF
        );

        draw_string(
            300,
            325,
            "ESC - WYJSCIE",
            0x00FFFFFF
        );

        if (!keyboard_has_key())
            continue;

        unsigned char sc = get_scancode();

        if (sc & 0x80)
            continue;

        if (sc == 0x48 && selected > 0)
            selected--;

        if (sc == 0x50 && selected < 2)
            selected++;

        if (sc == 0x1C)
            break;

        if (sc == 0x01)
            return;
    }

    if (selected == 0)
    {
        ms_rows = 9;
        ms_cols = 9;
        ms_total_mines = 10;
    }
    else if (selected == 1)
    {
        ms_rows = 16;
        ms_cols = 16;
        ms_total_mines = 40;
    }
    else
    {
        ms_rows = 16;
        ms_cols = 30;
        ms_total_mines = 99;
    }

    ms_clear();

    ms_first_click = 1;

    cur_r = 0;
    cur_c = 0;

    /*
     * GRA
     */

    while (status == 0)
    {
        ms_draw_board(cur_r, cur_c);

        if (!keyboard_has_key())
            continue;

        unsigned char sc = get_scancode();

        if (sc & 0x80)
            continue;

        if (sc == 0x01)
            return;

        if (sc == 0x48 && cur_r > 0)
            cur_r--;

        if (sc == 0x50 && cur_r < ms_rows - 1)
            cur_r++;

        if (sc == 0x4B && cur_c > 0)
            cur_c--;

        if (sc == 0x4D && cur_c < ms_cols - 1)
            cur_c++;

        /*
         * ENTER
         */

        if (sc == 0x1C)
        {
            if (ms_state[cur_r][cur_c] == 2)
                continue;

            if (ms_first_click)
            {
                ms_generate(cur_r, cur_c);
                ms_first_click = 0;
            }

            if (ms_mines[cur_r][cur_c])
            {
                status = 2;
                ms_show_all();
            }
            else
            {
                ms_reveal(cur_r, cur_c);

                if (ms_won())
                    status = 1;
            }
        }

        /*
         * F - flaga
         */

        if (sc == 0x21)
        {
            if (ms_state[cur_r][cur_c] == 0)
            {
                ms_state[cur_r][cur_c] = 2;
            }
            else if (ms_state[cur_r][cur_c] == 2)
            {
                ms_state[cur_r][cur_c] = 0;
            }
        }
    }

    /*
     * KONIEC
     */

    ms_draw_board(cur_r, cur_c);

    if (status == 1)
    {
        draw_string(
            300,
            620,
            "WYGRANA!",
            0x0000FF00
        );
    }
    else
    {
        draw_string(
            300,
            620,
            "BOOM! PRZEGRANA!",
            0x00FF0000
        );
    }

    draw_string(
        250,
        650,
        "ENTER - NOWA GRA",
        0x00FFFFFF
    );

    draw_string(
        250,
        670,
        "ESC - WYJSCIE",
        0x00FFFFFF
    );

    while (1)
    {
        if (!keyboard_has_key())
            continue;

        unsigned char sc = get_scancode();

        if (sc & 0x80)
            continue;

        if (sc == 0x01)
            return;

        if (sc == 0x1C)
        {
            game_minesweeper();
            return;
        }
    }
}

/* ============================================================
   PONG
   ============================================================ */

void game_pong(void)
{
    int selected = 0;
    int mode;

    while (1)
    {
        clear_screen(0x00000000);

        draw_string(
            300,
            70,
            "LOTOS PONG",
            0x00FFFFFF
        );

        draw_string(
            300,
            110,
            "WYBIERZ TRYB",
            0x00FFFF00
        );

        draw_string(
            300,
            155,
            "EASY",
            selected == 0 ? 0x00FFFF00 : 0x00FFFFFF
        );

        draw_string(
            300,
            185,
            "NORMAL",
            selected == 1 ? 0x00FFFF00 : 0x00FFFFFF
        );

        draw_string(
            300,
            215,
            "HARD",
            selected == 2 ? 0x00FFFF00 : 0x00FFFFFF
        );

        draw_string(
            300,
            245,
            "ULTRA HARD",
            selected == 3 ? 0x00FFFF00 : 0x00FFFFFF
        );

        draw_string(
            300,
            275,
            "AI MASTER",
            selected == 4 ? 0x00FFFF00 : 0x00FFFFFF
        );

        draw_string(
            300,
            305,
            "2 PLAYERS",
            selected == 5 ? 0x00FFFF00 : 0x00FFFFFF
        );

        draw_string(
            300,
            360,
            "STRZALKI - GRACZ 1",
            0x00FFFFFF
        );

        draw_string(
            300,
            380,
            "W/S - GRACZ 2",
            0x00FFFFFF
        );

        draw_string(
            300,
            420,
            "ENTER - START",
            0x00FFFFFF
        );

        if (!keyboard_has_key())
            continue;

        unsigned char sc = get_scancode();

        if (sc & 0x80)
            continue;

        if (sc == 0x48 && selected > 0)
            selected--;

        if (sc == 0x50 && selected < 5)
            selected++;

        if (sc == 0x01)
            return;

        if (sc == 0x1C)
        {
            mode = selected;
            break;
        }
    }

    int p1_y = framebuffer_height / 2 - 40;
    int p2_y = framebuffer_height / 2 - 40;

    int ball_x = framebuffer_width / 2;
    int ball_y = framebuffer_height / 2;

    int ball_dx = 5;
    int ball_dy = 3;

    int score1 = 0;
    int score2 = 0;

    while (score1 < 10 && score2 < 10)
    {
        if (keyboard_has_key())
        {
            unsigned char sc = get_scancode();

            if (!(sc & 0x80))
            {
                if (sc == 0x01)
                    return;

                if (sc == 0x48 && p1_y > 20)
                    p1_y -= 18;

                if (sc == 0x50 &&
                    p1_y < framebuffer_height - 100)
                    p1_y += 18;

                if (mode == 5)
                {
                    if (sc == 0x11 && p2_y > 20)
                        p2_y -= 18;

                    if (sc == 0x1F &&
                        p2_y < framebuffer_height - 100)
                        p2_y += 18;
                }
            }
        }

        /*
         * AI
         */

        if (mode != 5)
        {
            int speed;

            if (mode == 0)
                speed = 2;
            else if (mode == 1)
                speed = 4;
            else if (mode == 2)
                speed = 7;
            else if (mode == 3)
                speed = 11;
            else
                speed = 16;

            if (p2_y + 40 < ball_y)
                p2_y += speed;

            if (p2_y + 40 > ball_y)
                p2_y -= speed;
        }

        ball_x += ball_dx;
        ball_y += ball_dy;

        if (ball_y <= 10)
        {
            ball_y = 10;
            ball_dy = -ball_dy;
        }

        if (ball_y >= framebuffer_height - 20)
        {
            ball_y = framebuffer_height - 20;
            ball_dy = -ball_dy;
        }

        /*
         * LEWA PAŁKA
         */

        if (ball_x <= 45 &&
            ball_x >= 25 &&
            ball_y >= p1_y &&
            ball_y <= p1_y + 80)
        {
            ball_x = 45;
            ball_dx = -ball_dx;
        }

        /*
         * PRAWA PAŁKA
         */

        if (ball_x >= framebuffer_width - 45 &&
            ball_x <= framebuffer_width - 25 &&
            ball_y >= p2_y &&
            ball_y <= p2_y + 80)
        {
            ball_x = framebuffer_width - 45;
            ball_dx = -ball_dx;
        }

        /*
         * PUNKTY
         */

        if (ball_x < 0)
        {
            score2++;
            ball_x = framebuffer_width / 2;
            ball_y = framebuffer_height / 2;
            ball_dx = 5;
        }

        if (ball_x > framebuffer_width)
        {
            score1++;
            ball_x = framebuffer_width / 2;
            ball_y = framebuffer_height / 2;
            ball_dx = -5;
        }

        /*
         * RYSOWANIE
         */

        clear_screen(0x00080808);

        draw_string(
            20,
            20,
            "LOTOS PONG",
            0x00FFFFFF
        );

        /*
         * linia środkowa
         */

        int yy;

        for (yy = 0; yy < framebuffer_height; yy += 30)
        {
            game_rect(
                framebuffer_width / 2 - 2,
                yy,
                4,
                15,
                0x00606060
            );
        }

        game_rect(
            20,
            p1_y,
            12,
            80,
            0x0000FF00
        );

        game_rect(
            framebuffer_width - 32,
            p2_y,
            12,
            80,
            0x00FF0000
        );

        game_rect(
            ball_x - 7,
            ball_y - 7,
            14,
            14,
            0x00FFFFFF
        );

        /*
         * wynik
         */

        char s1[2];
        char s2[2];

        s1[0] = '0' + score1;
        s1[1] = '\0';

        s2[0] = '0' + score2;
        s2[1] = '\0';

        draw_string(
            framebuffer_width / 2 - 50,
            30,
            s1,
            0x0000FF00
        );

        draw_string(
            framebuffer_width / 2 + 40,
            30,
            s2,
            0x00FF0000
        );

        for (volatile int delay = 0;
             delay < 1200000;
             delay++)
        {
        }
    }

    clear_screen(0);

    if (score1 >= 10)
    {
        draw_string(
            300,
            250,
            "PLAYER 1 WINS!",
            0x0000FF00
        );
    }
    else
    {
        draw_string(
            300,
            250,
            mode == 5 ?
            "PLAYER 2 WINS!" :
            "COMPUTER WINS!",
            0x00FF0000
        );
    }

    draw_string(
        280,
        300,
        "ENTER - JESZCZE RAZ",
        0x00FFFFFF
    );

    draw_string(
        280,
        325,
        "ESC - WYJSCIE",
        0x00FFFFFF
    );

    while (1)
    {
        if (!keyboard_has_key())
            continue;

        unsigned char sc = get_scancode();

        if (sc & 0x80)
            continue;

        if (sc == 0x01)
            return;

        if (sc == 0x1C)
        {
            game_pong();
            return;
        }
    }
}

/* ============================================================
   SNAKE
   ============================================================ */

void game_snake(void)
{
    #define SNAKE_MAX 100
    #define SNAKE_CELL 16

    int sx[SNAKE_MAX];
    int sy[SNAKE_MAX];

    int length = 5;
    int direction = 3;

    int food_x = 20;
    int food_y = 15;

    int score = 0;
    int running = 1;

    int i;

    for (i = 0; i < length; i++)
    {
        sx[i] = 10 - i;
        sy[i] = 10;
    }

    while (running)
    {
        if (keyboard_has_key())
        {
            unsigned char sc = get_scancode();

            if (!(sc & 0x80))
            {
                if (sc == 0x01)
                    return;

                if (sc == 0x48 && direction != 1)
                    direction = 0;

                if (sc == 0x50 && direction != 0)
                    direction = 1;

                if (sc == 0x4B && direction != 3)
                    direction = 2;

                if (sc == 0x4D && direction != 2)
                    direction = 3;
            }
        }

        for (i = length - 1; i > 0; i--)
        {
            sx[i] = sx[i - 1];
            sy[i] = sy[i - 1];
        }

        if (direction == 0)
            sy[0]--;

        if (direction == 1)
            sy[0]++;

        if (direction == 2)
            sx[0]--;

        if (direction == 3)
            sx[0]++;

        if (sx[0] < 0 ||
            sy[0] < 2 ||
            sx[0] >= framebuffer_width / SNAKE_CELL ||
            sy[0] >= framebuffer_height / SNAKE_CELL)
        {
            running = 0;
        }

        for (i = 1; i < length; i++)
        {
            if (sx[0] == sx[i] &&
                sy[0] == sy[i])
            {
                running = 0;
            }
        }

        if (sx[0] == food_x &&
            sy[0] == food_y)
        {
            if (length < SNAKE_MAX)
                length++;

            score++;

            food_x =
                game_random() %
                (framebuffer_width / SNAKE_CELL - 2);

            food_y =
                2 +
                game_random() %
                (framebuffer_height / SNAKE_CELL - 3);
        }

        clear_screen(0x00000000);

        draw_string(
            10,
            10,
            "LOTOS SNAKE",
            0x0000FF00
        );

        game_rect(
            food_x * SNAKE_CELL,
            food_y * SNAKE_CELL,
            SNAKE_CELL - 2,
            SNAKE_CELL - 2,
            0x00FF0000
        );

        for (i = 0; i < length; i++)
        {
            game_rect(
                sx[i] * SNAKE_CELL,
                sy[i] * SNAKE_CELL,
                SNAKE_CELL - 2,
                SNAKE_CELL - 2,
                i == 0 ?
                0x0000FF00 :
                0x00008000
            );
        }

        for (volatile int delay = 0;
             delay < 2500000;
             delay++)
        {
        }
    }

    clear_screen(0);

    draw_string(
        300,
        220,
        "GAME OVER!",
        0x00FF0000
    );

    draw_string(
        300,
        250,
        "ESC - WYJSCIE",
        0x00FFFFFF
    );

    while (1)
    {
        if (!keyboard_has_key())
            continue;

        unsigned char sc = get_scancode();

        if (!(sc & 0x80) && sc == 0x01)
            return;
    }
}

/* ============================================================
   TETRIS
   ============================================================ */

void game_tetris(void)
{
    #define TW 10
    #define TH 20
    #define TC 24

    unsigned char board[TH][TW];

    int x = 4;
    int y = 0;

    int i;
    int j;

    for (i = 0; i < TH; i++)
    {
        for (j = 0; j < TW; j++)
        {
            board[i][j] = 0;
        }
    }

    while (1)
    {
        if (keyboard_has_key())
        {
            unsigned char sc = get_scancode();

            if (!(sc & 0x80))
            {
                if (sc == 0x01)
                    return;

                if (sc == 0x4B && x > 0)
                    x--;

                if (sc == 0x4D && x < TW - 2)
                    x++;

                if (sc == 0x50)
                    y++;
            }
        }

        y++;

        if (y >= TH - 1)
        {
            if (y < TH)
                board[y][x] = 1;

            if (x + 1 < TW)
                board[y][x + 1] = 1;

            x = 4;
            y = 0;
        }

        clear_screen(0x00000000);

        draw_string(
            20,
            20,
            "LOTOS TETRIS",
            0x00FFFFFF
        );

        draw_string(
            20,
            45,
            "STRZALKI - RUCH",
            0x00FFFFFF
        );

        draw_string(
            20,
            65,
            "ESC - WYJSCIE",
            0x00FFFFFF
        );

        for (i = 0; i < TH; i++)
        {
            for (j = 0; j < TW; j++)
            {
                game_border(
                    220 + j * TC,
                    80 + i * TC,
                    TC,
                    TC,
                    1,
                    0x00303030
                );

                if (board[i][j])
                {
                    game_rect(
                        220 + j * TC + 2,
                        80 + i * TC + 2,
                        TC - 4,
                        TC - 4,
                        0x0000FFFF
                    );
                }
            }
        }

        game_rect(
            220 + x * TC + 2,
            80 + y * TC + 2,
            TC - 4,
            TC - 4,
            0x00FF0000
        );

        for (volatile int delay = 0;
             delay < 2200000;
             delay++)
        {
        }
    }
}

/* ============================================================
   MARIO
   ============================================================ */

void game_mario(void)
{
    int x = 80;
    int y = 300;

    int vx = 0;
    int vy = 0;

    int jumping = 0;

    int camera = 0;

    int score = 0;

    #define MARIO_WORLD 120

    unsigned char blocks[MARIO_WORLD];

    int i;

    for (i = 0; i < MARIO_WORLD; i++)
        blocks[i] = 0;

    blocks[8] = 1;
    blocks[9] = 1;
    blocks[15] = 1;
    blocks[16] = 1;
    blocks[17] = 1;
    blocks[25] = 1;
    blocks[26] = 1;
    blocks[40] = 1;
    blocks[41] = 1;
    blocks[55] = 1;
    blocks[56] = 1;
    blocks[70] = 1;
    blocks[71] = 1;
    blocks[85] = 1;
    blocks[86] = 1;

    while (1)
    {
        if (keyboard_has_key())
        {
            unsigned char sc = get_scancode();

            if (!(sc & 0x80))
            {
                if (sc == 0x01)
                    return;

                if (sc == 0x4B)
                    vx = -5;

                if (sc == 0x4D)
                    vx = 5;

                if (sc == 0x48 && !jumping)
                {
                    vy = -14;
                    jumping = 1;
                }
            }
        }
        else
        {
            vx = 0;
        }

        x += vx;
        y += vy;

        vy++;

        /*
         * Podłoga
         */

        if (y >= 340)
        {
            y = 340;
            vy = 0;
            jumping = 0;
        }

        if (x < 0)
            x = 0;

        if (x > MARIO_WORLD * 40 - 40)
            x = MARIO_WORLD * 40 - 40;

        if (x > 400)
            camera = x - 400;

        clear_screen(0x0087CEEB);

        /*
         * Chmury
         */

        game_rect(
            100 - camera / 4,
            80,
            80,
            25,
            0x00FFFFFF
        );

        game_rect(
            600 - camera / 5,
            110,
            100,
            25,
            0x00FFFFFF
        );

        /*
         * ziemia
         */

        game_rect(
            0,
            380,
            framebuffer_width,
            framebuffer_height - 380,
            0x008B4513
        );

        /*
         * bloki
         */

        for (i = 0; i < MARIO_WORLD; i++)
        {
            int bx;

            if (!blocks[i])
                continue;

            bx = i * 40 - camera;

            if (bx < -40 || bx > framebuffer_width)
                continue;

            game_rect(
                bx,
                300,
                38,
                38,
                0x00C87520
            );

            game_border(
                bx,
                300,
                38,
                38,
                2,
                0x00703010
            );
        }

        /*
         * Mario - głowa
         */

        game_rect(
            x - camera,
            y,
            20,
            20,
            0x00FF0000
        );

        /*
         * twarz
         */

        game_rect(
            x - camera + 4,
            y + 5,
            12,
            8,
            0x00FFC080
        );

        /*
         * ubranie
         */

        game_rect(
            x - camera,
            y + 20,
            20,
            15,
            0x000000FF
        );

        /*
         * nogi
         */

        game_rect(
            x - camera,
            y + 35,
            8,
            7,
            0x00502020
        );

        game_rect(
            x - camera + 12,
            y + 35,
            8,
            7,
            0x00502020
        );

        /*
         * wynik
         */

        draw_string(
            15,
            15,
            "LOTOS MARIO",
            0x00FFFFFF
        );

        /*
         * meta
         */

        if (x > 4500)
        {
            draw_string(
                300,
                200,
                "YOU WIN!",
                0x0000FF00
            );

            for (volatile int delay = 0;
                 delay < 30000000;
                 delay++)
            {
            }

            return;
        }

        for (volatile int delay = 0;
             delay < 1000000;
             delay++)
        {
        }
    }
}

/* ============================================================
   GAME CENTER
   ============================================================ */

void games_start(void)
{
    console_print("\n");
    console_print("========================\n");
    console_print("      LOTOS GAMES       \n");
    console_print("========================\n");
    console_print("Snake\n");
    console_print("Pong\n");
    console_print("Tetris\n");
    console_print("Saper\n");
    console_print("Mario\n");
    console_print("========================\n");
}
