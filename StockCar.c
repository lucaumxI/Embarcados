#include <reg51.h>
#include <intrins.h>

// --- DEFINIÇÕES GLOBAIS ---
#define Data_Port P1
#define largura 30
#define PLAYER_SPEED 2
#define GAME_SPEED_DIVIDER 8 // VELOCIDADE DO JOGO. AUMENTE PARA DESACELERAR.

// --- VARIÁVEIS GLOBAIS ---
unsigned char count_obstacle = 0;
signed char current_curve_direction = 0;
unsigned char current_curve_length = 0;
volatile unsigned char g_tick_counter = 0;

// --- PINOS DO HARDWARE ---
sbit RS  = P2^0;
sbit RW  = P2^1;
sbit E   = P2^2;
sbit CS1 = P2^4;
sbit CS2 = P2^3;
sbit RST = P2^5;
sbit LINHA2 = P0^2;
sbit COL1   = P0^4;
sbit COL3   = P0^6;

// --- PROTÓTIPOS DE FUNÇÕES ---
unsigned int lfsr_rand(void);
char scanKey(void);
void Update_Game(unsigned char*, unsigned char*, unsigned char*, unsigned char*, unsigned int*, unsigned char*, char);
void Draw_Screen(unsigned char*, unsigned char*, unsigned char*, unsigned char*, unsigned int*);

// --- FUNÇÕES DE TEMPO E INTERRUPÇÃO ---
void Timer0_Init(void) {
    TMOD = 0x01;
    TH0 = 0xB8;
    TL0 = 0x00;
    ET0 = 1;
    EA = 1;
    TR0 = 1;
}

void Timer0_ISR(void) interrupt 1 {
    TH0 = 0xB8;
    TL0 = 0x00;
    g_tick_counter++;
}

// --- FUNÇÕES DO GLCD ---
void GLCD_Command(char Command) {
    Data_Port = Command;
    RS = 0; RW = 0;
    E = 1;
    _nop_(); _nop_();
    E = 0;
}

void GLCD_Data(char Data) {
    Data_Port = Data;
    RS = 1; RW = 0;
    E = 1; _nop_(); E = 0;
}

void GLCD_SelectSide(unsigned char x) {
    if(x < 64) {
        CS1 = 1; CS2 = 0;
        GLCD_Command(0x40 + x);
    } else {
        CS1 = 0; CS2 = 1;
        GLCD_Command(0x40 + (x - 64));
    }
}

void GLCD_Init() {
    int i, j;
    CS1 = 1; CS2 = 1; RST = 1;
    for(i=0; i<200; i++) for(j=0; j<112; j++);
    GLCD_Command(0x3E); GLCD_Command(0x40); GLCD_Command(0xB8);
    GLCD_Command(0xC0); GLCD_Command(0x3F);
}

void GLCD_ClearAll() {
    int i, j;
    for(i = 0; i < 8; i++) {
        CS1 = 1; CS2 = 0;
        GLCD_Command(0xB8+i);
        GLCD_Command(0x40);
        for(j = 0; j < 64; j++) GLCD_Data(0);
        CS1 = 0; CS2 = 1;
        GLCD_Command(0xB8+i);
        GLCD_Command(0x40);
        for(j = 0; j < 64; j++) GLCD_Data(0);
    }
    GLCD_Command(0xB8); GLCD_Command(0x40);
    CS1 = 1; CS2 = 1;
}

void GLCD_Erase(unsigned char x, unsigned char page){
    GLCD_SelectSide(x);
    GLCD_Command(0xB8 + page);
    GLCD_Data(0);
}

void GLCD_Draw(unsigned char x, unsigned char y, unsigned char width, unsigned char height) {
    unsigned char pageStart = y / 8;
    unsigned char pageEnd = (y + height - 1) / 8;
    unsigned char bitMaskTop = 0xFF << (y % 8);
    unsigned char bitMaskBottom = 0xFF >> (7 - ((y + height - 1) % 8));
    unsigned char pixelData;
    unsigned char page, col;

    if (x >= 128 || y >= 64) return;
    if (x + width > 128) width = 128 - x;
    if (y + height > 64) height = 64 - y;

    for(page = pageStart; page <= pageEnd; page++) {
        GLCD_SelectSide(x);
        GLCD_Command(0xB8 + page);
        for(col = x; col < x + width; col++) {
            if (col == 64) {
                GLCD_SelectSide(col);
                GLCD_Command(0xB8 + page);
            }
            if(page == pageStart && page == pageEnd) { pixelData = bitMaskTop & bitMaskBottom; } 
            else if(page == pageStart) { pixelData = bitMaskTop; } 
            else if(page == pageEnd) { pixelData = bitMaskBottom; } 
            else { pixelData = 0xFF; }
            GLCD_Data(pixelData);
        }
    }
}

// --- FUNÇÕES DE DESENHO DO JOGO ---
void DrawCar6x8(unsigned char x, unsigned char page) {
    GLCD_SelectSide(x);   GLCD_Command(0xB8 + page); GLCD_Data(0xE7);
    GLCD_SelectSide(x+1); GLCD_Command(0xB8 + page); GLCD_Data(0x42);
    GLCD_SelectSide(x+4); GLCD_Command(0xB8 + page); GLCD_Data(0x42);
    GLCD_SelectSide(x+5); GLCD_Command(0xB8 + page); GLCD_Data(0xE7);
    GLCD_Draw(x+2, page*8, 2, 8);
}

void GLCD_Road(unsigned char x_road, unsigned char y_road){
    GLCD_Draw(x_road, y_road, 1, 8);
    GLCD_Draw(x_road + largura, y_road, 1, 8);
}

// --- FUNÇÕES DE TEXTO E PLACAR ---
unsigned char const code font5x7[10][5] = {
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, {0x00, 0x42, 0x7F, 0x40, 0x00},
    {0x42, 0x61, 0x51, 0x49, 0x46}, {0x21, 0x41, 0x45, 0x4B, 0x31},
    {0x18, 0x14, 0x12, 0x7F, 0x10}, {0x27, 0x45, 0x45, 0x45, 0x39},
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, {0x01, 0x71, 0x09, 0x05, 0x03},
    {0x36, 0x49, 0x49, 0x49, 0x36}, {0x06, 0x49, 0x49, 0x29, 0x1E}
};

void GLCD_WriteChar(unsigned char x, unsigned char page, char c) {
    unsigned char i, digit;
    digit = (c >= '0' && c <= '9') ? c - '0' : 0;
    GLCD_SelectSide(x);
    GLCD_Command(0xB8 + page);
    for(i = 0; i < 5; i++) GLCD_Data(font5x7[digit][i]);
    GLCD_Data(0x00);
}

void GLCD_WriteString(unsigned char x, unsigned char page, char *str) {
    while(*str) {
        GLCD_WriteChar(x, page, *str++);
        x += 6;
        if(x >= 123) break;
    }
}

void Get_StringNumber(unsigned int num, char* digits) {
    digits[0] = (num/1000)%10 + '0';
    digits[1] = (num/100)%10 + '0';
    digits[2] = (num/10)%10 + '0';
    digits[3] = num%10 + '0';
    digits[4] = '\0';
}

// --- ENTRADA DO JOGADOR ---
char scanKey() {
    LINHA2 = 0;
    if(COL1 == 0) return '6';
    if(COL3 == 0) return '4';
    LINHA2 = 1;
    return 0;
}

// --- FUNÇÃO DE ALEATORIEDADE ---
unsigned int lfsr_rand(void) {
    static unsigned int lfsr_state = 12345;
    unsigned char lsb = lfsr_state & 1;
    lfsr_state >>= 1;
    if (lsb == 1) lfsr_state ^= 0xB400u;
    return lfsr_state;
}

// --- LÓGICA DO JOGO ---
char Check_Collision(unsigned char x, unsigned char x_car, unsigned char x_obstacle){
    unsigned char car_right = x_car + 6;
    if(x_car <= x || car_right > x + largura) return 1;
    if(x_obstacle == 0) return 0;
    if(car_right-1 < x_obstacle || x_car > x_obstacle + 5) return 0;
    return 1;
}

void Update_Game(unsigned char *x_road, unsigned char *curva, unsigned char *obstacle, unsigned char *x_car, unsigned int *score, unsigned char *accident, char key) {
    unsigned char i;
    unsigned int random_value;

    if(key == '4' && *x_car > 0){ *x_car -= PLAYER_SPEED; }
    if(key == '6' && *x_car < 128 - 6){ *x_car += PLAYER_SPEED; }

    for(i = 7; i > 0; i--){
        x_road[i] = x_road[i-1];
        curva[i] = curva[i-1];
        obstacle[i] = obstacle[i-1];
    }

    count_obstacle++;
    if(!curva[0] && count_obstacle >= 15){
        obstacle[0] = x_road[0] + (lfsr_rand() % (largura - 6));
        count_obstacle = 0;
    } else {
        obstacle[0] = 0;
    }

    if (current_curve_length > 0) {
        curva[0] = current_curve_direction;
        x_road[0] += current_curve_direction;
        current_curve_length--;
    } else {
        curva[0] = 0;
        random_value = lfsr_rand();
        if (random_value % 25 == 1) {
            current_curve_length = 10 + (lfsr_rand() % 15);
            current_curve_direction = (lfsr_rand() % 2 == 0) ? -1 : 1;
            if ((current_curve_direction == -1 && x_road[0] < 25) ||
                (current_curve_direction == 1 && x_road[0] > 55)) {
                current_curve_length = 0;
                current_curve_direction = 0;
            }
        }
    }

    if(Check_Collision(x_road[7], *x_car, obstacle[7]))
        (*accident) = 1;
    else
        (*score)++;
}

void Draw_Screen(unsigned char *x_road, unsigned char *curva, unsigned char *obstacle, unsigned char *x_car, unsigned int *score){
    char stringscore[5];
    unsigned char i, j;

    Get_StringNumber(*score, stringscore);
    GLCD_WriteString(92, 0, stringscore);

    for(i = 1; i < largura; i++){
        if((x_road[7] + i < *x_car) || (x_road[7] + i >= *x_car + 6))
            GLCD_Erase(x_road[7] + i, 7);
    }

    for(i = 0; i < 8; i++){
        if(obstacle[i] > 0 && i > 0){
            for (j = 0; j < 6; j++) {
                GLCD_Erase(obstacle[i] + j, i - 1);
            }
        }
        if(curva[i] == 1){
            GLCD_Erase(x_road[i] - 1, i);
            GLCD_Erase(x_road[i] + largura - 1, i);
        } else if(curva[i] == -1){
            GLCD_Erase(x_road[i] + 1, i);
            GLCD_Erase(x_road[i] + largura + 1, i);
        }
        if(obstacle[i] > 0) DrawCar6x8(obstacle[i], i);
        GLCD_Road(x_road[i], i*8);
    }

    DrawCar6x8(*x_car, 7);
}

// --- FUNÇÃO PRINCIPAL ---
void main() {
    unsigned char x_car;
    unsigned int score;
    unsigned char accident;
    unsigned char x_road[8], curva[8], obstacle[8];
    unsigned char i;
    char last_key_pressed;

    GLCD_Init();
    Timer0_Init();

    while(1) {
        x_car = 61;
        score = 0;
        accident = 0;
        count_obstacle = 0;
        current_curve_direction = 0;
        current_curve_length = 0;
        g_tick_counter = 0;

        for(i = 0; i < 8; i++){
            x_road[i] = 49;
            curva[i] = 0;
            obstacle[i] = 0;
        }

        GLCD_ClearAll();

        while(!accident){
            if(g_tick_counter >= GAME_SPEED_DIVIDER) {
                g_tick_counter = 0;
                last_key_pressed = scanKey();
                Update_Game(x_road, curva, obstacle, &x_car, &score, &accident, last_key_pressed);
                Draw_Screen(x_road, curva, obstacle, &x_car, &score);
            }
        }

        {
            long k;
            for(k=0; k < 50000; k++);
        }
        while(scanKey() != 0);
        while(scanKey() == 0);
    }
}
