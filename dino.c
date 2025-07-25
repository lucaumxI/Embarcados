#include "address_map_arm.h"
#include <stdlib.h> // Para a função rand()
#include <stdio.h>  // Necessário para a função sprintf()

/* protótipos de função */
void video_box(int, int, int, int, short);
int  resample_rgb(int, int);
int  get_data_bits(int);
void draw_bitmap(int, int, const unsigned char [16][16], int, int, short);
void delay(int);
void draw_ground(int, short);
void video_text(int, int, char *);
void clear_char_buffer(void);

/* constantes */
#define STANDARD_X 320
#define STANDARD_Y 240
#define WHITE       0xFFFFFF
#define GREEN       0x00FF00
#define BLACK       0x000000
#define MAX_CACTI   3 
#define MAX_CLOUDS  5 

/* estados da máquina */
typedef enum {
    STATE_START,
    STATE_PLAY,
    STATE_GAMEOVER
} GameState;

/* variáveis globais */
int screen_x, screen_y, res_offset, col_offset;

/* Bitmaps */
const unsigned char cloud_bitmap[16][16] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0},{0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0},{0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0},{0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},{0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};
const unsigned char bird_bitmap[16][16] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,1,1,1,0,0,1,1,1,1,1,0},{0,0,0,0,1,1,0,0,1,1,1,1,1,0,0,0},{0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0},
    {0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0},{0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0},{0,0,0,0,1,1,0,1,0,1,1,1,0,0,0,0},{0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0},{0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};
const unsigned char dino_bitmap[16][16] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0},{0,0,0,0,0,0,0,0,0,1,1,1,1,0,1,1},{0,0,0,0,1,1,0,0,0,1,1,1,1,1,1,1},{0,0,0,1,1,0,0,0,0,1,1,1,1,1,1,1},
    {0,0,1,1,1,0,0,0,0,0,1,1,1,1,0,0},{0,0,1,1,0,0,0,1,1,1,1,1,1,1,0,0},{0,0,1,1,0,0,1,1,1,1,1,1,1,0,0,0},{0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},{0,0,0,1,1,1,1,1,1,1,1,1,0,0,1,0},
    {0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0},{0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0},{0,0,0,0,0,1,1,0,0,1,0,0,0,0,0,0},{0,0,0,0,0,1,0,0,0,1,1,0,0,0,0,0},{0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};
const unsigned char cactus_bitmap[16][16] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0},{0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0},{0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,1,1,0,0,1,1,0,0,0},{0,0,0,0,0,0,1,1,1,0,0,1,1,0,0,0},{0,0,0,0,0,0,1,1,1,0,1,1,1,0,0,0},{0,0,1,1,0,0,1,1,1,1,1,1,0,0,0,0},{0,0,1,1,0,0,1,1,1,1,1,1,0,0,0,0},
    {0,0,1,1,0,0,1,1,1,1,1,0,0,0,0,0},{0,0,1,1,1,0,1,1,1,1,0,0,0,0,0,0},{0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0},{0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0},{0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

int main(void) {
    GameState state = STATE_START;
    int score;
    char score_text[16];
    volatile int *video_resolution = (int *)(PIXEL_BUF_CTRL_BASE + 0x8);
    volatile int *KEY_ptr = (int *)KEY_BASE;
    volatile int *rgb_status = (int *)(RGB_RESAMPLER_BASE);
    int db;
    short white, green, black;
    int dino_base_y = 150;
    int dino_y;
    int jump_velocity, is_jumping;
    int keys, i;
    int cactus_pos[MAX_CACTI], cactus_active[MAX_CACTI];
    int bird_pos_x, bird_pos_y, bird_active;
    int cloud_pos_x[MAX_CLOUDS], cloud_pos_y[MAX_CLOUDS];
    int cloud_active[MAX_CLOUDS];
    int next_cloud_timer;

    screen_x = *video_resolution & 0xFFFF;
    screen_y = (*video_resolution >> 16) & 0xFFFF;
    db = get_data_bits(*rgb_status & 0x3F);
    res_offset = (screen_x == 160) ? 1 : 0;
    col_offset = (db == 8) ? 1 : 0;
    white = resample_rgb(db, WHITE);
    green = resample_rgb(db, GREEN);
    black = resample_rgb(db, BLACK);

    while (1) {
        video_box(0, 0, STANDARD_X - 1, STANDARD_Y - 1, black);
        clear_char_buffer();
        
        keys = *KEY_ptr;

        switch(state) {
            case STATE_START:
                video_text(24, 29, "Pressione PULAR para iniciar");
                
                dino_y = dino_base_y;
                jump_velocity = 0;
                is_jumping = 0;
                score = 0;
                bird_active = 0;
                
                for (i = 0; i < MAX_CACTI; ++i) cactus_active[i] = 0;
                for (i = 0; i < MAX_CLOUDS; ++i) cloud_active[i] = 0; 
                next_cloud_timer = 0;

                if ((keys & 0x1) == 1) {
                    state = STATE_PLAY;
                    cactus_active[0] = 1;
                    cactus_pos[0] = STANDARD_X;
                }
                break;

            case STATE_PLAY:
                if (((keys & 0x1) == 1) && !is_jumping) {
                    is_jumping = 1; jump_velocity = -12;
                }
                if (is_jumping) {
                    dino_y += jump_velocity; jump_velocity += 1;
                    if (dino_y >= dino_base_y) { dino_y = dino_base_y; is_jumping = 0; }
                }

                /* ALTERADO: Lógica das nuvens para maior frequência */
                next_cloud_timer--;
                if (next_cloud_timer <= 0) {
                    for (i = 0; i < MAX_CLOUDS; ++i) {
                        if (!cloud_active[i]) {
                            cloud_active[i] = 1;
                            cloud_pos_x[i] = STANDARD_X;
                            cloud_pos_y[i] = 20 + (rand() % 50); 
                            next_cloud_timer = 40 + (rand() % 80); // Próxima nuvem em tempo menor
                            break;
                        }
                    }
                }
                for (i = 0; i < MAX_CLOUDS; ++i) {
                    if (cloud_active[i]) {
                        cloud_pos_x[i] -= 1;
                        if (cloud_pos_x[i] < -32) cloud_active[i] = 0;
                    }
                }

                if (score > 0 && score % 80 == 0) {
                    for (i = 0; i < MAX_CACTI; ++i) {
                        if (!cactus_active[i]) {
                            cactus_active[i] = 1; cactus_pos[i] = STANDARD_X + (rand() % 20); break; 
                        }
                    }
                }
                if (!bird_active && score > 200 && (rand() % 250) == 1) {
                    bird_active = 1; bird_pos_x = STANDARD_X; bird_pos_y = dino_base_y - 25;
                }
                if (bird_active) {
                    bird_pos_x -= 3;
                    if (bird_pos_x < -16) bird_active = 0;
                }
                for (i = 0; i < MAX_CACTI; ++i) {
                    if (cactus_active[i]) {
                        cactus_pos[i] -= 2;
                        if (cactus_pos[i] < -16) cactus_active[i] = 0;
                        if (cactus_pos[i] < 40 + 16 && cactus_pos[i] + 16 > 40 && dino_y + 16 > dino_base_y) {
                            state = STATE_GAMEOVER;
                        }
                    }
                }
                if (bird_active && bird_pos_x < 40 + 16 && bird_pos_x + 16 > 40 && bird_pos_y < dino_y + 16 && bird_pos_y + 16 > dino_y) {
                    state = STATE_GAMEOVER;
                }

                if (state != STATE_GAMEOVER) score++;

                sprintf(score_text, "Score: %04d", score);
                video_text(2, 2, score_text);
                break;

            case STATE_GAMEOVER:
                video_text(35, 29, "GAME OVER!");
                video_text(22, 32, "Pressione PULAR para reiniciar");
                if ((keys & 0x1) == 1) state = STATE_START;
                break;
        }

        /* --- Seção de Desenho --- */
        for (i = 0; i < MAX_CLOUDS; ++i) {
            if (cloud_active[i]) {
                draw_bitmap(cloud_pos_x[i], cloud_pos_y[i], cloud_bitmap, 16, 16, white);
            }
        }
        
        draw_ground(dino_base_y + 16, white);
        draw_bitmap(40, dino_y, dino_bitmap, 16, 16, white);
        
        for (i = 0; i < MAX_CACTI; ++i) {
            if (cactus_active[i]) draw_bitmap(cactus_pos[i], dino_base_y, cactus_bitmap, 16, 16, green);
        }
        
        if (bird_active) draw_bitmap(bird_pos_x, bird_pos_y, bird_bitmap, 16, 16, white);

        delay(3000);
    }
    return 0;
}
void clear_char_buffer() {
    volatile char *char_buffer = (char *)0xC9000000;
    int x, y;
    for (y = 0; y < 60; ++y) {
        for (x = 0; x < 80; ++x) {
            char_buffer[(y * 128) + x] = ' ';
        }
    }
}

void draw_ground(int y, short color) {
    video_box(0, y, STANDARD_X - 1, y, color);
}

void delay(int count) {
    volatile int i;
    for (i = 0; i < count; i++) {
        asm volatile("nop");
    }
}

void video_box(int x1, int y1, int x2, int y2, short pixel_color) {
    int pixel_buf_ptr = *(int *)PIXEL_BUF_CTRL_BASE;
    int pixel_ptr, row, col;
    int x_factor = 0x1 << (res_offset + col_offset);
    int y_factor = 0x1 << res_offset;
    x1 = x1 / x_factor; x2 = x2 / x_factor;
    y1 = y1 / y_factor; y2 = y2 / y_factor;

    for (row = y1; row <= y2; row++) {
        for (col = x1; col <= x2; col++) {
            pixel_ptr = pixel_buf_ptr + (row << (10 - res_offset - col_offset)) + (col << 1);
            *(short *)pixel_ptr = pixel_color;
        }
    }
}

int resample_rgb(int num_bits, int color) {
    if (num_bits == 8) {
        color = (((color >> 16) & 0xE0) | ((color >> 11) & 0x1C) | ((color >> 6) & 0x03));
        color = (color << 8) | color;
    } else if (num_bits == 16) {
        color = (((color >> 8) & 0xF800) | ((color >> 5) & 0x07E0) | ((color >> 3) & 0x001F));
    }
    return color;
}

int get_data_bits(int mode) {
    switch (mode) {
        case 0x0: return 1; case 0x7: return 8; case 0x11: return 8; case 0x12: return 9;
        case 0x14: return 16; case 0x17: return 24; case 0x19: return 30; case 0x31: return 8;
        case 0x32: return 12; case 0x33: return 16; case 0x37: return 32; case 0x39: return 40;
        default: return 16;
    }
}

void draw_bitmap(int x, int y, const unsigned char bitmap[16][16], int width, int height, short color) {
    int row, col;
    for (row = 0; row < height; row++) {
        for (col = 0; col < width; col++) {
            if (bitmap[row][col]) {
                video_box(x + col * 2, y + row * 2, x + col * 2 + 1, y + row * 2 + 1, color);
            }
        }
    }
}

void video_text(int x, int y, char *text) {
    volatile char *video_text_ptr = (char *)0xC9000000;
    int offset = (y * 128) + x;
    while (*text) {
        video_text_ptr[offset] = *text;
        offset++;
        text++;
    }
}
