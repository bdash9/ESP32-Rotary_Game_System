#ifndef TEMPEST_GORDO_H
#define TEMPEST_GORDO_H

#include <math.h>
#include <stdlib.h>
//#include <TFT_eSPI.h>

#define PIN_KO   22
#define PIN_PUSH 25
#define NUM_RIM_POINTS 16
#define TUNNEL_RADIUS 90
#define TUNNEL_DEPTH  90
#define TUNNEL_CENTER_X 160
#define TUNNEL_CENTER_Y 120
#define SCREEN_W 320
#define SCREEN_H 240
#define MAX_SHOTS 6
#define MAX_LIVES 3
#define MAX_ALIENS 24

#define COLOR_TUNNEL_1 TFT_BLUE
#define COLOR_LEVEL    TFT_BLUE
#define COLOR_GORDO    TFT_DARKGREY
#define COLOR_GORDO_V  TFT_WHITE
#define COLOR_BUNNY    TFT_DARKGREY
#define COLOR_CARROT   0xFDA0
#define COLOR_CARROT_TOP 0x03A0
#define COLOR_SEGMENT_ACTIVE TFT_WHITE

enum AlienType { FLIPPER, PULSAR };
enum AlienState { GOING_OUT, AT_RIM, GOING_IN, AT_CENTER };

struct Alien {
    AlienType type;
    int seg;
    float depth;
    bool alive;
    AlienState state;
    unsigned long lingerStart;
};

struct Shot {
  bool active;
  float angle;
  float pos;
  float last_angle;
  float last_pos; 
  float last_base_pos;
  int last_x, last_y;
};

Shot shots[MAX_SHOTS];
Alien aliens[MAX_ALIENS];

struct RimPoint {
  int x, y;
};
RimPoint rim[NUM_RIM_POINTS];
RimPoint inner[NUM_RIM_POINTS];

int playerScore = 0;
int playerLives = MAX_LIVES;
int currentLevel = 1;

// Splash screen vector font
struct VecStroke { int x0, y0, x1, y1; };
struct VecLetter { const VecStroke* strokes; int n; int w; };
const VecStroke strokes_G[] = {{0,0,8,0},{8,0,8,12},{8,12,0,12},{0,12,0,0},{0,6,5,6},{5,6,5,12}};
const VecStroke strokes_O[] = {{0,0,8,0},{8,0,8,12},{8,12,0,12},{0,12,0,0}};
const VecStroke strokes_R[] = {{0,12,0,0},{0,0,8,0},{8,0,8,6},{8,6,0,6},{0,6,8,12}};
const VecStroke strokes_D[] = {{0,12,0,0},{0,0,7,2},{7,2,8,6},{8,6,7,10},{7,10,0,12}};
const VecStroke strokes_E[] = {{8,0,0,0},{0,0,0,12},{0,6,6,6},{0,12,8,12}};
const VecStroke strokes_M[] = {{0,12,0,0},{0,0,4,6},{4,6,8,0},{8,0,8,12}};
const VecStroke strokes_P[] = {{0,12,0,0},{0,0,8,0},{8,0,8,6},{8,6,0,6}};
const VecStroke strokes_S[] = {{8,0,0,0},{0,0,0,6},{0,6,8,6},{8,6,8,12},{8,12,0,12}};
const VecStroke strokes_T[] = {{4,0,4,12},{0,0,8,0}};
const VecLetter vecFont[] = {
    {strokes_G, sizeof(strokes_G)/sizeof(VecStroke), 9},
    {strokes_O, sizeof(strokes_O)/sizeof(VecStroke), 9},
    {strokes_R, sizeof(strokes_R)/sizeof(VecStroke), 9},
    {strokes_D, sizeof(strokes_D)/sizeof(VecStroke), 9},
    {strokes_E, sizeof(strokes_E)/sizeof(VecStroke), 9},
    {strokes_M, sizeof(strokes_M)/sizeof(VecStroke), 9},
    {strokes_P, sizeof(strokes_P)/sizeof(VecStroke), 9},
    {strokes_S, sizeof(strokes_S)/sizeof(VecStroke), 9},
    {strokes_T, sizeof(strokes_T)/sizeof(VecStroke), 9},
};
int getVecFontIdx(char c) {
    switch(c) {
        case 'G': return 0; case 'O': return 1; case 'R': return 2; case 'D': return 3;
        case 'E': return 4; case 'M': return 5; case 'P': return 6; case 'S': return 7; case 'T': return 8;
        default: return -1;
    }
}
const uint16_t rainbow[7] = {TFT_RED, 0xFD20, TFT_YELLOW, TFT_GREEN, TFT_CYAN, TFT_BLUE, TFT_MAGENTA};

void drawVecLetter(TFT_eSPI &tft, char c, int x, int y, int scale, int rainbowDir, int trailLen) {
    int idx = getVecFontIdx(c);
    if(idx < 0) return;
    for (int t = 0; t < trailLen; t++) {
        uint16_t color = rainbow[(rainbowDir > 0 ? t : trailLen-t-1) % 7];
        int trailOffset = t * 2 * scale * rainbowDir;
        for (int i = 0; i < vecFont[idx].n; i++) {
            int x0 = x + vecFont[idx].strokes[i].x0 * scale;
            int y0 = y + vecFont[idx].strokes[i].y0 * scale + trailOffset;
            int x1 = x + vecFont[idx].strokes[i].x1 * scale;
            int y1 = y + vecFont[idx].strokes[i].y1 * scale + trailOffset;
            tft.drawLine(x0, y0, x1, y1, color);
        }
    }
    for (int i = 0; i < vecFont[idx].n; i++) {
        int x0 = x + vecFont[idx].strokes[i].x0 * scale;
        int y0 = y + vecFont[idx].strokes[i].y0 * scale;
        int x1 = x + vecFont[idx].strokes[i].x1 * scale;
        int y1 = y + vecFont[idx].strokes[i].y1 * scale;
        tft.drawLine(x0, y0, x1, y1, TFT_WHITE);
    }
}

void splashScreen(TFT_eSPI &tft) {
    tft.fillScreen(TFT_BLACK);
    int scale = 4, trailLen = 6, spacing = 9*scale;
    int y_offset = 25;
    const char* gordo = "GORDO";
    int nG = 5;
    int textWidth = nG * spacing;
    int xg = (SCREEN_W - textWidth) / 2;
    int yg = 38 + y_offset;
    for (int i = 0; i < nG; i++)
        drawVecLetter(tft, gordo[i], xg + i * spacing, yg, scale, -1, trailLen);

    const char* tempest = "TEMPEST";
    int nT = 7;
    int textWidthT = nT * spacing;
    int xt = (SCREEN_W - textWidthT) / 2;
    int yt = 90 + y_offset;
    for (int i = 0; i < nT; i++)
        drawVecLetter(tft, tempest[i], xt + i * spacing, yt, scale, +1, trailLen);

    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setTextFont(2);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("Hit fire button to start", SCREEN_W/2, SCREEN_H - 22);
    pinMode(PIN_KO, INPUT_PULLUP);
    while (digitalRead(PIN_KO) == LOW) delay(10);
    while (digitalRead(PIN_KO) == HIGH) delay(10);
    while (digitalRead(PIN_KO) == LOW) delay(10);
}

void computeTunnelGeometry() {
    for (int i=0; i<NUM_RIM_POINTS; i++) {
        float angle = 2 * PI * i / NUM_RIM_POINTS;
        rim[i].x = TUNNEL_CENTER_X + cos(angle) * TUNNEL_RADIUS;
        rim[i].y = TUNNEL_CENTER_Y + sin(angle) * TUNNEL_RADIUS;
        inner[i].x = TUNNEL_CENTER_X + cos(angle) * (TUNNEL_RADIUS - TUNNEL_DEPTH);
        inner[i].y = TUNNEL_CENTER_Y + sin(angle) * (TUNNEL_RADIUS - TUNNEL_DEPTH);
    }
}

void drawTunnel(TFT_eSPI &tft) {
    for (int i=0; i<NUM_RIM_POINTS; i++)
        tft.drawLine(rim[i].x, rim[i].y, rim[(i+1)%NUM_RIM_POINTS].x, rim[(i+1)%NUM_RIM_POINTS].y, COLOR_TUNNEL_1);
    for (int i=0; i<NUM_RIM_POINTS; i++)
        tft.drawLine(rim[i].x, rim[i].y, inner[i].x, inner[i].y, COLOR_TUNNEL_1);
    for (int i=0; i<NUM_RIM_POINTS; i++)
        tft.drawLine(inner[i].x, inner[i].y, inner[(i+1)%NUM_RIM_POINTS].x, inner[(i+1)%NUM_RIM_POINTS].y, COLOR_TUNNEL_1);
}

void drawSmallGordo(TFT_eSPI &tft, int x, int y) {
    tft.fillCircle(x, y, 6, COLOR_GORDO);
    tft.fillRect(x-2, y-13, 4, 8, COLOR_GORDO);
    tft.drawLine(x, y-6, x-2, y+4, COLOR_GORDO_V);
    tft.drawLine(x, y-6, x+2, y+4, COLOR_GORDO_V);
    tft.drawLine(x, y-6, x, y+4, COLOR_GORDO_V);
}

void drawLives(TFT_eSPI &tft, int lives) {
    int radius = 6, spacing = 18;
    int x0 = 319 - radius - (lives-1)*spacing;
    int y0 = 20;
    for (int i=0; i<lives; i++) {
        int x = x0 + i*spacing, y = y0;
        drawSmallGordo(tft, x, y);
    }
}

void drawScore(TFT_eSPI &tft, int score) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextFont(2);
    tft.setTextDatum(TL_DATUM);
    char buf[12];
    snprintf(buf, sizeof(buf), "%d", score);
    tft.drawString(buf, 5, 5);
}

void drawLevel(TFT_eSPI &tft, int level) {
    tft.setTextColor(COLOR_LEVEL, TFT_BLACK);
    tft.setTextFont(2);
    tft.setTextDatum(TC_DATUM);
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", level);
    tft.drawString(buf, TUNNEL_CENTER_X, 5);
}

void getSegmentCenter(int segIdx, int &x, int &y) {
    float angle = (2 * PI * (segIdx + 0.5f)) / NUM_RIM_POINTS;
    x = TUNNEL_CENTER_X + cos(angle) * TUNNEL_RADIUS;
    y = TUNNEL_CENTER_Y + sin(angle) * TUNNEL_RADIUS;
}

void drawGordo(TFT_eSPI &tft, int segIdx, bool erase = false) {
    int x, y;
    getSegmentCenter(segIdx, x, y);

    uint16_t head = erase ? TFT_BLACK : COLOR_GORDO;
    uint16_t vcol = erase ? TFT_BLACK : COLOR_GORDO_V;
    uint16_t eye = erase ? TFT_BLACK : TFT_BLACK;
    uint16_t nose = erase ? TFT_BLACK : TFT_PINK;

    tft.fillCircle(x, y, 10, head);
    tft.fillRect(x-4, y-25, 4, 15, head);
    tft.fillRect(x+1, y-25, 4, 15, head);
    tft.fillCircle(x-3, y-4, 1, eye);
    tft.fillCircle(x+3, y-4, 1, eye);
    tft.fillTriangle(x-2, y+4, x+2, y+4, x, y+7, nose);
    tft.drawLine(x, y-10, x-3, y+5, vcol);
    tft.drawLine(x, y-10, x+3, y+5, vcol);
    tft.drawLine(x, y-10, x, y+5, vcol);
    tft.drawLine(x-1, y-10, x-2, y+5, vcol);
    tft.drawLine(x+1, y-10, x+2, y+5, vcol);
}

void drawVectorCarrot(TFT_eSPI &tft, float angle, float pos, bool erase = false, int *last_x = nullptr, int *last_y = nullptr) {
    float r_tip = TUNNEL_RADIUS - pos * TUNNEL_DEPTH;
    float bodyLen = 12; // shorter carrot
    float r_base = r_tip + bodyLen; // green base at rim, tip at center
    int tipX = TUNNEL_CENTER_X + cos(angle) * r_tip;
    int tipY = TUNNEL_CENTER_Y + sin(angle) * r_tip;
    int baseX = TUNNEL_CENTER_X + cos(angle) * r_base;
    int baseY = TUNNEL_CENTER_Y + sin(angle) * r_base;
    uint16_t carrot = erase ? TFT_BLACK : COLOR_CARROT;
    uint16_t green = erase ? TFT_BLACK : COLOR_CARROT_TOP;

    float bodyWid = 3; // slimmer carrot
    int baseX1 = baseX + cos(angle + 0.18) * bodyWid;
    int baseY1 = baseY + sin(angle + 0.18) * bodyWid;
    int baseX2 = baseX + cos(angle - 0.18) * bodyWid;
    int baseY2 = baseY + sin(angle - 0.18) * bodyWid;

    // Main carrot body (lines)
    tft.drawLine(baseX1, baseY1, tipX, tipY, carrot);
    tft.drawLine(baseX2, baseY2, tipX, tipY, carrot);
    // Cross lines for "vector shading"
    for (int i = 1; i < 3; i++) {
        float frac = i / 3.0;
        int crossX1 = baseX1 + frac * (tipX - baseX1);
        int crossY1 = baseY1 + frac * (tipY - baseY1);
        int crossX2 = baseX2 + frac * (tipX - baseX2);
        int crossY2 = baseY2 + frac * (tipY - baseY2);
        tft.drawLine(crossX1, crossY1, crossX2, crossY2, carrot);
    }

    // Tip (V)
    int tipLx = tipX + cos(angle + 0.25) * 3;
    int tipLy = tipY + sin(angle + 0.25) * 3;
    int tipRx = tipX + cos(angle - 0.25) * 3;
    int tipRy = tipY + sin(angle - 0.25) * 3;
    tft.drawLine(tipX, tipY, tipLx, tipLy, carrot);
    tft.drawLine(tipX, tipY, tipRx, tipRy, carrot);

    // Green top at rim (three lines)
    for (float gangle = angle-0.5; gangle<=angle+0.5; gangle+=0.5) {
        int gx = baseX + cos(gangle) * 8;
        int gy = baseY + sin(gangle) * 8;
        tft.drawLine(baseX, baseY, gx, gy, green);
    }
    tft.fillRect(baseX-1, baseY-5, 3, 3, green);

    if (!erase && last_x && last_y) {
        *last_x = tipX; *last_y = tipY;
    }
}

void drawAlien(TFT_eSPI &tft, Alien &a, bool erase=false) {
    float angle = 2 * PI * (a.seg + 0.5f) / NUM_RIM_POINTS;
    float r = TUNNEL_RADIUS - a.depth * TUNNEL_DEPTH;
    int cx = TUNNEL_CENTER_X + cos(angle) * r;
    int cy = TUNNEL_CENTER_Y + sin(angle) * r;

    float sizeFactor = 1.0 - a.depth;
    int minSize = 1, maxSize = 7;
    int size = minSize + (int)((maxSize - minSize) * pow(sizeFactor, 2.0));
    if (size < 1) size = 1;

    int eraseMargin = 3 + (size/2);
    int eraseRadius = size + eraseMargin;

    if (erase) {
        tft.fillCircle(cx, cy, eraseRadius, TFT_BLACK);
        tft.fillCircle(cx, cy, 5, TFT_BLACK);
        tft.drawLine(cx-4, cy, cx+4, cy, TFT_BLACK);
        tft.drawLine(cx, cy-4, cx, cy+4, TFT_BLACK);
        tft.drawLine(cx-3, cy-3, cx+3, cy+3, TFT_BLACK);
        tft.drawLine(cx-3, cy+3, cx+3, cy-3, TFT_BLACK);
        return;
    }

    if (a.type == FLIPPER) {
        uint16_t color = TFT_RED;
        tft.drawLine(cx-size, cy-size, cx+size, cy+size, color);
        tft.drawLine(cx+size, cy-size, cx-size, cy+size, color);
        tft.drawLine(cx, cy-size, cx, cy+size, color);
        tft.drawLine(cx-size, cy, cx+size, cy, color);
    } else {
        tft.fillCircle(cx, cy, size, TFT_YELLOW);
        int numZigs = 6;
        for (int i=0; i<numZigs; i++) {
            float a0 = 2*PI*i/numZigs;
            uint16_t color = rainbow[i%7];
            int x0 = cx, y0 = cy;
            int len = size+3;
            for (int j=1; j<=len; j++) {
                int x1 = cx + cos(a0 + (j%2)*0.23) * j;
                int y1 = cy + sin(a0 + (j%2)*0.23) * j;
                tft.drawLine(x0, y0, x1, y1, color);
                x0 = x1; y0 = y1;
            }
        }
    }
}

void initAliens(int nAliens) {
    for (int i=0; i<MAX_ALIENS; i++) {
        aliens[i].alive = false;
    }
    for (int i=0; i<nAliens && i<MAX_ALIENS; i++) {
        aliens[i].type = (rand() % 2 == 0) ? FLIPPER : PULSAR;
        aliens[i].seg = rand() % NUM_RIM_POINTS;
        aliens[i].depth = 1.0;
        aliens[i].alive = true;
        aliens[i].state = GOING_OUT;
        aliens[i].lingerStart = 0;
    }
}

void drawSegmentFull(TFT_eSPI &tft, int segIdx, uint16_t color) {
    int next = (segIdx+1) % NUM_RIM_POINTS;
    tft.drawLine(rim[segIdx].x, rim[segIdx].y, rim[next].x, rim[next].y, color);
    tft.drawLine(rim[segIdx].x, rim[segIdx].y, inner[segIdx].x, inner[segIdx].y, color);
    tft.drawLine(rim[next].x, rim[next].y, inner[next].x, inner[next].y, color);
}

void superZapperEffect(TFT_eSPI &tft) {
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextFont(4);
    tft.setTextDatum(TL_DATUM);
    const char* super = "Super";
    for (int i=0; i<5; i++)
        tft.drawChar(super[i], 5, 40 + i*32, 4);
    const char* zapper = "Zapper!";
    for (int i=0; i<7; i++)
        tft.drawChar(zapper[i], 295, 28 + i*28, 4);
}

void run_Tempest_Gordo(TFT_eSPI &tft) {
    splashScreen(tft); 
    tft.setRotation(3);
    computeTunnelGeometry();

    tft.fillScreen(TFT_BLACK);
    drawTunnel(tft);

    playerScore = 0;
    playerLives = MAX_LIVES;
    currentLevel = 1;

    pinMode(PIN_KO, INPUT_PULLUP);
    pinMode(PIN_PUSH, INPUT_PULLUP);

    for (int i=0; i<MAX_SHOTS; i++) shots[i].active = false;

    extern volatile int rotaryPos;
    int bunny_last_seg_local = -1;
    int active_seg = -1, prev_active_seg = -1;
    int lastButton = HIGH;
    int lastPush = HIGH;
    bool superzapperUsed = false;

    drawScore(tft, playerScore);
    drawLives(tft, playerLives);
    drawLevel(tft, currentLevel);
    int aliensPerLevel = 10 + (currentLevel-1);

    int alienReleaseFrame = 0;
    int aliensReleased = 0;

    initAliens(aliensPerLevel);

    while (1) {
        int playerSeg = ((rotaryPos % NUM_RIM_POINTS) + NUM_RIM_POINTS) % NUM_RIM_POINTS;
        float angle = (2 * PI * (playerSeg + 0.5f)) / NUM_RIM_POINTS;

        active_seg = playerSeg;
        if (active_seg != prev_active_seg) {
            if (prev_active_seg != -1)
                drawSegmentFull(tft, prev_active_seg, COLOR_TUNNEL_1);
            drawSegmentFull(tft, active_seg, COLOR_SEGMENT_ACTIVE);
            prev_active_seg = active_seg;
        }

        // Super Zapper (one per level, rotary push)
        int push = digitalRead(PIN_PUSH);
        if (push == LOW && lastPush == HIGH && !superzapperUsed) {
            for (int i=0; i<MAX_ALIENS; i++) aliens[i].alive = false;
            superzapperUsed = true;
            tft.fillScreen(TFT_BLACK);
            drawTunnel(tft);
            superZapperEffect(tft);
            for (int i=0; i<MAX_ALIENS; i++)
                if (aliens[i].alive) drawAlien(tft, aliens[i], false);
            delay(600);
            tft.fillScreen(TFT_BLACK);
            drawTunnel(tft);
            drawScore(tft, playerScore);
            drawLives(tft, playerLives);
            drawLevel(tft, currentLevel);
        }
        lastPush = push;

        int button = digitalRead(PIN_KO);
        if (button == LOW && lastButton == HIGH) {
            for (int i=0; i<MAX_SHOTS; i++) {
                if (!shots[i].active) {
                    shots[i].active = true;
                    shots[i].angle = angle;
                    shots[i].pos = 0.0;
                    shots[i].last_angle = angle;
                    shots[i].last_pos = 0.0;
                    shots[i].last_base_pos = 0.0;
                    shots[i].last_x = -1;
                    shots[i].last_y = -1;
                    break;
                }
            }
        }
        lastButton = button;

        static int prevBunnySeg = -1;
        if (prevBunnySeg != -1 && prevBunnySeg != playerSeg)
            drawGordo(tft, prevBunnySeg, true);
        drawGordo(tft, playerSeg, false);
        prevBunnySeg = playerSeg;

        // --- Staggered alien release ---
        if (aliensReleased < aliensPerLevel && alienReleaseFrame % 28 == 0) {
            aliens[aliensReleased].type = (rand() % 2 == 0) ? FLIPPER : PULSAR;
            aliens[aliensReleased].seg = rand() % NUM_RIM_POINTS;
            aliens[aliensReleased].depth = 1.0;
            aliens[aliensReleased].alive = true;
            aliens[aliensReleased].state = GOING_OUT;
            aliens[aliensReleased].lingerStart = 0;
            aliensReleased++;
        }
        alienReleaseFrame++;

        // --- Aliens ---
        int aliveAliens = 0;
        for (int i=0; i<MAX_ALIENS; i++) {
            if (!aliens[i].alive) continue;
            aliveAliens++;

            // ---- Erase previous position ----
            drawAlien(tft, aliens[i], true);

            // Alien logic
            if (aliens[i].state == GOING_OUT) {
                aliens[i].depth -= 0.01;
                if (aliens[i].depth <= 0.01) {
                    aliens[i].depth = 0.01;
                    aliens[i].state = AT_RIM;
                    aliens[i].lingerStart = millis();
                }
            } else if (aliens[i].state == AT_RIM) {
                if (aliens[i].seg == playerSeg) {
                    playerLives--;
                    drawLives(tft, playerLives);
                    tft.fillScreen(TFT_BLACK);
                    drawTunnel(tft);
                    tft.setTextColor(TFT_RED, TFT_BLACK);
                    tft.setTextFont(4);
                    tft.setTextDatum(MC_DATUM);
                    tft.drawString("They got Gordo!", TUNNEL_CENTER_X, TUNNEL_CENTER_Y);
                    delay(2000);
                    if (playerLives <= 0) {
                        tft.fillScreen(TFT_BLACK);
                        tft.setTextColor(TFT_RED, TFT_BLACK);
                        tft.setTextFont(4);
                        tft.setTextDatum(MC_DATUM);
                        tft.drawString("Game Over", TUNNEL_CENTER_X, TUNNEL_CENTER_Y);
                        delay(2500);
                        splashScreen(tft);
                        return;
                    }
                    superzapperUsed = false;
                    aliensReleased = 0;
                    alienReleaseFrame = 0;
                    tft.fillScreen(TFT_BLACK);
                    drawTunnel(tft);
                    drawScore(tft, playerScore);
                    drawLives(tft, playerLives);
                    drawLevel(tft, currentLevel);
                    initAliens(aliensPerLevel);
                    break;
                } else if (millis() - aliens[i].lingerStart > 1800) {
                    aliens[i].state = GOING_IN;
                }
            } else if (aliens[i].state == GOING_IN) {
                aliens[i].depth += 0.01;
                if (aliens[i].depth >= 1.0) {
                    aliens[i].depth = 1.0;
                    aliens[i].state = AT_CENTER;
                    aliens[i].lingerStart = millis();
                }
            } else if (aliens[i].state == AT_CENTER) {
                if (millis() - aliens[i].lingerStart > 800) {
                    aliens[i].state = GOING_OUT;
                }
            }

            if ((aliens[i].type == FLIPPER || aliens[i].type == PULSAR) && rand()%100 < 2 && aliens[i].state == GOING_OUT) {
                int dir = (rand()%2)*2-1;
                aliens[i].seg = (aliens[i].seg + dir + NUM_RIM_POINTS) % NUM_RIM_POINTS;
            }

            // ---- Draw at new position ----
            drawAlien(tft, aliens[i], false);

            // ---- Collision with carrot ----
            for (int j=0; j<MAX_SHOTS; j++) {
                if (!shots[j].active) continue;
                float a_r = TUNNEL_RADIUS - aliens[i].depth * TUNNEL_DEPTH;
                int a_x = TUNNEL_CENTER_X + cos(2*PI*(aliens[i].seg+0.5)/NUM_RIM_POINTS) * a_r;
                int a_y = TUNNEL_CENTER_Y + sin(2*PI*(aliens[i].seg+0.5)/NUM_RIM_POINTS) * a_r;
                float c_r = TUNNEL_RADIUS - shots[j].pos * TUNNEL_DEPTH;
                int c_x = TUNNEL_CENTER_X + cos(shots[j].angle) * c_r;
                int c_y = TUNNEL_CENTER_Y + sin(shots[j].angle) * c_r;
                if (abs(a_x - c_x) < 10 && abs(a_y - c_y) < 10) {
                    drawAlien(tft, aliens[i], true); // erase again
                    aliens[i].alive = false;
                    shots[j].active = false;
                    playerScore += 100;
                    tft.fillRect(0, 0, 40, 20, TFT_BLACK);
                    drawScore(tft, playerScore);
                }
            }
        }

        // --- Level up if all aliens dead ---
        if (aliveAliens == 0 && aliensReleased == aliensPerLevel) {
            tft.fillScreen(TFT_BLACK);
            drawTunnel(tft);
            drawScore(tft, playerScore);
            drawLives(tft, playerLives);
            tft.setTextColor(COLOR_LEVEL, TFT_BLACK);
            tft.setTextFont(4);
            tft.setTextDatum(MC_DATUM);
            char buf[16];
            snprintf(buf, sizeof(buf), "LEVEL %d", currentLevel+1);
            tft.drawString(buf, TUNNEL_CENTER_X, TUNNEL_CENTER_Y);
            delay(2500);

            currentLevel++;
            playerScore += 1000;
            if (playerLives < MAX_LIVES) playerLives++;
            superzapperUsed = false;
            aliensReleased = 0;
            alienReleaseFrame = 0;
            tft.fillScreen(TFT_BLACK);
            drawTunnel(tft);
            drawScore(tft, playerScore);
            drawLives(tft, playerLives);
            drawLevel(tft, currentLevel);
            aliensPerLevel = 10 + (currentLevel-1);
            initAliens(aliensPerLevel);
        }

for (int i=0; i<MAX_SHOTS; i++) {
    if (shots[i].active) {
        // Erase along the swept path from last_pos to current pos, in small steps
        float from = shots[i].last_pos;
        float to = shots[i].pos;
        if (from > to) { float tmp=from; from=to; to=tmp; }
        for (float p = from; p <= to; p += 0.01) {
            drawVectorCarrot(tft, shots[i].angle, p, true, nullptr, nullptr);
        }

        shots[i].last_pos = shots[i].pos;
        shots[i].last_angle = shots[i].angle;

        shots[i].pos += 0.06;

        if (shots[i].pos > 1.0) {
            // Scrub from last_pos to tip (center)
            for (float p = shots[i].last_pos; p <= 1.0; p += 0.01) {
                drawVectorCarrot(tft, shots[i].angle, p, true, nullptr, nullptr);
            }
            shots[i].active = false;
            shots[i].last_pos = -1.0;
            shots[i].last_angle = 0.0;
        } else {
            drawVectorCarrot(tft, shots[i].angle, shots[i].pos, false, &shots[i].last_x, &shots[i].last_y);
        }
    }
}


        delay(20);
    }
}

#endif // TEMPEST_GORDO_H