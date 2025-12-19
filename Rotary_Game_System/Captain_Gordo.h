#ifndef CAPTAIN_GORDO_H
#define CAPTAIN_GORDO_H

#include <Arduino.h>
#include <TFT_eSPI.h>

#define SCREEN_W 320
#define SCREEN_H 240
#define STAR_COUNT 28
#define MAX_BULLETS 3
#define MAX_ENEMIES 8
#define MAX_ENEMY_BULLETS 8
#define MAX_LIVES 3
#define BOSS_MAX_HEALTH 10
#define MAX_BOSS_BULLETS 12

#define COLOR_CARROT    0xFDA0
#define COLOR_CARROT_TOP 0x03A0
#define COLOR_JET1      TFT_ORANGE
#define COLOR_JET2      TFT_YELLOW
#define COLOR_BULLET    TFT_ORANGE
#define COLOR_BAT       TFT_BLUE
#define COLOR_TROGLO    TFT_PURPLE
#define COLOR_TROGLO_SHOT 0xC9B7 // light purple
#define COLOR_SCORE     TFT_ORANGE
#define COLOR_WALL      TFT_WHITE
#define COLOR_WALL2     TFT_LIGHTGREY
#define COLOR_WALL_VERT TFT_GREEN
#define COLOR_STAR1     TFT_PINK
#define COLOR_STAR2     TFT_RED
#define COLOR_STAR3     TFT_BLUE
#define COLOR_STAR4     TFT_CYAN
#define COLOR_BG        TFT_BLACK

const int CARROT_SHIP_W = 38;
const int CARROT_SHIP_H = 14;
const int SHIP_W = 54;
const int SHIP_H = 28;
#define CARROT_Y_MIN 25
#define CARROT_Y_MAX (SCREEN_H-25)


struct Star {
    float x, y, speed;
    uint16_t color;
};
Star stars[STAR_COUNT];

struct Bullet {
    float x, y, vx, vy;
    bool active;
};
Bullet bullets[MAX_BULLETS];

struct Enemy {
    float x, y;
    int type; // 0 = bat, 1 = troglobite
    bool active;
    int frame;
    int fireTimer;
};
Enemy enemies[MAX_ENEMIES];

struct EnemyBullet {
    float x, y, vx, vy;
    bool active;
};
EnemyBullet enemyBullets[MAX_ENEMY_BULLETS];

// --- Vector Font (with H, E, S, etc) ---
struct CG_VecStroke { int x0, y0, x1, y1; };
struct CG_VecLetter { const CG_VecStroke* strokes; int n; int w; };
const CG_VecStroke cg_strokes_H[] = {{0,0,0,12},{8,0,8,12},{0,6,8,6}};
const CG_VecStroke cg_strokes_E[] = {{0,0,0,12},{0,0,8,0},{0,6,7,6},{0,12,8,12}};
const CG_VecStroke cg_strokes_S[] = {{8,0,0,0},{0,0,0,6},{0,6,8,6},{8,6,8,12},{8,12,0,12}};
const CG_VecStroke cg_strokes_C[] = {{8,0,0,0},{0,0,0,12},{0,12,8,12}};
const CG_VecStroke cg_strokes_A[] = {{0,12,4,0},{4,0,8,12},{2,6,6,6}};
const CG_VecStroke cg_strokes_P[] = {{0,12,0,0},{0,0,8,0},{8,0,8,6},{8,6,0,6}};
const CG_VecStroke cg_strokes_T[] = {{4,0,4,12},{0,0,8,0}};
const CG_VecStroke cg_strokes_I[] = {{4,0,4,12}};
const CG_VecStroke cg_strokes_N[] = {{0,12,0,0},{0,0,8,12},{8,12,8,0}};
const CG_VecStroke cg_strokes_G[] = {
    {8,0,0,0},
    {0,0,0,12},
    {0,12,8,12},
    {8,12,8,8},
    {8,8,5,8}
};
const CG_VecStroke cg_strokes_O[] = {{0,0,8,0},{8,0,8,12},{8,12,0,12},{0,12,0,0}};
const CG_VecStroke cg_strokes_R[] = {{0,12,0,0},{0,0,8,0},{8,0,8,6},{8,6,0,6},{0,6,8,12}};
const CG_VecStroke cg_strokes_D[] = {{0,12,0,0},{0,0,7,2},{7,2,8,6},{8,6,7,10},{7,10,0,12}};
const CG_VecStroke cg_strokes_M[] = {{0,12,0,0},{0,0,4,6},{4,6,8,0},{8,0,8,12}};
const CG_VecStroke cg_strokes_V[] = {{0,0,4,12},{4,12,8,0}};
const CG_VecStroke cg_strokes_L[] = {{0,0,0,12},{0,12,8,12}};
const CG_VecStroke cg_strokes_2[] = {{0,0,8,0},{8,0,8,6},{8,6,0,6},{0,6,0,12},{0,12,8,12}};
const CG_VecStroke cg_strokes_B[] = {{0,12,0,0},{0,0,7,0},{7,0,8,2},{8,2,7,4},{7,4,0,6},{0,6,7,8},{7,8,8,10},{8,10,7,12},{7,12,0,12}};
const CG_VecStroke cg_strokes_1[] = {{2,0,4,0},{4,0,4,12},{1,12,7,12}};
const CG_VecStroke cg_strokes_3[] = {{0,0,8,0},{8,0,8,6},{8,6,4,6},{8,6,8,12},{8,12,0,12}};
const CG_VecStroke cg_strokes_4[] = {{0,0,0,6},{0,6,8,6},{8,0,8,12}};
const CG_VecStroke cg_strokes_5[] = {{8,0,0,0},{0,0,0,6},{0,6,8,6},{8,6,8,12},{8,12,0,12}};
const CG_VecStroke cg_strokes_6[] = {{8,0,0,0},{0,0,0,12},{0,12,8,12},{8,12,8,6},{8,6,0,6}};
const CG_VecStroke cg_strokes_7[] = {{0,0,8,0},{8,0,4,12}};
const CG_VecStroke cg_strokes_8[] = {{0,0,8,0},{8,0,8,6},{8,6,0,6},{0,6,0,0},{0,6,0,12},{0,12,8,12},{8,12,8,6}};
const CG_VecStroke cg_strokes_9[] = {{8,12,8,0},{8,0,0,0},{0,0,0,6},{0,6,8,6}};
const CG_VecStroke cg_strokes_0[] = {{0,0,8,0},{8,0,8,12},{8,12,0,12},{0,12,0,0}};
const CG_VecLetter cgVecFont[] = {
    {cg_strokes_C, sizeof(cg_strokes_C)/sizeof(CG_VecStroke), 9}, // 0
    {cg_strokes_A, sizeof(cg_strokes_A)/sizeof(CG_VecStroke), 9},
    {cg_strokes_P, sizeof(cg_strokes_P)/sizeof(CG_VecStroke), 9},
    {cg_strokes_T, sizeof(cg_strokes_T)/sizeof(CG_VecStroke), 9},
    {cg_strokes_I, sizeof(cg_strokes_I)/sizeof(CG_VecStroke), 9},
    {cg_strokes_N, sizeof(cg_strokes_N)/sizeof(CG_VecStroke), 9},
    {cg_strokes_G, sizeof(cg_strokes_G)/sizeof(CG_VecStroke), 9},
    {cg_strokes_O, sizeof(cg_strokes_O)/sizeof(CG_VecStroke), 9},
    {cg_strokes_R, sizeof(cg_strokes_R)/sizeof(CG_VecStroke), 9},
    {cg_strokes_D, sizeof(cg_strokes_D)/sizeof(CG_VecStroke), 9},
    {cg_strokes_H, sizeof(cg_strokes_H)/sizeof(CG_VecStroke), 9},
    {cg_strokes_E, sizeof(cg_strokes_E)/sizeof(CG_VecStroke), 9}, 
    {cg_strokes_S, sizeof(cg_strokes_S)/sizeof(CG_VecStroke), 9},
    {cg_strokes_M, sizeof(cg_strokes_M)/sizeof(CG_VecStroke), 9},
    {cg_strokes_V, sizeof(cg_strokes_V)/sizeof(CG_VecStroke), 9},
    {cg_strokes_L, sizeof(cg_strokes_L)/sizeof(CG_VecStroke), 9},
    {cg_strokes_2, sizeof(cg_strokes_2)/sizeof(CG_VecStroke), 9},
    {cg_strokes_1, sizeof(cg_strokes_1)/sizeof(CG_VecStroke), 9},
    {cg_strokes_3, sizeof(cg_strokes_3)/sizeof(CG_VecStroke), 9},
    {cg_strokes_4, sizeof(cg_strokes_4)/sizeof(CG_VecStroke), 9},
    {cg_strokes_B, sizeof(cg_strokes_B)/sizeof(CG_VecStroke), 9},
    {cg_strokes_5, sizeof(cg_strokes_5)/sizeof(CG_VecStroke), 9},
    {cg_strokes_6, sizeof(cg_strokes_6)/sizeof(CG_VecStroke), 9},
    {cg_strokes_7, sizeof(cg_strokes_7)/sizeof(CG_VecStroke), 9},
    {cg_strokes_8, sizeof(cg_strokes_8)/sizeof(CG_VecStroke), 9},
    {cg_strokes_9, sizeof(cg_strokes_9)/sizeof(CG_VecStroke), 9},
    {cg_strokes_0, sizeof(cg_strokes_0)/sizeof(CG_VecStroke), 9}
};
int cgGetVecFontIdx(char c) {
    switch(c) {
        case 'C': return 0; case 'A': return 1; case 'P': return 2; case 'T': return 3;
        case 'I': return 4; case 'N': return 5; case 'G': return 6; case 'O': return 7;
        case 'R': return 8; case 'D': return 9; case 'H': return 10; case 'E': return 11; 
        case 'S': return 12; case 'M': return 13; case 'V': return 14; case 'L': return 15;
        case '2': return 16; case '1': return 17; case '3': return 18; case '4': return 19;
        case 'B': return 20;
        case '5': return 21; case '6': return 22; case '7': return 23; case '8': return 24;
        case '9': return 25; case '0': return 26;
        default: return -1;
    }
}
void cgDrawVecLetter(TFT_eSPI &tft, char c, int x, int y, int scale, uint16_t color) {
    int idx = cgGetVecFontIdx(c);
    if(idx < 0) return;
    for (int i = 0; i < cgVecFont[idx].n; i++) {
        int x0 = x + cgVecFont[idx].strokes[i].x0 * scale;
        int y0 = y + cgVecFont[idx].strokes[i].y0 * scale;
        int x1 = x + cgVecFont[idx].strokes[i].x1 * scale;
        int y1 = y + cgVecFont[idx].strokes[i].y1 * scale;
        tft.drawLine(x0, y0, x1, y1, color);
    }
}
void cgDrawVecText(TFT_eSPI &tft, const char* text, int x, int y, int scale, uint16_t color) {
    int spacing = 9*scale;
    int i = 0;
    while (text[i]) {
        if (text[i] != ' ') cgDrawVecLetter(tft, text[i], x + i * spacing, y, scale, color);
        i++;
    }
}
int cgVecTextWidth(const char* text, int scale) {
    int len = strlen(text);
    return len * 9 * scale;
}
void cg_drawVecQuestionMark(TFT_eSPI &tft, int x, int y, int scale, uint16_t color) {
    tft.drawLine(x + 0*scale, y + 2*scale, x + 2*scale, y + 0*scale, color);
    tft.drawLine(x + 2*scale, y + 0*scale, x + 6*scale, y + 0*scale, color);
    tft.drawLine(x + 6*scale, y + 0*scale, x + 8*scale, y + 2*scale, color);
    tft.drawLine(x + 8*scale, y + 2*scale, x + 8*scale, y + 4*scale, color);
    tft.drawLine(x + 8*scale, y + 4*scale, x + 4*scale, y + 8*scale, color);
    tft.drawLine(x + 4*scale, y + 8*scale, x + 4*scale, y + 10*scale, color);
    tft.drawLine(x + 4*scale, y + 12*scale, x + 4*scale, y + 13*scale, color);
}

// -------- Starfield and Drawing Functions --------
void cg_initStarfield(int mode = 0) {
    // mode 0 = white (splash/cutscenes), mode 1 = red/pink (level 1), mode 2 = cyan/blue (level 2), mode 3 = orange/yellow (level 3)
    for (int i = 0; i < STAR_COUNT; i++) {
        stars[i].x = random(0, SCREEN_W);
        stars[i].y = random(0, SCREEN_H);
        
        if (mode == 0) {
            stars[i].color = TFT_WHITE;
        } else if (mode == 1) {
            if (stars[i].y < SCREEN_H/2)
                stars[i].color = COLOR_STAR1;  // Pink
            else
                stars[i].color = COLOR_STAR2;  // Red
        } else if (mode == 2) {
            if (stars[i].y < SCREEN_H/2)
                stars[i].color = COLOR_STAR4;  // Cyan
            else
                stars[i].color = COLOR_STAR3;  // Blue
        } else if (mode == 3) {
            // Orange and yellow for level 3
            if (stars[i].y < SCREEN_H/2)
                stars[i].color = TFT_ORANGE;
            else
                stars[i].color = TFT_YELLOW;
        }
        stars[i].speed = 1.2 + 1.8 * (float)random(0, 100)/100.0;
    }
}

void cg_eraseStarfield(TFT_eSPI &tft) {
    for (int i = 0; i < STAR_COUNT; i++)
        tft.drawPixel((int)stars[i].x, (int)stars[i].y, COLOR_BG);
}

void cg_moveStarfield(bool reverse = false, bool vertical = false) {
    for (int i = 0; i < STAR_COUNT; i++) {
        if (vertical) {
            // FIXED: Move top to bottom for level 2
            stars[i].y += stars[i].speed;
            if (stars[i].y >= SCREEN_H) {
                stars[i].y = 0;
                stars[i].x = random(0, SCREEN_W);
            }
        } else if (reverse) {
            // Move right to left for level 3
            stars[i].x += stars[i].speed;
            if (stars[i].x >= SCREEN_W) {
                stars[i].x = 0;
                stars[i].y = random(0, SCREEN_H);
            }
        } else {
            // Normal left to right
            stars[i].x -= stars[i].speed;
            if (stars[i].x < 0) {
                stars[i].x = SCREEN_W-1;
                stars[i].y = random(0, SCREEN_H);
            }
        }
    }
}
void cg_drawStarfield(TFT_eSPI &tft) {
    for (int i = 0; i < STAR_COUNT; i++)
        tft.drawPixel((int)stars[i].x, (int)stars[i].y, stars[i].color);
}
void cg_drawVectorCarrot(TFT_eSPI &tft, int x, int y, bool erase=false) {
    uint16_t carrot = erase ? COLOR_BG : COLOR_CARROT;
    uint16_t green = erase ? COLOR_BG : COLOR_CARROT_TOP;
    uint16_t jet1 = erase ? COLOR_BG : COLOR_JET1;
    uint16_t jet2 = erase ? COLOR_BG : COLOR_JET2;
    uint16_t cockpit = erase ? COLOR_BG : TFT_CYAN;
    tft.drawLine(x, y, x+CARROT_SHIP_W, y, carrot);
    tft.drawLine(x, y-4, x+CARROT_SHIP_W-4, y-4, carrot);
    tft.drawLine(x, y+4, x+CARROT_SHIP_W-4, y+4, carrot);
    tft.drawLine(x+CARROT_SHIP_W, y, x+CARROT_SHIP_W-4, y-4, carrot);
    tft.drawLine(x+CARROT_SHIP_W, y, x+CARROT_SHIP_W-4, y+4, carrot);
    tft.drawLine(x, y, x-6, y-6, green);
    tft.drawLine(x, y, x-7, y, green);
    tft.drawLine(x, y, x-6, y+6, green);
    tft.drawLine(x-8, y, x-18, y, jet1);
    tft.drawLine(x-8, y-2, x-16, y-4, jet2);
    tft.drawLine(x-8, y+2, x-16, y+4, jet2);
    tft.drawLine(x+CARROT_SHIP_W-8, y-3, x+CARROT_SHIP_W-2, y-3, cockpit);
    tft.drawLine(x+CARROT_SHIP_W-8, y-2, x+CARROT_SHIP_W-2, y-2, cockpit);
    tft.drawLine(x+CARROT_SHIP_W-8, y-1, x+CARROT_SHIP_W-2, y-1, cockpit);
}

// NEW: Vertical carrot ship (pointing up)
void cg_drawVectorCarrotVertical(TFT_eSPI &tft, int x, int y, bool erase=false) {
    uint16_t carrot = erase ? COLOR_BG : COLOR_CARROT;
    uint16_t green = erase ? COLOR_BG : COLOR_CARROT_TOP;
    uint16_t jet1 = erase ? COLOR_BG : COLOR_JET1;
    uint16_t jet2 = erase ? COLOR_BG : COLOR_JET2;
    uint16_t cockpit = erase ? COLOR_BG : TFT_CYAN;
    
    // Main body (vertical)
    tft.drawLine(x, y, x, y-CARROT_SHIP_W, carrot);
    tft.drawLine(x-4, y, x-4, y-CARROT_SHIP_W+4, carrot);
    tft.drawLine(x+4, y, x+4, y-CARROT_SHIP_W+4, carrot);
    tft.drawLine(x, y-CARROT_SHIP_W, x-4, y-CARROT_SHIP_W+4, carrot);
    tft.drawLine(x, y-CARROT_SHIP_W, x+4, y-CARROT_SHIP_W+4, carrot);
    
    // Green top
    tft.drawLine(x, y-CARROT_SHIP_W, x-6, y-CARROT_SHIP_W-6, green);
    tft.drawLine(x, y-CARROT_SHIP_W, x, y-CARROT_SHIP_W-7, green);
    tft.drawLine(x, y-CARROT_SHIP_W, x+6, y-CARROT_SHIP_W-6, green);
    
    // Jets (pointing down)
    tft.drawLine(x, y+8, x, y+18, jet1);
    tft.drawLine(x-2, y+8, x-4, y+16, jet2);
    tft.drawLine(x+2, y+8, x+4, y+16, jet2);
    
    // Cockpit
    tft.drawLine(x-3, y-CARROT_SHIP_W+8, x-3, y-CARROT_SHIP_W+2, cockpit);
    tft.drawLine(x-2, y-CARROT_SHIP_W+8, x-2, y-CARROT_SHIP_W+2, cockpit);
    tft.drawLine(x-1, y-CARROT_SHIP_W+8, x-1, y-CARROT_SHIP_W+2, cockpit);
}

// NEW: Carrot ship facing LEFT (for level 3)
void cg_drawVectorCarrotLeft(TFT_eSPI &tft, int x, int y, bool erase=false) {
    uint16_t carrot = erase ? COLOR_BG : COLOR_CARROT;
    uint16_t green = erase ? COLOR_BG : COLOR_CARROT_TOP;
    uint16_t jet1 = erase ? COLOR_BG : COLOR_JET1;
    uint16_t jet2 = erase ? COLOR_BG : COLOR_JET2;
    uint16_t cockpit = erase ? COLOR_BG : TFT_CYAN;
    
    // Main body (horizontal, pointing left)
    tft.drawLine(x, y, x-CARROT_SHIP_W, y, carrot);
    tft.drawLine(x, y-4, x-CARROT_SHIP_W+4, y-4, carrot);
    tft.drawLine(x, y+4, x-CARROT_SHIP_W+4, y+4, carrot);
    tft.drawLine(x-CARROT_SHIP_W, y, x-CARROT_SHIP_W+4, y-4, carrot);
    tft.drawLine(x-CARROT_SHIP_W, y, x-CARROT_SHIP_W+4, y+4, carrot);
    
    // Green top (pointing left)
    tft.drawLine(x-CARROT_SHIP_W, y, x-CARROT_SHIP_W-6, y-6, green);
    tft.drawLine(x-CARROT_SHIP_W, y, x-CARROT_SHIP_W-7, y, green);
    tft.drawLine(x-CARROT_SHIP_W, y, x-CARROT_SHIP_W-6, y+6, green);
    
    // Jets (pointing right, since ship faces left)
    tft.drawLine(x+8, y, x+18, y, jet1);
    tft.drawLine(x+8, y-2, x+16, y-4, jet2);
    tft.drawLine(x+8, y+2, x+16, y+4, jet2);
    
    // Cockpit
    tft.drawLine(x-CARROT_SHIP_W+8, y-3, x-CARROT_SHIP_W+2, y-3, cockpit);
    tft.drawLine(x-CARROT_SHIP_W+8, y-2, x-CARROT_SHIP_W+2, y-2, cockpit);
    tft.drawLine(x-CARROT_SHIP_W+8, y-1, x-CARROT_SHIP_W+2, y-1, cockpit);
}

// NEW: Carrot ship facing DOWN (for boss level)
void cg_drawVectorCarrotDown(TFT_eSPI &tft, int x, int y, bool erase=false) {
    uint16_t carrot = erase ? COLOR_BG : COLOR_CARROT;
    uint16_t green = erase ? COLOR_BG : COLOR_CARROT_TOP;
    uint16_t jet1 = erase ? COLOR_BG : COLOR_JET1;
    uint16_t jet2 = erase ? COLOR_BG : COLOR_JET2;
    uint16_t cockpit = erase ? COLOR_BG : TFT_CYAN;
    
    // Main body (vertical, pointing down)
    tft.drawLine(x, y, x, y+CARROT_SHIP_W, carrot);
    tft.drawLine(x-4, y, x-4, y+CARROT_SHIP_W-4, carrot);
    tft.drawLine(x+4, y, x+4, y+CARROT_SHIP_W-4, carrot);
    tft.drawLine(x, y+CARROT_SHIP_W, x-4, y+CARROT_SHIP_W-4, carrot);
    tft.drawLine(x, y+CARROT_SHIP_W, x+4, y+CARROT_SHIP_W-4, carrot);
    
    // Green top
    tft.drawLine(x, y+CARROT_SHIP_W, x-6, y+CARROT_SHIP_W+6, green);
    tft.drawLine(x, y+CARROT_SHIP_W, x, y+CARROT_SHIP_W+7, green);
    tft.drawLine(x, y+CARROT_SHIP_W, x+6, y+CARROT_SHIP_W+6, green);
    
    // Jets (pointing up)
    tft.drawLine(x, y-8, x, y-18, jet1);
    tft.drawLine(x-2, y-8, x-4, y-16, jet2);
    tft.drawLine(x+2, y-8, x+4, y-16, jet2);
    
    // Cockpit
    tft.drawLine(x-3, y+CARROT_SHIP_W-8, x-3, y+CARROT_SHIP_W-2, cockpit);
    tft.drawLine(x-2, y+CARROT_SHIP_W-8, x-2, y+CARROT_SHIP_W-2, cockpit);
    tft.drawLine(x-1, y+CARROT_SHIP_W-8, x-1, y+CARROT_SHIP_W-2, cockpit);
}

// NEW: Bullet facing LEFT (for level 3)
void cg_drawLaserBoltLeft(TFT_eSPI &tft, int x, int y, bool erase=false) {
    uint16_t c1 = erase ? COLOR_BG : COLOR_BULLET;
    uint16_t c2 = erase ? COLOR_BG : TFT_YELLOW;
    tft.drawLine(x, y-1, x-14, y, c2);
    tft.drawLine(x, y, x-14, y+1, c1);
    tft.drawLine(x, y+1, x-14, y+2, c2);
}

// NEW: Bullet facing DOWN (for boss level)
void cg_drawLaserBoltDown(TFT_eSPI &tft, int x, int y, bool erase=false) {
    uint16_t c1 = erase ? COLOR_BG : COLOR_BULLET;
    uint16_t c2 = erase ? COLOR_BG : TFT_YELLOW;
    tft.drawLine(x-1, y, x, y+14, c2);
    tft.drawLine(x, y, x+1, y+14, c1);
    tft.drawLine(x+1, y, x+2, y+14, c2);
}

// -------- Boss Level Functions --------

// Draw cave floor for boss level (vector style like walls)
void cg_drawBossFloor(TFT_eSPI &tft, bool skipHatch = false) {
    static int floorPoints[SCREEN_W / 8];
    static bool floorInitialized = false;
    
    int floorY = SCREEN_H - 15;
    
    if (!floorInitialized) {
        for (int i = 0; i < SCREEN_W / 8; i++) {
            floorPoints[i] = floorY + random(-4, 5);
            if (floorPoints[i] > SCREEN_H - 10) floorPoints[i] = SCREEN_H - 10;
            if (floorPoints[i] < floorY - 5) floorPoints[i] = floorY - 5;
        }
        floorInitialized = true;
    }
    
    // Draw jagged floor line (skip hatch area if open)
    int hatchWidth = SCREEN_W / 4;
    int hatchStartX = (SCREEN_W - hatchWidth) / 2;
    int hatchEndX = hatchStartX + hatchWidth;
    
    for (int i = 0; i < (SCREEN_W / 8) - 1; i++) {
        int x1 = i * 8;
        int x2 = (i + 1) * 8;
        
        // Skip drawing floor segments in the hatch area if skipHatch is true
        if (skipHatch && x1 >= hatchStartX && x2 <= hatchEndX) {
            continue;
        }
        
        tft.drawLine(x1, floorPoints[i], x2, floorPoints[i+1], TFT_YELLOW);
    }
}

// Draw the hatch door at bottom center (grey, 25% of floor width)
void cg_drawHatch(TFT_eSPI &tft, bool open=false) {
    int hatchWidth = SCREEN_W / 4;  // 25% of screen width
    int hatchX = (SCREEN_W - hatchWidth) / 2;
    int hatchY = SCREEN_H - 14;  // FIXED: At the very bottom
    
    if (open) {
        // Blown open hatch - jagged edges
        uint16_t darkGray = TFT_DARKGREY;
        tft.drawLine(hatchX, hatchY, hatchX+8, hatchY-6, darkGray);
        tft.drawLine(hatchX+8, hatchY-6, hatchX+15, hatchY-4, darkGray);
        tft.drawLine(hatchX+hatchWidth, hatchY, hatchX+hatchWidth-8, hatchY-6, darkGray);
        tft.drawLine(hatchX+hatchWidth-8, hatchY-6, hatchX+hatchWidth-15, hatchY-4, darkGray);
    } else {
        // Closed hatch (grey)
        tft.fillRect(hatchX, hatchY, hatchWidth, 8, TFT_DARKGREY);
        tft.drawRect(hatchX, hatchY, hatchWidth, 8, TFT_LIGHTGREY);
        
        // Hatch panels
        for (int i = 1; i < 4; i++) {
            tft.drawLine(hatchX + i * (hatchWidth/4), hatchY, 
                        hatchX + i * (hatchWidth/4), hatchY + 8, TFT_LIGHTGREY);
        }
        
        // Center seam
        tft.drawLine(hatchX + hatchWidth/2, hatchY, 
                    hatchX + hatchWidth/2, hatchY + 8, TFT_BLACK);
    }
}

// Draw boss explosion effect
void cg_drawBossExplosion(TFT_eSPI &tft, int x, int y, int frame) {
    // Multiple expanding circles with colors
    for (int i = 0; i < 5; i++) {
        int radius = frame * 3 + i * 8;
        uint16_t color = (i % 2 == 0) ? TFT_RED : TFT_ORANGE;
        if (radius < 60) {
            tft.drawCircle(x, y, radius, color);
            tft.drawCircle(x, y, radius-1, color);
        }
    }
    
    // Radiating lines
    for (int i = 0; i < 12; i++) {
        float angle = (i * 30 + frame * 10) * 0.01745;
        int length = frame * 4;
        if (length < 80) {
            int x1 = x + cos(angle) * 10;
            int y1 = y + sin(angle) * 10;
            int x2 = x + cos(angle) * length;
            int y2 = y + sin(angle) * length;
            uint16_t color = (i % 3 == 0) ? TFT_YELLOW : TFT_ORANGE;
            tft.drawLine(x1, y1, x2, y2, color);
        }
    }
    
    // Random sparks
    for (int i = 0; i < 15; i++) {
        int sx = x + random(-frame*2, frame*2);
        int sy = y + random(-frame*2, frame*2);
        tft.drawPixel(sx, sy, (random(0,2) == 0) ? TFT_YELLOW : TFT_WHITE);
    }
}

// Boss level splash screen
void cg_showBossSplash(TFT_eSPI &tft) {
    tft.fillScreen(COLOR_BG);
    
    int scale = 3;
    int textW = cgVecTextWidth("BOSS", scale);
    int textX = (SCREEN_W - textW) / 2;
    cgDrawVecText(tft, "BOSS", textX, 70, scale, TFT_RED);  // FIXED: Moved up from 80
    
    textW = cgVecTextWidth("LEVEL", scale);
    textX = (SCREEN_W - textW) / 2;
    cgDrawVecText(tft, "LEVEL", textX, 120, scale, TFT_RED);  // FIXED: Moved up from 130
    
    delay(2000);
    tft.fillScreen(COLOR_BG);
}

void cg_drawLaserBolt(TFT_eSPI &tft, int x, int y, bool erase=false) {
    uint16_t c1 = erase ? COLOR_BG : COLOR_BULLET;
    uint16_t c2 = erase ? COLOR_BG : TFT_YELLOW;
    // FIXED: Made smaller - reduced from 22 pixels to 14 pixels
    tft.drawLine(x, y-1, x+14, y, c2);
    tft.drawLine(x, y, x+14, y+1, c1);
    tft.drawLine(x, y+1, x+14, y+2, c2);
}

// NEW: Vertical laser bolt (pointing up)
void cg_drawLaserBoltVertical(TFT_eSPI &tft, int x, int y, bool erase=false) {
    uint16_t c1 = erase ? COLOR_BG : COLOR_BULLET;
    uint16_t c2 = erase ? COLOR_BG : TFT_YELLOW;
    // FIXED: Made smaller - reduced from 22 pixels to 14 pixels
    tft.drawLine(x-1, y, x, y-14, c2);
    tft.drawLine(x, y, x+1, y-14, c1);
    tft.drawLine(x+1, y, x+2, y-14, c2);
}

void cg_drawVectorShip(TFT_eSPI &tft, int x, int y, bool erase=false) {
    uint16_t white = erase ? COLOR_BG : TFT_WHITE;
    uint16_t blue = erase ? COLOR_BG : TFT_BLUE;
    uint16_t yellow = erase ? COLOR_BG : TFT_YELLOW;
    uint16_t jet = erase ? COLOR_BG : TFT_CYAN;
    int scale = 2;

    tft.drawLine(x, y, x+SHIP_W*scale, y, white);
    tft.drawLine(x+SHIP_W*scale, y, x+SHIP_W*scale-16, y-16, white);
    tft.drawLine(x+SHIP_W*scale, y, x+SHIP_W*scale-16, y+16, white);
    tft.drawLine(x, y, x+16, y-24, white);
    tft.drawLine(x, y, x+16, y+24, white);
    tft.drawLine(x+16, y-24, x+SHIP_W*scale-24, y-16, white);
    tft.drawLine(x+16, y+24, x+SHIP_W*scale-24, y+16, white);

    tft.drawLine(x+32, y-24, x+40, y-44, white);
    tft.drawLine(x+32, y+24, x+40, y+44, white);
    tft.drawLine(x+40, y-44, x+SHIP_W*scale-40, y-20, white);
    tft.drawLine(x+40, y+44, x+SHIP_W*scale-40, y+20, white);

    tft.drawLine(x+40, y-44, x+40, y-52, blue);
    tft.drawLine(x+40, y+44, x+40, y+52, blue);

    tft.drawLine(x+40, y-52, x+45, y-54, yellow);
    tft.drawLine(x+40, y+52, x+45, y+54, yellow);

    tft.drawLine(x, y, x-12, y-16, white);
    tft.drawLine(x, y, x-12, y+16, white);
    tft.drawLine(x-12, y-16, x-20, y-8, white);
    tft.drawLine(x-12, y+16, x-20, y+8, white);

    tft.drawLine(x+SHIP_W*scale-48, y-6, x+SHIP_W*scale-20, y-6, blue);
    tft.drawLine(x+SHIP_W*scale-48, y+6, x+SHIP_W*scale-20, y+6, blue);
    tft.drawLine(x+SHIP_W*scale-48, y-6, x+SHIP_W*scale-48, y+6, yellow);
    tft.drawLine(x+SHIP_W*scale-20, y-6, x+SHIP_W*scale-20, y+6, yellow);

    tft.drawLine(x-22, y-8, x-40, y-14, jet);
    tft.drawLine(x-22, y+8, x-40, y+14, jet);
    tft.drawLine(x-16, y, x-32, y, blue);
    tft.drawLine(x-16, y-4, x-32, y-10, blue);
    tft.drawLine(x-16, y+4, x-32, y+10, blue);
}

void cg_drawVectorAsteroid(TFT_eSPI &tft, int x, int y, int r, bool erase=false) {
    uint16_t color = erase ? COLOR_BG : TFT_LIGHTGREY;
    int pts = 16;
    float angles[16] = {0, 0.23, 0.45, 0.7, 1.0, 1.4, 1.7, 2.0, 2.6, 2.9, 3.4, 3.8, 4.1, 4.4, 4.7, 5.3};
    int rx[16], ry[16];
    for (int i=0; i<pts; i++) {
        float a = angles[i];
        float rr = r * (0.85 + 0.2 * (i%3));
        rx[i] = x + (int)(cos(a) * rr);
        ry[i] = y + (int)(sin(a) * rr);
    }
    for (int i=0; i<pts; i++) {
        int j = (i+1)%pts;
        tft.drawLine(rx[i], ry[i], rx[j], ry[j], color);
    }
    uint16_t craterCol = erase ? COLOR_BG : TFT_DARKGREY;
    tft.drawCircle(x+18, y+2, r/3, craterCol);
    tft.drawCircle(x-8, y-11, r/7, craterCol);
    tft.drawCircle(x+5, y+12, r/9, craterCol);
    tft.drawCircle(x-12, y+8, r/11, craterCol);
}

void cg_drawBubbleWithVectorText(TFT_eSPI &tft, int x, int y, const char* line1, bool erase, uint16_t textColor, const char* line2 = nullptr) {
    uint16_t bubbleCol = erase ? TFT_BLACK : TFT_WHITE;
    tft.drawEllipse(x, y, 110, 36, bubbleCol);
    tft.drawCircle(x-80, y+28, 12, bubbleCol);
    tft.drawCircle(x-103, y+50, 7, bubbleCol);
    tft.drawCircle(x-120, y+67, 4, bubbleCol);

    int scale = 1;
    int msgw1 = cgVecTextWidth(line1, scale);
    int msgx1 = x - msgw1/2;
    int msgy1 = y - 9;
    if (!erase)
        cgDrawVecText(tft, line1, msgx1, msgy1, scale, textColor);
    else
        cgDrawVecText(tft, line1, msgx1, msgy1, scale, TFT_BLACK);

    if (line2 && line2[0]) {
        int msgw2 = cgVecTextWidth(line2, scale);
        int msgx2 = x - msgw2/2;
        int msgy2 = y + 11;
        if (!erase)
            cgDrawVecText(tft, line2, msgx2, msgy2, scale, textColor);
        else
            cgDrawVecText(tft, line2, msgx2, msgy2, scale, TFT_BLACK);
    }
}

void captainGordoSplash(TFT_eSPI &tft) {
    tft.fillScreen(COLOR_BG);
    cg_initStarfield(0);  // White stars for splash

    const char* splash1 = "CAPTAIN";
    const char* splash2 = "GORDO";
    int title_scale = 5;
    int w1 = cgVecTextWidth(splash1, title_scale);
    int w2 = cgVecTextWidth(splash2, title_scale);
    int x1 = (SCREEN_W - w1) / 2 - 2;
    int x2 = (SCREEN_W - w2) / 2 - 3;
    int y1 = 24;
    int y2 = 106;
    int carrot_x = (SCREEN_W - CARROT_SHIP_W) / 2;
    int carrot_y = y2 + title_scale*13 + 14 + 8;

    while (digitalRead(PIN_KO) == LOW) delay(10);
    int lastBtn = HIGH;
    bool buttonPressed = false;
    while (!buttonPressed) {
        cg_eraseStarfield(tft);
        cg_moveStarfield();
        cg_drawStarfield(tft);

        cgDrawVecText(tft, splash1, x1, y1, title_scale, TFT_WHITE);
        cgDrawVecText(tft, splash2, x2, y2, title_scale, TFT_WHITE);

        cg_drawVectorCarrot(tft, carrot_x, carrot_y, false);

        if (((millis()/400)%2) == 0) {
            tft.setTextColor(TFT_RED, COLOR_BG);
            tft.setTextFont(2);
            tft.setTextDatum(TC_DATUM);
            tft.drawString("Press button to start", SCREEN_W/2, SCREEN_H-20);
        }

        int btn = digitalRead(PIN_KO);
        if (btn == LOW && lastBtn == HIGH) {
            buttonPressed = true;
            while (digitalRead(PIN_KO) == LOW) delay(10);
        }
        lastBtn = btn;

        delay(22);
    }
    tft.fillScreen(COLOR_BG);
}


// -------- Jagged Wall Drawing (updated for level 3 yellow walls) --------

// Global wall arrays (need to be accessible for collision detection)
int wall_top[SCREEN_W], wall_bot[SCREEN_W];
int wall_left[SCREEN_H], wall_right[SCREEN_H];
bool wallsInitialized = false;

// FIXED: Add arrays to track previous wall positions for erasing
int prev_wall_top[SCREEN_W], prev_wall_bot[SCREEN_W];
int prev_wall_left[SCREEN_H], prev_wall_right[SCREEN_H];

void cg_drawCaveWalls(TFT_eSPI &tft, bool vertical, bool flipped, uint16_t wallColor = COLOR_WALL) {
    static int wallOffset = 0;

    if (!vertical) {
        // Horizontal cave - walls on top and bottom
        if (!wallsInitialized) {
            int y0 = 50, y1 = SCREEN_H-30;
            for (int x=0; x<SCREEN_W; x+=4) {
                int dy0 = random(-6, 7);
                int dy1 = random(-6, 7);
                wall_top[x] = y0 + dy0;
                wall_bot[x] = y1 + dy1;
                if (wall_top[x] < 45) wall_top[x] = 45;
                if (wall_bot[x] > SCREEN_H-20) wall_bot[x] = SCREEN_H-20;
                
                prev_wall_top[x] = wall_top[x];
                prev_wall_bot[x] = wall_bot[x];
            }
            wallsInitialized = true;
            wallOffset = 0;
        }
        
        // Erase old wall positions
        for (int x=0; x<SCREEN_W-4; x+=4) {
            tft.drawLine(x, prev_wall_top[x], x+4, prev_wall_top[x+4], COLOR_BG);
            tft.drawLine(x, prev_wall_bot[x], x+4, prev_wall_bot[x+4], COLOR_BG);
        }
        
        // Store current positions as previous
        for (int x=0; x<SCREEN_W; x+=4) {
            prev_wall_top[x] = wall_top[x];
            prev_wall_bot[x] = wall_bot[x];
        }
        
        // Scroll walls (left for levels 1-2, right for level 3)
        if (flipped) {
            // Scroll RIGHT for level 3
            for (int x=SCREEN_W-4; x>0; x-=4) {
                wall_top[x] = wall_top[x-4];
                wall_bot[x] = wall_bot[x-4];
            }
            
            // Generate new wall segment at LEFT edge
            int dy0 = random(-6, 7);
            int dy1 = random(-6, 7);
            wall_top[0] = wall_top[4] + dy0;
            wall_bot[0] = wall_bot[4] + dy1;
            if (wall_top[0] < 45) wall_top[0] = 45;
            if (wall_top[0] > 70) wall_top[0] = 70;
            if (wall_bot[0] > SCREEN_H-20) wall_bot[0] = SCREEN_H-20;
            if (wall_bot[0] < SCREEN_H-50) wall_bot[0] = SCREEN_H-50;
        } else {
            // Scroll LEFT for levels 1-2
            for (int x=0; x<SCREEN_W-4; x+=4) {
                wall_top[x] = wall_top[x+4];
                wall_bot[x] = wall_bot[x+4];
            }
            
            // Generate new wall segment at right edge
            int dy0 = random(-6, 7);
            int dy1 = random(-6, 7);
            wall_top[SCREEN_W-4] = wall_top[SCREEN_W-8] + dy0;
            wall_bot[SCREEN_W-4] = wall_bot[SCREEN_W-8] + dy1;
            if (wall_top[SCREEN_W-4] < 45) wall_top[SCREEN_W-4] = 45;
            if (wall_top[SCREEN_W-4] > 70) wall_top[SCREEN_W-4] = 70;
            if (wall_bot[SCREEN_W-4] > SCREEN_H-20) wall_bot[SCREEN_W-4] = SCREEN_H-20;
            if (wall_bot[SCREEN_W-4] < SCREEN_H-50) wall_bot[SCREEN_W-4] = SCREEN_H-50;
        }
        
        // Draw top/bottom walls at new positions with specified color
        for (int x=0; x<SCREEN_W-4; x+=4) {
            tft.drawLine(x, wall_top[x], x+4, wall_top[x+4], wallColor);
            tft.drawLine(x, wall_bot[x], x+4, wall_bot[x+4], wallColor);
        }
    } else {
        // Vertical cave - walls on left and right (level 2)
        if (!wallsInitialized) {
            int x0 = 50, x1 = SCREEN_W-50;
            for (int y=0; y<SCREEN_H; y+=4) {
                int dx0 = random(-6, 7);
                int dx1 = random(-6, 7);
                wall_left[y] = x0 + dx0;
                wall_right[y] = x1 + dx1;
                if (wall_left[y] < 40) wall_left[y] = 40;
                if (wall_right[y] > SCREEN_W-40) wall_right[y] = SCREEN_W-40;
                
                prev_wall_left[y] = wall_left[y];
                prev_wall_right[y] = wall_right[y];
            }
            wallsInitialized = true;
        }
        
        // Erase old wall positions
        for (int y=0; y<SCREEN_H-4; y+=4) {
            tft.drawLine(prev_wall_left[y], y, prev_wall_left[y+4], y+4, COLOR_BG);
            tft.drawLine(prev_wall_right[y], y, prev_wall_right[y+4], y+4, COLOR_BG);
        }
        
        // Store current positions as previous
        for (int y=0; y<SCREEN_H; y+=4) {
            prev_wall_left[y] = wall_left[y];
            prev_wall_right[y] = wall_right[y];
        }
        
        // Scroll walls DOWN
        for (int y=SCREEN_H-4; y>0; y-=4) {
            wall_left[y] = wall_left[y-4];
            wall_right[y] = wall_right[y-4];
        }
        
        // Generate new wall segment at TOP edge
        int dx0 = random(-6, 7);
        int dx1 = random(-6, 7);
        wall_left[0] = wall_left[4] + dx0;
        wall_right[0] = wall_right[4] + dx1;
        if (wall_left[0] < 40) wall_left[0] = 40;
        if (wall_left[0] > 70) wall_left[0] = 70;
        if (wall_right[0] > SCREEN_W-40) wall_right[0] = SCREEN_W-40;
        if (wall_right[0] < SCREEN_W-70) wall_right[0] = SCREEN_W-70;
        
        // Draw left/right walls at new positions
        for (int y=0; y<SCREEN_H-4; y+=4) {
            tft.drawLine(wall_left[y], y, wall_left[y+4], y+4, wallColor);
            tft.drawLine(wall_right[y], y, wall_right[y+4], y+4, wallColor);
        }
    }
}

// Helper function to check if a point is inside the cave (not hitting walls)
bool cg_checkWallCollision(int x, int y, bool vertical) {
    if (!vertical) {
        // Horizontal mode - check top and bottom walls
        if (x < 0 || x >= SCREEN_W) return false;
        int wallIdx = (x / 4) * 4;
        if (wallIdx >= SCREEN_W-4) wallIdx = SCREEN_W-4;
        
        // Interpolate wall position
        float frac = (x % 4) / 4.0f;
        int topY = wall_top[wallIdx] + (wall_top[wallIdx+4] - wall_top[wallIdx]) * frac;
        int botY = wall_bot[wallIdx] + (wall_bot[wallIdx+4] - wall_bot[wallIdx]) * frac;
        
        if (y < topY + 8 || y > botY - 8) return true;  // Hit wall
    } else {
        // Vertical mode - check left and right walls
        if (y < 0 || y >= SCREEN_H) return false;
        int wallIdx = (y / 4) * 4;
        if (wallIdx >= SCREEN_H-4) wallIdx = SCREEN_H-4;
        
        // Interpolate wall position
        float frac = (y % 4) / 4.0f;
        int leftX = wall_left[wallIdx] + (wall_left[wallIdx+4] - wall_left[wallIdx]) * frac;
        int rightX = wall_right[wallIdx] + (wall_right[wallIdx+4] - wall_right[wallIdx]) * frac;
        
        if (x < leftX + 8 || x > rightX - 8) return true;  // Hit wall
    }
    return false;
}

// -------- Mini Carrot for Lives Display --------
void cg_drawMiniCarrot(TFT_eSPI &tft, int x, int y, bool erase=false) {
    uint16_t carrot = erase ? COLOR_BG : COLOR_CARROT;
    uint16_t green = erase ? COLOR_BG : COLOR_CARROT_TOP;
    tft.drawLine(x, y, x+12, y, carrot);
    tft.drawLine(x, y-2, x+10, y-2, carrot);
    tft.drawLine(x, y+2, x+10, y+2, carrot);
    tft.drawLine(x+12, y, x+10, y-2, carrot);
    tft.drawLine(x+12, y, x+10, y+2, carrot);
    tft.drawLine(x, y, x-3, y-3, green);
    tft.drawLine(x, y, x-3, y+3, green);
}


// -------- Bat Enemy --------
void cg_drawBat(TFT_eSPI &tft, int x, int y, bool erase=false, uint16_t color=COLOR_BAT) {
    uint16_t batColor = erase ? COLOR_BG : color;
    // Body
    tft.drawLine(x, y, x-4, y+6, batColor);
    tft.drawLine(x, y, x+4, y+6, batColor);
    tft.drawLine(x-4, y+6, x+4, y+6, batColor);
    // Left wing
    tft.drawLine(x-4, y+6, x-12, y+2, batColor);
    tft.drawLine(x-12, y+2, x-8, y+10, batColor);
    // Right wing
    tft.drawLine(x+4, y+6, x+12, y+2, batColor);
    tft.drawLine(x+12, y+2, x+8, y+10, batColor);
    // Ears
    tft.drawLine(x-2, y, x-2, y-4, batColor);
    tft.drawLine(x+2, y, x+2, y-4, batColor);
}

// -------- Troglobite Enemy --------
void cg_drawTroglobite(TFT_eSPI &tft, int x, int y, bool erase=false, bool onCeiling=false, uint16_t color=COLOR_TROGLO) {
    uint16_t purple = erase ? COLOR_BG : color;
    uint16_t red = erase ? COLOR_BG : TFT_RED;
    
    if (!onCeiling) {
        // On floor - blob shape
        tft.drawLine(x-8, y, x-6, y-6, purple);
        tft.drawLine(x-6, y-6, x-2, y-8, purple);
        tft.drawLine(x-2, y-8, x+2, y-8, purple);
        tft.drawLine(x+2, y-8, x+6, y-6, purple);
        tft.drawLine(x+6, y-6, x+8, y, purple);
        tft.drawLine(x+8, y, x-8, y, purple);
        
        // Bottom bulges
        tft.drawLine(x-8, y, x-10, y+3, purple);
        tft.drawLine(x-10, y+3, x-8, y+4, purple);
        tft.drawLine(x+8, y, x+10, y+3, purple);
        tft.drawLine(x+10, y+3, x+8, y+4, purple);
        
        // Red spots
        tft.drawCircle(x-3, y-4, 2, red);
        tft.drawCircle(x+3, y-4, 2, red);
        tft.drawCircle(x, y-2, 1, red);
    } else {
        // On ceiling - hanging blob (flipped)
        tft.drawLine(x-8, y, x-6, y+6, purple);
        tft.drawLine(x-6, y+6, x-2, y+8, purple);
        tft.drawLine(x-2, y+8, x+2, y+8, purple);
        tft.drawLine(x+2, y+8, x+6, y+6, purple);
        tft.drawLine(x+6, y+6, x+8, y, purple);
        tft.drawLine(x+8, y, x-8, y, purple);
        
        // Top bulges
        tft.drawLine(x-8, y, x-10, y-3, purple);
        tft.drawLine(x-10, y-3, x-8, y-4, purple);
        tft.drawLine(x+8, y, x+10, y-3, purple);
        tft.drawLine(x+10, y-3, x+8, y-4, purple);
        
        // Red spots
        tft.drawCircle(x-3, y+4, 2, red);
        tft.drawCircle(x+3, y+4, 2, red);
        tft.drawCircle(x, y+2, 1, red);
    }
}


void cg_drawTroglobiteShot(TFT_eSPI &tft, int x, int y, bool erase=false) {
    uint16_t color = erase ? COLOR_BG : TFT_WHITE;  // FIXED: Changed to white
    // FIXED: Made larger
    tft.drawLine(x-3, y, x+3, y, color);
    tft.drawLine(x, y-3, x, y+3, color);
    tft.drawLine(x-2, y-2, x+2, y+2, color);
    tft.drawLine(x-2, y+2, x+2, y-2, color);
}

// -------- Game Over --------
void cg_drawGameOver(TFT_eSPI &tft) {
    tft.fillScreen(COLOR_BG);
    cgDrawVecText(tft, "GAME", 66, 50, 5, TFT_RED);  // FIXED: Moved up from 80 to 50
    cgDrawVecText(tft, "OVER", 66, 110, 5, TFT_RED); // FIXED: Moved up from 140 to 110
}

// --------- Intro Chase Scene (carrot chased by alien ship, lasers) ---------
void run_CG_ChaseIntro(TFT_eSPI &tft) {
    cg_initStarfield(0);  // White stars for cutscene
    tft.fillScreen(COLOR_BG);

    float carrotX = -CARROT_SHIP_W - 24;
    float carrotY = SCREEN_H / 2;
    float alienX = -SHIP_W * 2 - 100;
    float alienY = carrotY - 30;
    bool showAlien = false;
    bool fireLasers = false;
    int shotsFired = 0;
    int shotDelay = 0;
    
    // FIXED: Warning shot laser
    float warningLaserX = -30;
    float warningLaserY = carrotY - 40;
    bool warningLaserActive = false;
    
    // FIXED: 1 laser from top wing, 2 lasers from bottom wing
    struct Laser { float x, y; bool active; int lifetime; };
    Laser lasers[3] = {
        {-40, carrotY-30, false, 0},  // Top wing
        {-40, carrotY+30, false, 0},  // Bottom wing 1
        {-40, carrotY+35, false, 0}   // Bottom wing 2
    };

    int frame = 0;
    
    // Phase 1: Carrot enters alone until 1/4 across, then warning shot
    while (carrotX < SCREEN_W/2 && !showAlien) {
        // FIXED: Check for button press to skip
        if (digitalRead(PIN_PUSH) == LOW) {
            while (digitalRead(PIN_PUSH) == LOW) delay(10);
            tft.fillScreen(COLOR_BG);
            return;
        }
        
        tft.fillScreen(COLOR_BG);
        cg_drawStarfield(tft);

        carrotX += 4.2f;
        cg_drawVectorCarrot(tft, (int)carrotX, (int)carrotY, false);
        
        // FIXED: Fire warning shot when carrot is 1/4 across screen
        if (carrotX > SCREEN_W/4 && !warningLaserActive) {
            warningLaserActive = true;
            warningLaserX = -30;
            warningLaserY = carrotY - 40;
        }
        
        // Draw and move warning laser
        if (warningLaserActive) {
            warningLaserX += 12.0f;
            
            // Draw blue laser
            uint16_t blue = TFT_BLUE;
            uint16_t cyan = TFT_CYAN;
            int lx = (int)warningLaserX;
            int ly = (int)warningLaserY;
            tft.drawLine(lx, ly-2, lx+22, ly, cyan);
            tft.drawLine(lx, ly, lx+22, ly+2, blue);
            tft.drawLine(lx, ly+2, lx+22, ly+4, cyan);
            
            // Remove warning laser after it passes
            if (warningLaserX > SCREEN_W + 30) {
                warningLaserActive = false;
            }
        }

        delay(28);
        cg_moveStarfield();
        frame++;
    }
    
    // Phase 2: Carrot reaches halfway, alien appears
    showAlien = true;
    alienX = -SHIP_W*2-100;
    alienY = carrotY - 30;
    
    // Phase 3: Both move, alien catches up and shoots TWICE
    while (alienX < SCREEN_W + SHIP_W*2 + 100) {
        // FIXED: Check for button press to skip
        if (digitalRead(PIN_PUSH) == LOW) {
            while (digitalRead(PIN_PUSH) == LOW) delay(10);
            tft.fillScreen(COLOR_BG);
            return;
        }
        
        tft.fillScreen(COLOR_BG);
        cg_drawStarfield(tft);

        // Carrot keeps moving
        if (carrotX < SCREEN_W + 40) {
            carrotX += 4.2f;
            cg_drawVectorCarrot(tft, (int)carrotX, (int)carrotY, false);
        }

        // Alien moves faster to catch up
        alienX += 5.5f;
cg_drawVectorShip(tft, (int)alienX, (int)alienY, false);

        // Start firing when alien is on screen
        if (!fireLasers && alienX > 20) {
            fireLasers = true;
            shotDelay = 0;
        }
        
        if (fireLasers && shotsFired < 2) {
            shotDelay++;
            
            // Fire all lasers together (1 from top wing, 2 from bottom wing)
            if (shotDelay == 10) {
                for (int i=0; i<3; i++) {
                    lasers[i].active = true;
                    lasers[i].x = alienX + SHIP_W*2 - 10;
                    if (i == 0) {
                        // Top wing laser
                        lasers[i].y = alienY - 35;
                    } else {
                        // Bottom wing lasers
                        lasers[i].y = alienY + 40 + (i-1)*5;
                    }
                    lasers[i].lifetime = 0;
                }
            }
            
            // Move and draw active lasers
            for (int i=0; i<3; i++) {
                if (lasers[i].active) {
                    lasers[i].x += 12.0f;
                    lasers[i].lifetime++;
                    
                    // Draw blue lasers
                    uint16_t blue = TFT_BLUE;
                    uint16_t cyan = TFT_CYAN;
                    int lx = (int)lasers[i].x;
                    int ly = (int)lasers[i].y;
                    tft.drawLine(lx, ly-2, lx+22, ly, cyan);
                    tft.drawLine(lx, ly, lx+22, ly+2, blue);
                    tft.drawLine(lx, ly+2, lx+22, ly+4, cyan);
                    
                    // Remove laser after traveling across screen
                    if (lasers[i].x > SCREEN_W + 30) {
                        lasers[i].active = false;
                    }
                }
            }
            
            // Check if all lasers finished
            bool allInactive = true;
            for (int i=0; i<3; i++) {
                if (lasers[i].active) allInactive = false;
            }
            
            if (shotDelay > 60 && allInactive) {
                shotsFired++;
                shotDelay = 0;
            }
        }

        delay(28);
        cg_moveStarfield();
        frame++;
    }
    
    delay(300);
    tft.fillScreen(COLOR_BG);
}

// -------- Asteroid Cutscene --------
void run_CG_AsteroidCutscene(TFT_eSPI &tft) {
    tft.fillScreen(COLOR_BG);
    cg_initStarfield(0);  // White stars for cutscene

    int asteroidX = SCREEN_W-30, asteroidY = SCREEN_H/2, asteroidR = 54;
    int carrotY = SCREEN_H/2, carrotStartX = -CARROT_SHIP_W-24, carrotEndX = asteroidX-asteroidR+8;
    int carrotX = carrotStartX;
    int carrotPauseX = 60;

    while (carrotX < carrotPauseX) {
        // FIXED: Check for button press to skip
        if (digitalRead(PIN_PUSH) == LOW) {
            while (digitalRead(PIN_PUSH) == LOW) delay(10);
            tft.fillScreen(COLOR_BG);
            return;
        }
        
        cg_eraseStarfield(tft); cg_moveStarfield(); cg_drawStarfield(tft);
        cg_drawVectorAsteroid(tft, asteroidX, asteroidY, asteroidR, false);
        cg_drawVectorCarrot(tft, carrotX, carrotY, false);
        delay(40);
        cg_drawVectorCarrot(tft, carrotX, carrotY, true);
        carrotX += 2;
    }
    cg_drawVectorCarrot(tft, carrotX, carrotY, false);
    cg_drawVectorAsteroid(tft, asteroidX, asteroidY, asteroidR, false);

    int bubble_x = carrotX+130;
    int bubble_y = 50;
    cg_drawBubbleWithVectorText(tft, bubble_x, bubble_y, "I CAN HIDE IN THAT", false, TFT_ORANGE, "ASTEROID.");
    
    // FIXED: Allow skip during pause
    for (int i = 0; i < 30; i++) {
        if (digitalRead(PIN_PUSH) == LOW) {
            while (digitalRead(PIN_PUSH) == LOW) delay(10);
            tft.fillScreen(COLOR_BG);
            return;
        }
        delay(100);
    }
    
    cg_drawBubbleWithVectorText(tft, bubble_x, bubble_y, "I CAN HIDE IN THAT", true, TFT_ORANGE, "ASTEROID.");

    int steps = 36;
    for (int i=0; i<steps; i++) {
        // FIXED: Check for button press to skip
        if (digitalRead(PIN_PUSH) == LOW) {
            while (digitalRead(PIN_PUSH) == LOW) delay(10);
            tft.fillScreen(COLOR_BG);
            return;
        }
        
        cg_eraseStarfield(tft); cg_moveStarfield(); cg_drawStarfield(tft);
        cg_drawVectorAsteroid(tft, asteroidX, asteroidY, asteroidR, false);
        float f = (float)i/(steps-1);
        float s = 1.0f - 0.63f*f;
        int cx = carrotX + (int)((carrotEndX-carrotX)*f);
        int cy = carrotY + (int)((asteroidY-carrotY)*f*0.18);
        int x = cx, y = cy;
        int w = (int)(CARROT_SHIP_W*s), h = (int)(CARROT_SHIP_H*s);
        tft.drawLine(x, y, x+w, y, COLOR_CARROT);
        tft.drawLine(x, y-h/3, x+w-4, y-h/3, COLOR_CARROT);
        tft.drawLine(x, y+h/3, x+w-4, y+h/3, COLOR_CARROT);
        tft.drawLine(x+w, y, x+w-4, y-h/3, COLOR_CARROT);
        tft.drawLine(x+w, y, x+w-4, y+h/3, COLOR_CARROT);
        tft.drawLine(x, y, x-6*s, y-6*s, COLOR_CARROT_TOP);
        tft.drawLine(x, y, x-7*s, y, COLOR_CARROT_TOP);
        tft.drawLine(x, y, x-6*s, y+6*s, COLOR_CARROT_TOP);
        tft.drawLine(x-8*s, y, x-18*s, y, COLOR_JET1);
        tft.drawLine(x-8*s, y-2*s, x-16*s, y-4*s, COLOR_JET2);
        tft.drawLine(x-8*s, y+2*s, x-16*s, y+4*s, COLOR_JET2);
        tft.drawLine(x+w-8, y-3, x+w-2, y-3, TFT_CYAN);
        tft.drawLine(x+w-8, y-2, x+w-2, y-2, TFT_CYAN);
        tft.drawLine(x+w-8, y-1, x+w-2, y-1, TFT_CYAN);
        delay(28);
        tft.drawLine(x, y, x+w, y, COLOR_BG);
        tft.drawLine(x, y-h/3, x+w-4, y-h/3, COLOR_BG);
        tft.drawLine(x, y+h/3, x+w-4, y+h/3, COLOR_BG);
        tft.drawLine(x+w, y, x+w-4, y-h/3, COLOR_BG);
        tft.drawLine(x+w, y, x+w-4, y+h/3, COLOR_BG);
        tft.drawLine(x, y, x-6*s, y-6*s, COLOR_BG);
        tft.drawLine(x, y, x-7*s, y, COLOR_BG);
        tft.drawLine(x, y, x-6*s, y+6*s, COLOR_BG);
        tft.drawLine(x-8*s, y, x-18*s, y, COLOR_BG);
        tft.drawLine(x-8*s, y-2*s, x-16*s, y-4*s, COLOR_BG);
        tft.drawLine(x-8*s, y+2*s, x-16*s, y+4*s, COLOR_BG);
        tft.drawLine(x+w-8, y-3, x+w-2, y-3, COLOR_BG);
        tft.drawLine(x+w-8, y-2, x+w-2, y-2, COLOR_BG);
        tft.drawLine(x+w-8, y-1, x+w-2, y-1, COLOR_BG);
    }
    
    // FIXED: Allow skip during pause
    for (int i = 0; i < 10; i++) {
        if (digitalRead(PIN_PUSH) == LOW) {
            while (digitalRead(PIN_PUSH) == LOW) delay(10);
            tft.fillScreen(COLOR_BG);
            return;
        }
        delay(100);
    }

    int shipX = -SHIP_W*2-24, shipY = SCREEN_H/2-2;
    int targetX = 0;
    while (shipX < targetX) {
        // FIXED: Check for button press to skip
        if (digitalRead(PIN_PUSH) == LOW) {
            while (digitalRead(PIN_PUSH) == LOW) delay(10);
            tft.fillScreen(COLOR_BG);
            return;
        }
        
        cg_eraseStarfield(tft); cg_moveStarfield(); cg_drawStarfield(tft);
        cg_drawVectorAsteroid(tft, asteroidX, asteroidY, asteroidR, false);
cg_drawVectorShip(tft, shipX, shipY, false);        
delay(44);
cg_drawVectorShip(tft, shipX, shipY, true);
        shipX += 3;
    }
    cg_drawVectorAsteroid(tft, asteroidX, asteroidY, asteroidR, false);
cg_drawVectorShip(tft, shipX, shipY, false);
    int shipTipX = shipX + SHIP_W*2;
    int abubble_x = shipTipX+44;
    int abubble_y = 50;
    cg_drawBubbleWithVectorText(tft, abubble_x, abubble_y, "", false, TFT_WHITE, "");
    int qx = abubble_x - 14;
    for (int i = 0; i < 3; i++, qx += 13) {
        cg_drawVecQuestionMark(tft, qx, abubble_y-9, 1, TFT_WHITE);
    }
    
    // FIXED: Allow skip during pause
    for (int i = 0; i < 10; i++) {
        if (digitalRead(PIN_PUSH) == LOW) {
            while (digitalRead(PIN_PUSH) == LOW) delay(10);
            tft.fillScreen(COLOR_BG);
            return;
        }
        delay(100);
    }
    
    cg_drawBubbleWithVectorText(tft, abubble_x, abubble_y, "", true, TFT_WHITE, "");
    qx = abubble_x - 14;
    for (int i = 0; i < 3; i++, qx += 13) {
        cg_drawVecQuestionMark(tft, qx, abubble_y-9, 1, COLOR_BG);
    }

    for (int i=0; i<10; i++) {
        // FIXED: Check for button press to skip
        if (digitalRead(PIN_PUSH) == LOW) {
            while (digitalRead(PIN_PUSH) == LOW) delay(10);
            tft.fillScreen(COLOR_BG);
            return;
        }
        
        tft.fillRect(0,0,SCREEN_W,SCREEN_H,COLOR_BG);
        delay(45);
    }
    tft.fillScreen(COLOR_BG);
}

// Draw mini shield icon at top center
void cg_drawMiniShield(TFT_eSPI &tft, int x, int y, bool available) {
    uint16_t color = available ? TFT_WHITE : TFT_DARKGREY;
    // Draw shield shape
    tft.drawEllipse(x, y, 8, 10, color);
    tft.drawEllipse(x, y, 7, 9, color);
    // Draw shield cross pattern
    tft.drawLine(x-6, y-6, x+6, y+6, color);
    tft.drawLine(x-6, y+6, x+6, y-6, color);
}

// Draw shield around ship
void cg_drawShield(TFT_eSPI &tft, int x, int y, bool vertical, bool erase=false, bool bossLevel=false) {
    if (erase) {
        // FIXED: Larger erase area to catch all rotating energy lines
        if (!vertical && !bossLevel) {
            tft.fillRect(x - 20, y - 30, CARROT_SHIP_W + 50, 60, COLOR_BG);
        } else if (bossLevel) {
            // Boss level - even larger area for small ship pointing down
            tft.fillRect(x - 35, y - 35, 70, CARROT_SHIP_W + 50, COLOR_BG);
        } else {
            // Vertical mode (level 2)
            tft.fillRect(x - 30, y - CARROT_SHIP_W - 20, 60, CARROT_SHIP_W + 50, COLOR_BG);
        }
        return;
    }
    
    uint16_t color = TFT_WHITE;
    
    if (!vertical && !bossLevel) {
        // Horizontal mode - oval around horizontal ship
        tft.drawEllipse(x + CARROT_SHIP_W/2, y, CARROT_SHIP_W + 10, 20, color);
        tft.drawEllipse(x + CARROT_SHIP_W/2, y, CARROT_SHIP_W + 9, 19, color);
        // Add energy lines
        for (int i = 0; i < 6; i++) {
            float angle = (millis() / 100.0 + i * 60) * 0.01745;
            int x1 = x + CARROT_SHIP_W/2 + cos(angle) * (CARROT_SHIP_W + 8);
            int y1 = y + sin(angle) * 18;
            int x2 = x + CARROT_SHIP_W/2 + cos(angle) * (CARROT_SHIP_W + 12);
            int y2 = y + sin(angle) * 22;
            tft.drawLine(x1, y1, x2, y2, color);
        }
    } else if (bossLevel) {
        // Boss level - smaller ship pointing down
        int shipW = CARROT_SHIP_W / 2;
        tft.drawEllipse(x, y + shipW/2, 15, shipW + 8, color);
        tft.drawEllipse(x, y + shipW/2, 14, shipW + 7, color);
        // Add energy lines
        for (int i = 0; i < 6; i++) {
            float angle = (millis() / 100.0 + i * 60) * 0.01745;
            int x1 = x + cos(angle) * 13;
            int y1 = y + shipW/2 + sin(angle) * (shipW + 6);
            int x2 = x + cos(angle) * 17;
            int y2 = y + shipW/2 + sin(angle) * (shipW + 10);
            tft.drawLine(x1, y1, x2, y2, color);
        }
    } else {
        // Vertical mode - oval around vertical ship
        tft.drawEllipse(x, y - CARROT_SHIP_W/2, 20, CARROT_SHIP_W + 10, color);
        tft.drawEllipse(x, y - CARROT_SHIP_W/2, 19, CARROT_SHIP_W + 9, color);
        // Add energy lines
        for (int i = 0; i < 6; i++) {
            float angle = (millis() / 100.0 + i * 60) * 0.01745;
            int x1 = x + cos(angle) * 18;
            int y1 = y - CARROT_SHIP_W/2 + sin(angle) * (CARROT_SHIP_W + 8);
            int x2 = x + cos(angle) * 22;
            int y2 = y - CARROT_SHIP_W/2 + sin(angle) * (CARROT_SHIP_W + 12);
            tft.drawLine(x1, y1, x2, y2, color);
        }
    }
}

// Draw boss health bar (on right side)
void cg_drawBossHealth(TFT_eSPI &tft, int health, int maxHealth) {
    int barWidth = 20;  // Vertical bar
    int barHeight = 120;
    int barX = SCREEN_W - 30;  // FIXED: Far right side
    int barY = (SCREEN_H - barHeight) / 2;  // Centered vertically
    
    // Background
    tft.fillRect(barX, barY, barWidth, barHeight, TFT_BLACK);
    tft.drawRect(barX, barY, barWidth, barHeight, TFT_WHITE);
    
    // Health fill (from bottom up)
    int fillHeight = (barHeight - 4) * health / maxHealth;
    uint16_t healthColor = TFT_GREEN;
    if (health < maxHealth / 3) healthColor = TFT_RED;
    else if (health < maxHealth * 2 / 3) healthColor = TFT_YELLOW;
    
    if (fillHeight > 0) {
        tft.fillRect(barX + 2, barY + barHeight - 2 - fillHeight, barWidth - 4, fillHeight, healthColor);
    }
}


// NEW: Smaller carrot ship facing DOWN (for boss level)
void cg_drawVectorCarrotDownSmall(TFT_eSPI &tft, int x, int y, bool erase=false) {
    uint16_t carrot = erase ? COLOR_BG : COLOR_CARROT;
    uint16_t green = erase ? COLOR_BG : COLOR_CARROT_TOP;
    uint16_t jet1 = erase ? COLOR_BG : COLOR_JET1;
    uint16_t jet2 = erase ? COLOR_BG : COLOR_JET2;
    uint16_t cockpit = erase ? COLOR_BG : TFT_CYAN;
    
    int shipW = CARROT_SHIP_W / 2;  // Half size
    
    // Main body (vertical, pointing down)
    tft.drawLine(x, y, x, y+shipW, carrot);
    tft.drawLine(x-2, y, x-2, y+shipW-2, carrot);
    tft.drawLine(x+2, y, x+2, y+shipW-2, carrot);
    tft.drawLine(x, y+shipW, x-2, y+shipW-2, carrot);
    tft.drawLine(x, y+shipW, x+2, y+shipW-2, carrot);
    
    // Green top
    tft.drawLine(x, y+shipW, x-3, y+shipW+3, green);
    tft.drawLine(x, y+shipW, x, y+shipW+4, green);
    tft.drawLine(x, y+shipW, x+3, y+shipW+3, green);
    
    // Jets (pointing up)
    tft.drawLine(x, y-4, x, y-9, jet1);
    tft.drawLine(x-1, y-4, x-2, y-8, jet2);
    tft.drawLine(x+1, y-4, x+2, y-8, jet2);
    
    // Cockpit
    tft.drawLine(x-1, y+shipW-4, x-1, y+shipW-1, cockpit);
    tft.drawLine(x, y+shipW-4, x, y+shipW-1, cockpit);
    tft.drawLine(x+1, y+shipW-4, x+1, y+shipW-1, cockpit);
}

void cg_drawVectorShipBoss(TFT_eSPI &tft, int x, int y, bool erase=false) {
    uint16_t white = erase ? COLOR_BG : TFT_WHITE;
    uint16_t blue = erase ? COLOR_BG : TFT_BLUE;
    uint16_t yellow = erase ? COLOR_BG : TFT_YELLOW;
    
    // FIXED: Smaller ship - reduced all dimensions by ~40%
    // Main body pointing UP
    tft.drawLine(x, y, x, y-30, white);  // Center line (was -50)
    tft.drawLine(x, y-30, x-8, y-24, white);  // Nose left (was -12, -40)
    tft.drawLine(x, y-30, x+8, y-24, white);  // Nose right
    
    // Main wings
    tft.drawLine(x, y, x-12, y+10, white);  // Left wing (was -18, +15)
    tft.drawLine(x, y, x+12, y+10, white);  // Right wing
    tft.drawLine(x-12, y+10, x-8, y-24, white);  // Left wing to nose
    tft.drawLine(x+12, y+10, x+8, y-24, white);  // Right wing to nose
    
    // Outer wings
    tft.drawLine(x-12, y+10, x-16, y+20, white);  // (was -24, +30)
    tft.drawLine(x+12, y+10, x+16, y+20, white);
    tft.drawLine(x-16, y+20, x-10, y-20, white);  // (was -24, +30, -15, -35)
    tft.drawLine(x+16, y+20, x+10, y-20, white);
    
    // Wing tips blue
    tft.drawLine(x-16, y+20, x-18, y+20, blue);  // (was -28)
    tft.drawLine(x+16, y+20, x+18, y+20, blue);
    
    // Wing tip yellow
    tft.drawLine(x-18, y+20, x-20, y+22, yellow);  // (was -30, +32)
    tft.drawLine(x+18, y+20, x+20, y+22, yellow);
    
    // Rear engines
    tft.drawLine(x, y, x-6, y+8, white);  // (was -10, +12)
    tft.drawLine(x, y, x+6, y+8, white);
    tft.drawLine(x-6, y+8, x-10, y+5, white);  // (was -15, +8)
    tft.drawLine(x+6, y+8, x+10, y+5, white);
    
    // Cockpit
    tft.drawLine(x-3, y-12, x+3, y-12, blue);  // (was -5, -20)
    tft.drawLine(x-3, y-9, x+3, y-9, blue);  // (was -5, -15)
    tft.drawLine(x-3, y-12, x-3, y-9, yellow);
    tft.drawLine(x+3, y-12, x+3, y-9, yellow);
}

// Calculate difficulty multipliers based on level
float getDifficultyMultiplier(int level) {
    if (level <= 4) return 1.0f;
    // Each loop makes it 15% harder
    int loops = (level - 1) / 4;
    return 1.0f + (loops * 0.15f);
}

// ---- Main game loop -----
void run_Captain_Gordo(TFT_eSPI &tft) {
    tft.setRotation(3);
    pinMode(PIN_KO, INPUT_PULLUP);
    pinMode(PIN_PUSH, INPUT_PULLUP);

    int score = 0;
    int lives = MAX_LIVES;
    int currentGameLevel = 1;  // Track which level (1-4, then repeats as 5-8, etc)
    
    bool firstGame = true;
    
    while (1) {  // Outer game loop (replay loop)
        captainGordoSplash(tft);
        if (firstGame) {
            run_CG_ChaseIntro(tft);
            run_CG_AsteroidCutscene(tft);
            firstGame = false;
        }

        // --- Initialize variables ---
        score = 0;
        lives = MAX_LIVES;
        currentGameLevel = 1;
        
// LEVEL LOOP - infinite levels with increasing difficulty
while (lives > 0) {
            int levelType = ((currentGameLevel - 1) % 4) + 1;  // 1, 2, 3, or 4
            bool vertical = (levelType == 2);
            bool leftFacing = (levelType == 3);
            bool bossLevel = (levelType == 4);
            
            if (bossLevel) {
                cg_showBossSplash(tft);
                
                // BOSS LEVEL CODE
                tft.fillScreen(COLOR_BG);
                cg_initStarfield(0);  // White stars
                
                for (int i=0; i<MAX_BULLETS; i++) bullets[i].active = false;
                
                int carrotX = SCREEN_W / 2;
                int carrotY = 45; 
                int prevCarrotX = carrotX;
                int prevCarrotY = carrotY;
                
// Boss variables
                int bossX = SCREEN_W / 2;
                int bossY = SCREEN_H - 50;  // FIXED: Very close to floor (was - 80)
float diffMult = getDifficultyMultiplier(currentGameLevel);
int bossHealth = (int)(BOSS_MAX_HEALTH * diffMult);
int maxBossHealth = bossHealth;  // Store for health bar
                int bossDirection = 1;
                int bossFireTimer = 0;
                bool bossDefeated = false;
                
                struct BossBullet { float x, y, vx, vy; bool active; };
                BossBullet bossBullets[MAX_BOSS_BULLETS];
                for (int i=0; i<MAX_BOSS_BULLETS; i++) bossBullets[i].active = false;
                
                // SHIELD VARIABLES for boss level
                bool shieldActive = false;
                bool shieldAvailable = true;
                unsigned long shieldActivatedTime = 0;
                unsigned long shieldChargeStartTime = 0;
                const unsigned long SHIELD_DURATION = 5000;
                const unsigned long SHIELD_CHARGE_TIME = 30000;
                bool prevShieldActive = false;
                
                int lastScore = -1;
                int lastLives = -1;
                bool lastShieldAvailable = true;
                
              //  unsigned long levelStart = millis();
                
                // Boss fight loop
                while (!bossDefeated && lives > 0) {
                    cg_eraseStarfield(tft);
                    cg_moveStarfield(false);
                    cg_drawStarfield(tft);
                    
                    cg_drawBossFloor(tft);
                    cg_drawHatch(tft, false);
                    
// Draw score
                    if (score != lastScore) {
                        tft.fillRect(0, 0, 100, 25, COLOR_BG);
                        char scorebuf[12];
                        snprintf(scorebuf, sizeof(scorebuf), "%d", score);
                        tft.setTextColor(COLOR_SCORE, COLOR_BG);
                        tft.setTextFont(2);
                        tft.setTextDatum(TL_DATUM);
                        tft.drawString(scorebuf, 8, 5);
                        
                        // FIXED: Award extra life every 10,000 points in boss level too
                        static int lastBossLifeScore = 0;
                        if (score / 10000 > lastBossLifeScore / 10000) {
                            if (lives < MAX_LIVES) {
                                lives++;
                                lastLives = -1;  // Force lives redraw
                            }
                        }
                        lastBossLifeScore = score;
                        
                        lastScore = score;
                    }
                    
                    // Draw lives
                    if (lives != lastLives) {
                        tft.fillRect(SCREEN_W-100, 0, 100, 25, COLOR_BG);
                        for (int i=0; i<lives; i++)
                            cg_drawMiniCarrot(tft, SCREEN_W-20-16*i, 17, false);
                        lastLives = lives;
                    }
                    
                    // Draw mini shield
                    if (shieldAvailable != lastShieldAvailable) {
                        tft.fillRect(SCREEN_W/2 - 15, 0, 30, 25, COLOR_BG);
                        lastShieldAvailable = shieldAvailable;
                    }
                    cg_drawMiniShield(tft, SCREEN_W/2, 15, shieldAvailable);
                    
                    // Shield activation
                    static int lastPush = HIGH;
                    int push = digitalRead(PIN_PUSH);
                    if (push == LOW && lastPush == HIGH && shieldAvailable && !shieldActive) {
                        shieldActive = true;
                        shieldAvailable = false;
                        shieldActivatedTime = millis();
                        shieldChargeStartTime = 0;
                    }
                    lastPush = push;
                    
                    // Update shield state
                    if (shieldActive) {
                        if (millis() - shieldActivatedTime > SHIELD_DURATION) {
                            shieldActive = false;
                            shieldChargeStartTime = millis();
                        }
                    } else if (!shieldAvailable) {
                        if (millis() - shieldChargeStartTime > SHIELD_CHARGE_TIME) {
                            shieldAvailable = true;
                        }
                    }
                    
                    // Boss health bar
                    cg_drawBossHealth(tft, bossHealth, maxBossHealth);
                    
// Erase carrot at old position
if (prevShieldActive) {
    cg_drawShield(tft, prevCarrotX, prevCarrotY, true, true, true);  // Erase shield (bossLevel=true)
} else {
    cg_drawVectorCarrotDownSmall(tft, prevCarrotX, prevCarrotY, true);
}
                    
                    // Move carrot left/right
                    extern volatile int rotaryPos;
                    static int lastRotary = rotaryPos;
                    int diff = rotaryPos - lastRotary;
                    lastRotary = rotaryPos;
                    carrotX += diff * 4;
                    if (carrotX < 60) carrotX = 60;
                    if (carrotX > SCREEN_W-60) carrotX = SCREEN_W-60;
                    
// Draw carrot
cg_drawVectorCarrotDownSmall(tft, carrotX, carrotY, false);
if (shieldActive) {
    cg_drawShield(tft, carrotX, carrotY, true, false, true);  // bossLevel=true
}
                    prevCarrotX = carrotX;
                    prevCarrotY = carrotY;
                    prevShieldActive = shieldActive;
                    
                    // Fire bullets
                    static int lastBtn = HIGH;
                    int btn = digitalRead(PIN_KO);
                    if (btn == LOW && lastBtn == HIGH) {
                        for (int i=0; i<MAX_BULLETS; i++) {
                            if (!bullets[i].active) {
                                bullets[i].active = true;
                                bullets[i].x = carrotX;
                                bullets[i].y = carrotY + CARROT_SHIP_W + 2;
                                bullets[i].vx = 0;
                                bullets[i].vy = 8.0f;  // Shoot down
                                break;
                            }
                        }
                    }
                    lastBtn = btn;
                    
                    // Update player bullets
                    for (int i=0; i<MAX_BULLETS; i++) {
                        if (!bullets[i].active) continue;
                        cg_drawLaserBoltDown(tft, (int)bullets[i].x, (int)bullets[i].y, true);
                        bullets[i].y += bullets[i].vy;
                        if (bullets[i].y > SCREEN_H-50) {
                            bullets[i].active = false;
                            continue;
                        }
                        cg_drawLaserBoltDown(tft, (int)bullets[i].x, (int)bullets[i].y, false);
                    }
                    
                    // Erase boss at old position
                    cg_drawVectorShipBoss(tft, bossX, bossY, true);
                    
                    // Move boss left/right
                    bossX += bossDirection * 3;
                    if (bossX < 80 || bossX > SCREEN_W-80) {
                        bossDirection *= -1;
                    }
                    
                    // Draw boss
                    cg_drawVectorShipBoss(tft, bossX, bossY, false);
                    
// Boss fires bullets upward
                    bossFireTimer++;
float diffMult = getDifficultyMultiplier(currentGameLevel);
int bossFireDelay = (int)(15 / diffMult);
if (bossFireDelay < 8) bossFireDelay = 8;  // Minimum delay
if (bossFireTimer > bossFireDelay) {
                          for (int i=0; i<MAX_BOSS_BULLETS; i++) {
                            if (!bossBullets[i].active) {
                                bossBullets[i].active = true;
                                bossBullets[i].x = bossX + random(-15, 16);  // FIXED: Even smaller spread
                                bossBullets[i].y = bossY - 15;  // FIXED: From top of smaller ship
                                bossBullets[i].vx = 0;
                                bossBullets[i].vy = -7.0f;  // Shoot up
                                break;
                            }
                        }
                        bossFireTimer = 0;
                    }
                    
                    // Update boss bullets
                    for (int i=0; i<MAX_BOSS_BULLETS; i++) {
                        if (!bossBullets[i].active) continue;
                        
                        // Erase
                        uint16_t blue = TFT_BLUE;
                        uint16_t cyan = TFT_CYAN;
                        int lx = (int)bossBullets[i].x;
                        int ly = (int)bossBullets[i].y;
                        tft.drawLine(lx, ly-2, lx, ly+22, COLOR_BG);
                        tft.drawLine(lx-1, ly-2, lx-1, ly+22, COLOR_BG);
                        tft.drawLine(lx+1, ly-2, lx+1, ly+22, COLOR_BG);
                        
                        // Move
                        bossBullets[i].y += bossBullets[i].vy;
                        
                        if (bossBullets[i].y < 0) {
                            bossBullets[i].active = false;
                            continue;
                        }
                        
                        // Draw
                        lx = (int)bossBullets[i].x;
                        ly = (int)bossBullets[i].y;
                        tft.drawLine(lx, ly-2, lx, ly+22, cyan);
                        tft.drawLine(lx-1, ly, lx-1, ly+22, blue);
                        tft.drawLine(lx+1, ly, lx+1, ly+22, cyan);
                    }
                    
// Check player bullet hits boss
                    for (int i=0; i<MAX_BULLETS; i++) {
                        if (!bullets[i].active) continue;
                        if (abs(bullets[i].x - bossX) < 20 && abs(bullets[i].y - bossY) < 25) {  // FIXED: Even smaller hitbox
                            bossHealth--;
                            bullets[i].active = false;
                            cg_drawLaserBoltDown(tft, (int)bullets[i].x, (int)bullets[i].y, true);
                            score += 200;
                            
                            if (bossHealth <= 0) {
                                bossDefeated = true;
                            }
                        }
                    }
                    
                    // Check boss bullet hits player
                    bool lostLife = false;
                    for (int i=0; i<MAX_BOSS_BULLETS; i++) {
                        if (!bossBullets[i].active) continue;
                        if (abs(bossBullets[i].x - carrotX) < 15 && abs(bossBullets[i].y - carrotY) < 20) {
                            if (shieldActive) {
                                int lx = (int)bossBullets[i].x;
                                int ly = (int)bossBullets[i].y;
                                tft.drawLine(lx, ly-2, lx, ly+22, COLOR_BG);
                                tft.drawLine(lx-1, ly-2, lx-1, ly+22, COLOR_BG);
                                tft.drawLine(lx+1, ly-2, lx+1, ly+22, COLOR_BG);
                                bossBullets[i].active = false;
                            } else {
                                lostLife = true;
                            }
                        }
                    }
                    
                        if (lostLife) {
                        lives--;
if (lives <= 0) {
    cg_drawGameOver(tft);
    delay(2500);
    break;
}                        
                        tft.fillScreen(COLOR_BG);
                        delay(650);
                        
                        carrotX = SCREEN_W / 2;
                        carrotY = 40;  // FIXED: Moved UP (was 80)
                        prevCarrotX = carrotX;
                        prevCarrotY = carrotY;
                        
                        for (int i=0; i<MAX_BULLETS; i++) bullets[i].active = false;
                        for (int i=0; i<MAX_BOSS_BULLETS; i++) bossBullets[i].active = false;
                        
if (prevShieldActive) {
    cg_drawShield(tft, prevCarrotX, prevCarrotY, true, true, true);  // Clear any remaining shield
}
shieldActive = false;
shieldAvailable = true;
prevShieldActive = false;
                        lastScore = -1;
                        lastLives = -1;
                        lastShieldAvailable = true;
                    }
                    
                    delay(24);
                }
                
if (bossDefeated) {
    // EXPLOSION!
    tft.fillScreen(COLOR_BG);
    for (int frame = 0; frame < 20; frame++) {
        tft.fillScreen(COLOR_BG);
        cg_drawBossFloor(tft, false);
        cg_drawHatch(tft, false);
        cg_drawBossExplosion(tft, bossX, bossY, frame);
        delay(80);
    }
    
    // Clear screen and show closed hatch one more time
    tft.fillScreen(COLOR_BG);
    cg_drawBossFloor(tft, false);
    cg_drawHatch(tft, false);
    delay(500);
    
    // Hatch opens with animation
    for (int openFrame = 0; openFrame < 8; openFrame++) {
        tft.fillScreen(COLOR_BG);
        cg_drawBossFloor(tft, openFrame > 3);  // Skip floor in hatch area halfway through
        
        if (openFrame < 4) {
            cg_drawHatch(tft, false);  // Closed
        } else {
            cg_drawHatch(tft, true);   // Open
        }
        delay(100);
    }
    
    // Hatch is now fully open, floor doesn't draw in hatch area
    tft.fillScreen(COLOR_BG);
    cg_drawBossFloor(tft, true);  // Skip floor in hatch
    cg_drawHatch(tft, true);      // Open hatch
    delay(300);
    
    // Carrot escapes DOWN through hatch
    int escapeY = 40;  // Start from top of screen
    for (int ey = escapeY; ey < SCREEN_H + 50; ey += 3) {
        tft.fillRect(SCREEN_W/2 - 30, ey - 50, 60, 55, COLOR_BG);
        cg_drawBossFloor(tft, true);  // Skip floor in hatch area
        cg_drawHatch(tft, true);
        cg_drawVectorCarrotDown(tft, SCREEN_W/2, ey, false);
        delay(30);
    }
                    
                    delay(500);
                    
// Only show cutscenes after first boss defeat
if (currentGameLevel == 5) {
    run_CG_ChaseIntro(tft);
    run_CG_AsteroidCutscene(tft);
}
                    
                    score += 5000;  // Huge bonus
                    currentGameLevel++;  // This becomes level 5 (which will display as level 1 again)
                }
                
                if (lives <= 0) break;
                
            } else {
                // NORMAL LEVELS 1-3
                tft.fillScreen(COLOR_BG);
                
                // Show level transition
                char levelBuf[16];
                snprintf(levelBuf, sizeof(levelBuf), "LEVEL %d", currentGameLevel);
                int scale = 2;
                int textW = cgVecTextWidth(levelBuf, scale);
                int textX = (SCREEN_W - textW) / 2;
                cgDrawVecText(tft, levelBuf, textX, 100, scale, TFT_GREEN);
                delay(1500);
                tft.fillScreen(COLOR_BG);
                
                // Initialize starfield based on level
                if (levelType == 1) cg_initStarfield(1);  // Red/pink
                else if (levelType == 2) cg_initStarfield(2);  // Cyan/blue
                else if (levelType == 3) cg_initStarfield(3);  // Orange/yellow
                
                // Reset walls
                wallsInitialized = false;
                
                for (int i=0; i<MAX_BULLETS; i++) bullets[i].active = false;
                for (int i=0; i<MAX_ENEMIES; i++) enemies[i].active = false;
                for (int i=0; i<MAX_ENEMY_BULLETS; i++) enemyBullets[i].active = false;
                
                unsigned long levelStart = millis();
                
// Set starting position based on level type
                int carrotY, carrotX;
                if (levelType == 1) {
                    carrotX = 40;
                    carrotY = SCREEN_H/2;
                } else if (levelType == 2) {
                    carrotX = SCREEN_W/2;
                    carrotY = SCREEN_H - 35;  // FIXED: Even closer to bottom (was - 50)
                } else { // levelType == 3
                    carrotX = SCREEN_W - 35;
                    carrotY = SCREEN_H/2;
                }
                
                int prevCarrotX = carrotX;
                int prevCarrotY = carrotY;
                
                int batCount = 0;
                int gameOver = 0;
                
                // SHIELD VARIABLES
                bool shieldActive = false;
                bool shieldAvailable = true;
                unsigned long shieldActivatedTime = 0;
                unsigned long shieldChargeStartTime = 0;
                const unsigned long SHIELD_DURATION = 5000;
                const unsigned long SHIELD_CHARGE_TIME = 30000;
                bool prevShieldActive = false;
                
                int lastScore = -1;
                int lastLives = -1;
                bool lastShieldAvailable = true;
                
// Enemy colors based on level
                uint16_t batColor, trogColor;
                if (levelType == 1) {
                    batColor = TFT_BLUE;
                    trogColor = TFT_RED;
                } else if (levelType == 2) {
                    batColor = TFT_PURPLE;
                    trogColor = 0xF7BE;  // FIXED: Light purple (was TFT_YELLOW)
                } else { // levelType == 3
                    batColor = TFT_PINK;
                    trogColor = TFT_CYAN;
                }
                
                // Wall color
                uint16_t wallColor = (levelType == 3) ? TFT_YELLOW : (vertical ? COLOR_WALL_VERT : COLOR_WALL);
                
                // --- Level Loop (1 minute per level) ---
                while (!gameOver && (millis() - levelStart) < 60000) { // 60000 = 1 minute
                cg_eraseStarfield(tft);
                    cg_moveStarfield(leftFacing, vertical);  // FIXED: Pass vertical flag
                    cg_drawStarfield(tft);
                    
                    // Draw cave walls
                    cg_drawCaveWalls(tft, vertical, leftFacing, wallColor);
                    
// Draw score
                    if (score != lastScore) {
                        tft.fillRect(0, 0, 100, 25, COLOR_BG);
                        char scorebuf[12];
                        snprintf(scorebuf, sizeof(scorebuf), "%d", score);
                        tft.setTextColor(COLOR_SCORE, COLOR_BG);
                        tft.setTextFont(2);
                        tft.setTextDatum(TL_DATUM);
                        tft.drawString(scorebuf, 8, 5);
                        
                        // FIXED: Award extra life every 10,000 points
                        static int lastLifeScore = 0;
                        if (score / 10000 > lastLifeScore / 10000) {
                            if (lives < MAX_LIVES) {
                                lives++;
                                lastLives = -1;  // Force lives redraw
                            }
                        }
                        lastLifeScore = score;
                        
                        lastScore = score;
                    }
                    
                    // Draw lives
                    if (lives != lastLives) {
                        tft.fillRect(SCREEN_W-100, 0, 100, 25, COLOR_BG);
                        for (int i=0; i<lives; i++)
                            cg_drawMiniCarrot(tft, SCREEN_W-20-16*i, 17, false);
                        lastLives = lives;
                    }
                    
                    // Draw mini shield
                    if (shieldAvailable != lastShieldAvailable) {
                        tft.fillRect(SCREEN_W/2 - 15, 0, 30, 25, COLOR_BG);
                        lastShieldAvailable = shieldAvailable;
                    }
                    cg_drawMiniShield(tft, SCREEN_W/2, 15, shieldAvailable);
                    
                    // Shield activation
                    static int lastPush = HIGH;
                    int push = digitalRead(PIN_PUSH);
                    if (push == LOW && lastPush == HIGH && shieldAvailable && !shieldActive) {
                        shieldActive = true;
                        shieldAvailable = false;
                        shieldActivatedTime = millis();
                        shieldChargeStartTime = 0;
                    }
                    lastPush = push;
                    
                    // Update shield state
                    if (shieldActive) {
                        if (millis() - shieldActivatedTime > SHIELD_DURATION) {
                            shieldActive = false;
                            shieldChargeStartTime = millis();
                        }
                    } else if (!shieldAvailable) {
                        if (millis() - shieldChargeStartTime > SHIELD_CHARGE_TIME) {
                            shieldAvailable = true;
                        }
                    }
                    
                    // Erase carrot at old position
                    if (prevShieldActive) {
                        if (!vertical && !leftFacing) {
                            tft.fillRect(prevCarrotX - 35, prevCarrotY - 35, CARROT_SHIP_W + 70, 70, COLOR_BG);
                        } else if (vertical) {
                            tft.fillRect(prevCarrotX - 35, prevCarrotY - CARROT_SHIP_W - 35, 70, CARROT_SHIP_W + 70, COLOR_BG);
                        } else { // leftFacing
                            tft.fillRect(prevCarrotX - CARROT_SHIP_W - 35, prevCarrotY - 35, CARROT_SHIP_W + 70, 70, COLOR_BG);
                        }
                    } else {
                        if (!vertical && !leftFacing) {
                            cg_drawVectorCarrot(tft, prevCarrotX, prevCarrotY, true);
                        } else if (vertical) {
                            cg_drawVectorCarrotVertical(tft, prevCarrotX, prevCarrotY, true);
                        } else { // leftFacing
                            cg_drawVectorCarrotLeft(tft, prevCarrotX, prevCarrotY, true);
                        }
                    }
                    
                    // Input
                    extern volatile int rotaryPos;
                    static int lastRotary = rotaryPos;
                    int diff = rotaryPos - lastRotary;
                    lastRotary = rotaryPos;
                    
                    if (!vertical && !leftFacing) {
                        // Level 1: Move up/down
                        carrotY += diff * 4;
                        if (carrotY < CARROT_Y_MIN) carrotY = CARROT_Y_MIN;
                        if (carrotY > CARROT_Y_MAX) carrotY = CARROT_Y_MAX;
                    } else if (vertical) {
                        // Level 2: Move left/right
                        carrotX += diff * 4;
                        int leftLimit = 80;
                        int rightLimit = SCREEN_W - 80;
                        if (carrotX < leftLimit) carrotX = leftLimit;
                        if (carrotX > rightLimit) carrotX = rightLimit;
                    } else { // leftFacing
                        // Level 3: Move up/down
                        carrotY += diff * 4;
                        if (carrotY < CARROT_Y_MIN) carrotY = CARROT_Y_MIN;
                        if (carrotY > CARROT_Y_MAX) carrotY = CARROT_Y_MAX;
                    }
                    
                    // Draw carrot
                    if (!vertical && !leftFacing) {
                        cg_drawVectorCarrot(tft, carrotX, carrotY, false);
                    } else if (vertical) {
                        cg_drawVectorCarrotVertical(tft, carrotX, carrotY, false);
                    } else { // leftFacing
                        cg_drawVectorCarrotLeft(tft, carrotX, carrotY, false);
                    }
                    
                    // Draw shield
                    if (shieldActive) {
                        if (!vertical && !leftFacing) {
                            cg_drawShield(tft, carrotX, carrotY, false, false);
                        } else if (vertical) {
                            cg_drawShield(tft, carrotX, carrotY, true, false);
                        } else { // leftFacing
                            cg_drawShield(tft, carrotX, carrotY, false, false);
                        }
                    }
                    
                    prevCarrotX = carrotX;
                    prevCarrotY = carrotY;
                    prevShieldActive = shieldActive;
                    
                    // Fire bullets
                    static int lastBtn = HIGH;
                    int btn = digitalRead(PIN_KO);
                    if (btn == LOW && lastBtn == HIGH) {
                        for (int i=0; i<MAX_BULLETS; i++) {
                            if (!bullets[i].active) {
                                bullets[i].active = true;
                                if (!vertical && !leftFacing) {
                                    // Level 1: Shoot right
                                    bullets[i].x = carrotX+CARROT_SHIP_W+2;
                                    bullets[i].y = carrotY;
                                    bullets[i].vx = 8.0f;
                                    bullets[i].vy = 0;
                                } else if (vertical) {
                                    // Level 2: Shoot up
                                    bullets[i].x = carrotX;
                                    bullets[i].y = carrotY-CARROT_SHIP_W-2;
                                    bullets[i].vx = 0;
                                    bullets[i].vy = -8.0f;
                                } else { // leftFacing
                                    // Level 3: Shoot left
                                    bullets[i].x = carrotX-CARROT_SHIP_W-2;
                                    bullets[i].y = carrotY;
                                    bullets[i].vx = -8.0f;
                                    bullets[i].vy = 0;
                                }
                                break;
                            }
                        }
                    }
                    lastBtn = btn;
                    
                    // Update and draw bullets
                    for (int i=0; i<MAX_BULLETS; i++) {
                        if (!bullets[i].active) continue;
                        
                        // Erase
                        if (!vertical && !leftFacing) {
                            cg_drawLaserBolt(tft, (int)bullets[i].x, (int)bullets[i].y, true);
                        } else if (vertical) {
                            cg_drawLaserBoltVertical(tft, (int)bullets[i].x, (int)bullets[i].y, true);
                        } else { // leftFacing
                            cg_drawLaserBoltLeft(tft, (int)bullets[i].x, (int)bullets[i].y, true);
                        }
                        
                        // Move
                        bullets[i].x += bullets[i].vx;
                        bullets[i].y += bullets[i].vy;
                        
                        if (bullets[i].x < 0 || bullets[i].x > SCREEN_W || bullets[i].y < 0 || bullets[i].y > SCREEN_H) {
                            bullets[i].active = false;
                            continue;
                        }
                        
                        // Draw
                        if (!vertical && !leftFacing) {
                            cg_drawLaserBolt(tft, (int)bullets[i].x, (int)bullets[i].y, false);
                        } else if (vertical) {
                            cg_drawLaserBoltVertical(tft, (int)bullets[i].x, (int)bullets[i].y, false);
                        } else { // leftFacing
                            cg_drawLaserBoltLeft(tft, (int)bullets[i].x, (int)bullets[i].y, false);
                        }
                    }
                    
// Spawn enemies
                    static int enemySpawnTimer = 0;
                    enemySpawnTimer++;
                    float diffMult = getDifficultyMultiplier(currentGameLevel);
int spawnDelay = (int)(36 / diffMult);
if (spawnDelay < 12) spawnDelay = 12;  // Minimum delay
if (enemySpawnTimer > spawnDelay) {
                        for (int i=0; i<MAX_ENEMIES; i++) {
                            if (!enemies[i].active) {
                                enemies[i].active = true;
                                enemies[i].type = (batCount >= 5) ? (random(0,2) ? 0 : 1) : 0;
                                
                                if (!vertical && !leftFacing) {
                                    // Level 1: Spawn from right
                                    enemies[i].x = SCREEN_W-16;
                                    int topLimit = wall_top[SCREEN_W-16] + 20;
                                    int botLimit = wall_bot[SCREEN_W-16] - 20;
                                    enemies[i].y = random(topLimit, botLimit);
                                } else if (vertical) {
                                    // Level 2: Spawn from top
                                    if (enemies[i].type == 1) {
                                        // FIXED: Troglobites only on TOP wall, not bottom
                                        int wallIdx = random(80, SCREEN_W-80);
                                        enemies[i].x = wallIdx;
                                        enemies[i].y = 35;  // Only spawn at top
                                    } else {
                                        enemies[i].x = random(80, SCREEN_W-80);
                                        enemies[i].y = 30;
                                    }
                                } else { // leftFacing
                                    // Level 3: Spawn from left
                                    enemies[i].x = 16;
                                    int topLimit = wall_top[16] + 20;
                                    int botLimit = wall_bot[16] - 20;
                                    enemies[i].y = random(topLimit, botLimit);
                                }
                                
                                enemies[i].frame = 0;
                                enemies[i].fireTimer = 0;
                                batCount += (enemies[i].type==0);
                                break;
                            }
                        }
                        enemySpawnTimer = 0;
                    }
                    
                    // Update and draw enemies
                    for (int i=0; i<MAX_ENEMIES; i++) {
                        if (!enemies[i].active) continue;
                        
                        // Erase
                        if (enemies[i].type == 0) {
                            cg_drawBat(tft, (int)enemies[i].x, (int)enemies[i].y, true, batColor);
                        } else {
                            bool onCeiling = (enemies[i].y < SCREEN_H/2);
                            cg_drawTroglobite(tft, (int)enemies[i].x, (int)enemies[i].y, true, onCeiling, trogColor);
                        }
                        
                        // Move
                        if (enemies[i].type == 0) { // bat
                            if (!vertical && !leftFacing) {
float diffMult = getDifficultyMultiplier(currentGameLevel);
enemies[i].x -= (3.5f * diffMult);
                                float dy = carrotY - enemies[i].y;
                                if (abs(dy) > 2) {
                                    enemies[i].y += (dy > 0) ? 1.5f : -1.5f;
                                }
                                int ex = (int)enemies[i].x;
                                if (ex >= 0 && ex < SCREEN_W) {
                                    int wallIdx = (ex / 4) * 4;
                                    if (wallIdx < SCREEN_W-4) {
                                        int topLimit = wall_top[wallIdx] + 15;
                                        int botLimit = wall_bot[wallIdx] - 15;
                                        if (enemies[i].y < topLimit) enemies[i].y = topLimit;
                                        if (enemies[i].y > botLimit) enemies[i].y = botLimit;
                                    }
                                }
                            } else if (vertical) {
                                enemies[i].y += 3.5f;
                                float dx = carrotX - enemies[i].x;
                                if (abs(dx) > 2) {
                                    enemies[i].x += (dx > 0) ? 1.5f : -1.5f;
                                }
                                int ey = (int)enemies[i].y;
                                if (ey >= 0 && ey < SCREEN_H) {
                                    int wallIdx = (ey / 4) * 4;
                                    if (wallIdx < SCREEN_H-4) {
                                        int leftLimit = wall_left[wallIdx] + 15;
                                        int rightLimit = wall_right[wallIdx] - 15;
                                        if (enemies[i].x < leftLimit) enemies[i].x = leftLimit;
                                        if (enemies[i].x > rightLimit) enemies[i].x = rightLimit;
                                    }
                                }
                            } else { // leftFacing
                                enemies[i].x += 3.5f;  // Move right (toward ship on right side)
                                float dy = carrotY - enemies[i].y;
                                if (abs(dy) > 2) {
                                    enemies[i].y += (dy > 0) ? 1.5f : -1.5f;
                                }
                                int ex = (int)enemies[i].x;
                                if (ex >= 0 && ex < SCREEN_W) {
                                    int wallIdx = (ex / 4) * 4;
                                    if (wallIdx < SCREEN_W-4) {
                                        int topLimit = wall_top[wallIdx] + 15;
                                        int botLimit = wall_bot[wallIdx] - 15;
                                        if (enemies[i].y < topLimit) enemies[i].y = topLimit;
                                        if (enemies[i].y > botLimit) enemies[i].y = botLimit;
                                    }
                                }
                            }
                            
                            if ((!vertical && !leftFacing && enemies[i].x < 0) || 
                                (vertical && enemies[i].y > SCREEN_H) ||
                                (leftFacing && enemies[i].x > SCREEN_W)) {
                                enemies[i].active = false;
                                continue;
                            }
                            
                            cg_drawBat(tft, (int)enemies[i].x, (int)enemies[i].y, false, batColor);
                            
                        } else { // troglobite
                            if (!vertical && !leftFacing) {
                                enemies[i].x -= 2.8f;
                            } else if (vertical) {
                                // Stay on ceiling/floor
                            } else { // leftFacing
                                enemies[i].x += 2.8f;  // Move right
                            }
                            
                            enemies[i].fireTimer++;
float diffMult = getDifficultyMultiplier(currentGameLevel);
int fireDelay = (int)(32 / diffMult);
if (fireDelay < 15) fireDelay = 15;  // Minimum delay
if (enemies[i].fireTimer >= fireDelay) {
                                  for (int j=0; j<MAX_ENEMY_BULLETS; j++) {
                                    if (!enemyBullets[j].active) {
                                        enemyBullets[j].active = true;
                                        enemyBullets[j].x = enemies[i].x;
                                        enemyBullets[j].y = enemies[i].y;
                                        float angle = atan2(carrotY - enemies[i].y, carrotX - enemies[i].x);
                                        angle += random(-15, 16) * 0.01745f;
                                        enemyBullets[j].vx = cos(angle) * 7.0f;
                                        enemyBullets[j].vy = sin(angle) * 7.0f;
                                        break;
                                    }
                                }
                                enemies[i].fireTimer = 0;
                            }
                            
                            if ((!vertical && !leftFacing && enemies[i].x < 0) ||
                                (vertical && enemies[i].x < 0) ||
                                (leftFacing && enemies[i].x > SCREEN_W)) {
                                enemies[i].active = false;
                                continue;
                            }
                            
                            bool onCeiling = (enemies[i].y < SCREEN_H/2);
                            cg_drawTroglobite(tft, (int)enemies[i].x, (int)enemies[i].y, false, onCeiling, trogColor);
                        }
                    }
                    
                    // Update enemy bullets
                    for (int i=0; i<MAX_ENEMY_BULLETS; i++) {
                        if (!enemyBullets[i].active) continue;
                        cg_drawTroglobiteShot(tft, (int)enemyBullets[i].x, (int)enemyBullets[i].y, true);
                        enemyBullets[i].x += enemyBullets[i].vx;
                        enemyBullets[i].y += enemyBullets[i].vy;
                        if (enemyBullets[i].x < 0 || enemyBullets[i].x > SCREEN_W || 
                            enemyBullets[i].y < 0 || enemyBullets[i].y > SCREEN_H) {
                            enemyBullets[i].active = false;
                            continue;
                        }
                        cg_drawTroglobiteShot(tft, (int)enemyBullets[i].x, (int)enemyBullets[i].y, false);
                    }
                    
                    // Check collisions
                    bool lostLife = false;
                    
                    if (!shieldActive && cg_checkWallCollision(carrotX + CARROT_SHIP_W/2, carrotY, vertical)) {
                        lostLife = true;
                    }
                    
                    for (int i=0; i<MAX_ENEMIES; i++) {
                        if (!enemies[i].active) continue;
                        int ex = (int)enemies[i].x, ey = (int)enemies[i].y;
                        if (abs(carrotX+CARROT_SHIP_W/2-ex)<18 && abs(carrotY-ey)<18) {
                            if (shieldActive) {
                                if (enemies[i].type == 0) {
                                    cg_drawBat(tft, ex, ey, true, batColor);
                                } else {
                                    bool onCeiling = (enemies[i].y < SCREEN_H/2);
                                    cg_drawTroglobite(tft, ex, ey, true, onCeiling, trogColor);
                                }
                                enemies[i].active = false;
                                score += 50;
                            } else {
                                lostLife = true;
                            }
                        }
                    }
                    
                    for (int i=0; i<MAX_ENEMY_BULLETS; i++) {
                        if (!enemyBullets[i].active) continue;
                        if (abs(carrotX+CARROT_SHIP_W/2-enemyBullets[i].x)<12 && abs(carrotY-enemyBullets[i].y)<12) {
                            if (shieldActive) {
                                cg_drawTroglobiteShot(tft, (int)enemyBullets[i].x, (int)enemyBullets[i].y, true);
                                enemyBullets[i].active = false;
                            } else {
                                lostLife = true;
                            }
                        }
                    }
                    
                    // Bullet with enemy
                    for (int i=0; i<MAX_BULLETS; i++) {
                        if (!bullets[i].active) continue;
                        for (int j=0; j<MAX_ENEMIES; j++) {
                            if (!enemies[j].active) continue;
                            if (abs(bullets[i].x-enemies[j].x)<12 && abs(bullets[i].y-enemies[j].y)<12) {
                                if (enemies[j].type == 0) {
                                    cg_drawBat(tft, (int)enemies[j].x, (int)enemies[j].y, true, batColor);
                                } else {
                                    bool onCeiling = (enemies[j].y < SCREEN_H/2);
                                    cg_drawTroglobite(tft, (int)enemies[j].x, (int)enemies[j].y, true, onCeiling, trogColor);
                                }
                                
                                if (!vertical && !leftFacing) {
                                    cg_drawLaserBolt(tft, (int)bullets[i].x, (int)bullets[i].y, true);
                                } else if (vertical) {
                                    cg_drawLaserBoltVertical(tft, (int)bullets[i].x, (int)bullets[i].y, true);
                                } else {
                                    cg_drawLaserBoltLeft(tft, (int)bullets[i].x, (int)bullets[i].y, true);
                                }
                                
                                enemies[j].active = false;
                                bullets[i].active = false;
                                score += 100;
                            }
                        }
                    }
                    
                    // Bullet with enemy bullet
                    for (int i=0; i<MAX_BULLETS; i++) {
                        if (!bullets[i].active) continue;
                        for (int j=0; j<MAX_ENEMY_BULLETS; j++) {
                            if (!enemyBullets[j].active) continue;
                            if (abs(bullets[i].x-enemyBullets[j].x)<8 && abs(bullets[i].y-enemyBullets[j].y)<8) {
                                if (!vertical && !leftFacing) {
                                    cg_drawLaserBolt(tft, (int)bullets[i].x, (int)bullets[i].y, true);
                                } else if (vertical) {
                                    cg_drawLaserBoltVertical(tft, (int)bullets[i].x, (int)bullets[i].y, true);
                                } else {
                                    cg_drawLaserBoltLeft(tft, (int)bullets[i].x, (int)bullets[i].y, true);
                                }
                                cg_drawTroglobiteShot(tft, (int)enemyBullets[j].x, (int)enemyBullets[j].y, true);
                                enemyBullets[j].active = false;
                                bullets[i].active = false;
                            }
                        }
                    }
                    
if (lostLife) {
    lives--;
    if (lives <= 0) {
        tft.fillScreen(COLOR_BG);
        cg_drawGameOver(tft);
        delay(2500);
        break;
    }
    
    tft.fillScreen(COLOR_BG);
    delay(650);
                        
// Respawn
                        if (levelType == 1) {
                            carrotX = 40;
                            carrotY = SCREEN_H/2;
                        } else if (levelType == 2) {
                            carrotX = SCREEN_W/2;
                            carrotY = SCREEN_H - 35;  // FIXED: Even closer to bottom
                        } else {
                            carrotX = SCREEN_W - 35;
                            carrotY = SCREEN_H/2;
                        }
                        
                        prevCarrotX = carrotX;
                        prevCarrotY = carrotY;
                        
                        for (int i=0; i<MAX_BULLETS; i++) bullets[i].active = false;
                        for (int i=0; i<MAX_ENEMIES; i++) enemies[i].active = false;
                        for (int i=0; i<MAX_ENEMY_BULLETS; i++) enemyBullets[i].active = false;
                        
                        shieldActive = false;
                        shieldAvailable = true;
                        prevShieldActive = false;
                        lastScore = -1;
                        lastLives = -1;
                        lastShieldAvailable = true;
                        continue;
                    }
                    
                    delay(24);
                }
                
                // Level complete (1 minute passed)
                if (lives > 0) {
                    score += 1000;
                    currentGameLevel++;
                }
            }
            
            if (lives <= 0) break;
        }
        
// Game over - loop will restart and show splash screen
        if (lives <= 0) {
            // Don't return - let the outer loop continue to show splash again
            firstGame = true;  // Show cutscenes again on next playthrough
        }
    } // End of outer replay loop
} // End of run_Captain_Gordo function

#endif // CAPTAIN_GORDO_H