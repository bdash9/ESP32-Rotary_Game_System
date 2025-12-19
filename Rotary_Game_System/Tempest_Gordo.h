#ifndef TEMPEST_GORDO_H
#define TEMPEST_GORDO_H

#include <math.h>
#include <stdlib.h>

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

const uint16_t tunnelLevelColors[10] = {
    TFT_BLUE,      // 1. Circle
    TFT_RED,       // 2. Square
    TFT_GREEN,     // 3. Triangle
    TFT_YELLOW,    // 4. Star
    TFT_CYAN,      // 5. Heart
    TFT_MAGENTA,   // 6. Bowtie
    TFT_ORANGE,    // 7. U-shape (define as 0xFDA0 if not defined)
    TFT_PINK,      // 8. V-shape (define as 0xF81F if not defined)
    TFT_DARKGREY,  // 9. Steps
    TFT_WHITE      // 10. Infinity
};

// --- 10 geometric tunnel shapes (normalized 0..1) ---
// --- 10 geometric tunnel shapes (normalized 0..1) ---
const float tunnelShapes[10][NUM_RIM_POINTS][2] = {
// 1. Circle
{
    {1.0,0.5},
    {0.9808,0.6929},
    {0.9239,0.8536},
    {0.8315,0.9808},
    {0.7071,1.0},
    {0.5556,0.9808},
    {0.5,0.9239},
    {0.269,0.8315},
    {0.1464,0.7071},
    {0.0192,0.5556},
    {0.0,0.5},
    {0.0192,0.4444},
    {0.1464,0.2929},
    {0.269,0.1685},
    {0.5,0.0761},
    {0.7071,0.0}
},
// 2. Square
{
    {0.8,0.2},{0.8,0.35},{0.8,0.5},{0.8,0.65},
    {0.8,0.8},{0.65,0.8},{0.5,0.8},{0.35,0.8},
    {0.2,0.8},{0.2,0.65},{0.2,0.5},{0.2,0.35},
    {0.2,0.2},{0.35,0.2},{0.5,0.2},{0.65,0.2}
},
// 3. Triangle (use your improved version if you already changed it)
{
    {0.50, 0.10},
    {0.57, 0.23},
    {0.64, 0.36},
    {0.71, 0.49},
    {0.78, 0.62},
    {0.85, 0.75},
    {0.72, 0.80},
    {0.59, 0.85},
    {0.46, 0.90},
    {0.33, 0.85},
    {0.20, 0.80},
    {0.23, 0.67},
    {0.26, 0.54},
    {0.29, 0.41},
    {0.32, 0.28},
    {0.35, 0.15}
},
// 4. Star (unchanged)
{
    {0.5,0.07},{0.62,0.38},{0.95,0.38},{0.68,0.57},
    {0.78,0.9},{0.5,0.7},{0.22,0.9},{0.32,0.57},
    {0.05,0.38},{0.38,0.38},{0.5,0.07},{0.62,0.38},
    {0.95,0.38},{0.68,0.57},{0.78,0.9},{0.5,0.7}
},

// 5. Heart – clean perimeter, no internal chord
{
    // Go around the heart outline counter‑clockwise
    {0.25, 0.40},  // left mid
    {0.22, 0.48},
    {0.22, 0.56},
    {0.25, 0.64},
    {0.32, 0.74},
    {0.40, 0.82},
    {0.50, 0.88},  // bottom tip
    {0.60, 0.82},
    {0.68, 0.74},
    {0.75, 0.64},
    {0.78, 0.56},
    {0.78, 0.48},
    {0.75, 0.40},  // right upper
    {0.68, 0.30},
    {0.60, 0.24},
    {0.50, 0.22}   // near the top indentation
},

// 6. Bowtie – two connected triangles (hourglass)
{
    // Top edge, left to right
    {0.30, 0.20},
    {0.40, 0.20},
    {0.50, 0.20},
    {0.60, 0.20},
    {0.70, 0.20},

    // Upper right down to center pinch
    {0.72, 0.32},
    {0.64, 0.44},
    {0.56, 0.52},

    // Center pinch to lower right
    {0.64, 0.60},
    {0.72, 0.72},
    {0.70, 0.80},
    {0.60, 0.80},

    // Bottom edge back to left
    {0.50, 0.80},
    {0.40, 0.80},
    {0.30, 0.80},

    // Lower left up to left pinch (completes two‑triangle hourglass)
    {0.36, 0.64}
},

// 7. U-shape – no top connecting segment
{
    // Left vertical of U (top to bottom)
    {0.25, 0.20},
    {0.25, 0.30},
    {0.25, 0.40},
    {0.25, 0.50},
    {0.25, 0.60},

    // Bottom curve of U (left to right)
    {0.30, 0.72},
    {0.40, 0.80},
    {0.50, 0.82},
    {0.60, 0.80},
    {0.70, 0.72},

    // Right vertical of U (bottom to top)
    {0.75, 0.60},
    {0.75, 0.50},
    {0.75, 0.40},
    {0.75, 0.30},
    {0.75, 0.20},

    // Duplicate the left‑top point to avoid a top bar:
    // segment 15->0 has zero length, so no visible line.
    {0.25, 0.20}
},

// 8. V-shape – (use your cleaned-up version if already changed)
{
    {0.25, 0.20},
    {0.30, 0.32},
    {0.35, 0.44},
    {0.40, 0.56},
    {0.45, 0.68},
    {0.48, 0.80},
    {0.50, 0.90},
    {0.52, 0.80},
    {0.55, 0.68},
    {0.60, 0.56},
    {0.65, 0.44},
    {0.70, 0.32},
    {0.75, 0.20},
    {0.65, 0.18},
    {0.55, 0.16},
    {0.45, 0.16}
},

// 9. Steps (unchanged)
{
    {0.2,0.2},{0.2,0.35},{0.35,0.35},{0.35,0.5},
    {0.5,0.5},{0.5,0.65},{0.65,0.65},{0.65,0.8},
    {0.8,0.8},{0.8,0.65},{0.65,0.65},{0.65,0.5},
    {0.5,0.5},{0.5,0.35},{0.35,0.35},{0.35,0.2}
},
// 10. Infinity (unchanged)
{
    {0.35,0.5},{0.3,0.35},{0.35,0.2},{0.5,0.2},
    {0.65,0.2},{0.7,0.35},{0.65,0.5},{0.5,0.5},
    {0.35,0.5},{0.3,0.65},{0.35,0.8},{0.5,0.8},
    {0.65,0.8},{0.7,0.65},{0.65,0.5},{0.5,0.5}
}
};
const int numTunnelShapes = 10;

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
  int   seg;          // which tunnel segment this carrot is in
  float pos;          // 0.0 = at rim, 1.0 = at center
  float last_pos;
  float last_base_pos;
  int   last_x, last_y;
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
extern int currentLevel;

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

void computeTunnelGeometry(int level) {
    int shapeIdx = (level-1) % numTunnelShapes;

    if (shapeIdx == 0) {
        // Level 1: true circle (unchanged)
        for (int i=0; i<NUM_RIM_POINTS; i++) {
            float angle = 2 * M_PI * i / NUM_RIM_POINTS;
            rim[i].x = TUNNEL_CENTER_X + cos(angle) * TUNNEL_RADIUS;
            rim[i].y = TUNNEL_CENTER_Y + sin(angle) * TUNNEL_RADIUS;
            // Inner at center (radial tunnel)
            inner[i].x = TUNNEL_CENTER_X;
            inner[i].y = TUNNEL_CENTER_Y;
        }
    } else {
        // Normalize shape so its furthest point has radius ~= TUNNEL_RADIUS
        float maxR = 0.0f;
        float dx0[NUM_RIM_POINTS], dy0[NUM_RIM_POINTS];

        for (int i=0; i<NUM_RIM_POINTS; i++) {
            float nx = tunnelShapes[shapeIdx][i][0];
            float ny = tunnelShapes[shapeIdx][i][1];
            dx0[i] = nx - 0.5f;
            dy0[i] = ny - 0.5f;
            float r = sqrtf(dx0[i]*dx0[i] + dy0[i]*dy0[i]);
            if (r > maxR) maxR = r;
        }
        if (maxR < 1e-3f) maxR = 1.0f;   // safety

        // Scale so max radius == TUNNEL_RADIUS
        for (int i=0; i<NUM_RIM_POINTS; i++) {
            float dx = dx0[i] / maxR;   // normalized to radius 1
            float dy = dy0[i] / maxR;
            rim[i].x = (int)(TUNNEL_CENTER_X + dx * TUNNEL_RADIUS);
            rim[i].y = (int)(TUNNEL_CENTER_Y + dy * TUNNEL_RADIUS);

            // Inner stays at center = true radial corridors
            inner[i].x = TUNNEL_CENTER_X;
            inner[i].y = TUNNEL_CENTER_Y;
        }
    }
}

// seg: 0..NUM_RIM_POINTS-1 : corridor between i and i+1
// depth: 0.0 = rim, 1.0 = center
void getCorridorPoint(int seg, float depth, int &x, int &y) {
    int i0 = seg;
    int i1 = (seg + 1) % NUM_RIM_POINTS;

    // Wall 0: rim[i0] -> inner[i0]
    float w0x = rim[i0].x * (1.0f - depth) + inner[i0].x * depth;
    float w0y = rim[i0].y * (1.0f - depth) + inner[i0].y * depth;

    // Wall 1: rim[i1] -> inner[i1]
    float w1x = rim[i1].x * (1.0f - depth) + inner[i1].x * depth;
    float w1y = rim[i1].y * (1.0f - depth) + inner[i1].y * depth;

    // Corridor center is midpoint between the two walls
    x = (int)((w0x + w1x) * 0.5f);
    y = (int)((w0y + w1y) * 0.5f);
}

void drawTunnel(TFT_eSPI &tft, int level) {
    int shapeIdx = (level - 1) % numTunnelShapes;
    uint16_t tunnelColor = tunnelLevelColors[shapeIdx];

    // 1. Outer rim
    for (int i = 0; i < NUM_RIM_POINTS; i++) {
        int j = (i + 1) % NUM_RIM_POINTS;
        tft.drawLine(rim[i].x, rim[i].y, rim[j].x, rim[j].y, tunnelColor);
    }

    // 2. Corridor walls (all shapes, including heart)
    for (int i = 0; i < NUM_RIM_POINTS; i++) {
        tft.drawLine(rim[i].x,   rim[i].y,
                     inner[i].x, inner[i].y,
                     tunnelColor);
    }

    // 3. Inner polygon (optional; with inner at center it’s just a dot)
    /*
    for (int i = 0; i < NUM_RIM_POINTS; i++) {
        int j = (i + 1) % NUM_RIM_POINTS;
        tft.drawLine(inner[i].x, inner[i].y,
                     inner[j].x, inner[j].y,
                     tunnelColor);
    }
    */
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
    // Center of the corridor at the rim
    getCorridorPoint(segIdx, 0.0f, x, y);
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

// seg: corridor index (0..NUM_RIM_POINTS-1)
// pos: 0.0 = at rim, 1.0 = at center
void drawVectorCarrot(TFT_eSPI &tft,
                      int seg,
                      float pos,
                      bool erase /*= false*/,
                      int *last_x /*= nullptr*/,
                      int *last_y /*= nullptr*/)
{
    // Tip position along the corridor centerline
    int tipX, tipY;
    getCorridorPoint(seg, pos, tipX, tipY);

    // Base is a bit closer to the rim than the tip
    float bodyFrac = 0.12f;          // length of carrot along corridor
    float basePos  = pos - bodyFrac;
    if (basePos < 0.0f) basePos = 0.0f;

    int baseX, baseY;
    getCorridorPoint(seg, basePos, baseX, baseY);

    // Direction from base to tip (unit vector)
    float dx = (float)tipX - (float)baseX;
    float dy = (float)tipY - (float)baseY;
    float len = sqrtf(dx*dx + dy*dy);
    float ux = 0.0f, uy = 0.0f;
    if (len > 1e-3f) {
        ux = dx / len;
        uy = dy / len;
    }

    // Perpendicular for width
    float vx = -uy;
    float vy =  ux;

    uint16_t carrot = erase ? TFT_BLACK : COLOR_CARROT;
    uint16_t green  = erase ? TFT_BLACK : COLOR_CARROT_TOP;

    // Carrot width
    float bodyWid = 3.0f;

    // Side bases for the carrot body
    float bx1 = (float)baseX + vx * bodyWid;
    float by1 = (float)baseY + vy * bodyWid;
    float bx2 = (float)baseX - vx * bodyWid;
    float by2 = (float)baseY - vy * bodyWid;

    // Main carrot body: two lines from base sides to tip
    tft.drawLine((int)bx1, (int)by1, tipX, tipY, carrot);
    tft.drawLine((int)bx2, (int)by2, tipX, tipY, carrot);

    // Cross "shading" lines
    for (int i = 1; i < 3; i++) {
        float frac = i / 3.0f;
        float cx1 = bx1 + frac * ((float)tipX - bx1);
        float cy1 = by1 + frac * ((float)tipY - by1);
        float cx2 = bx2 + frac * ((float)tipX - bx2);
        float cy2 = by2 + frac * ((float)tipY - by2);
        tft.drawLine((int)cx1, (int)cy1, (int)cx2, (int)cy2, carrot);
    }

    // Tip "V" using direction and perpendicular
    float tipSpan = 3.0f;
    float tlx = (float)tipX + ( vx * tipSpan + ux * tipSpan * 0.3f);
    float tly = (float)tipY + ( vy * tipSpan + uy * tipSpan * 0.3f);
    float trx = (float)tipX + (-vx * tipSpan + ux * tipSpan * 0.3f);
    float try_ = (float)tipY + (-vy * tipSpan + uy * tipSpan * 0.3f);
    tft.drawLine(tipX, tipY, (int)tlx, (int)tly, carrot);
    tft.drawLine(tipX, tipY, (int)trx, (int)try_, carrot);

    // Green top at the base, fanned a bit
    float leafLen = 8.0f;
    for (int k = -1; k <= 1; k++) {
        float ang = 0.4f * k;  // fan angle
        float cx =  ux * cosf(ang) - vx * sinf(ang);
        float cy =  uy * cosf(ang) - vy * sinf(ang);
        float lx = (float)baseX - cx * leafLen;  // point a bit toward rim
        float ly = (float)baseY - cy * leafLen;
        tft.drawLine(baseX, baseY, (int)lx, (int)ly, green);
    }
    tft.fillRect(baseX - 1, baseY - 1, 3, 3, green);

    if (!erase && last_x && last_y) {
        *last_x = tipX;
        *last_y = tipY;
    }
}

void drawSegmentFull(TFT_eSPI &tft, int segIdx, uint16_t color);

void drawAlien(TFT_eSPI &tft, Alien &a, bool erase=false) {
int cx, cy;
getCorridorPoint(a.seg, a.depth, cx, cy);

    float sizeFactor = 1.0 - a.depth;
    int minSize = 1, maxSize = 7;
    int size = minSize + (int)((maxSize - minSize) * pow(sizeFactor, 2.0));
    if (size < 1) size = 1;

    int eraseMargin = 3 + (size/2);
    int eraseRadius = size + eraseMargin;

if (erase) {
    // Erase the alien sprite
    tft.fillCircle(cx, cy, eraseRadius, TFT_BLACK);

    // Restore the tunnel graphics for this corridor segment
    drawSegmentFull(
        tft,
        a.seg,
        tunnelLevelColors[(currentLevel - 1) % 10]
    );

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
    int next = (segIdx + 1) % NUM_RIM_POINTS;

    // Rim edge for this segment
    tft.drawLine(rim[segIdx].x, rim[segIdx].y,
                 rim[next].x,   rim[next].y,
                 color);

    // Both radial walls for this segment
    tft.drawLine(rim[segIdx].x,   rim[segIdx].y,
                 inner[segIdx].x, inner[segIdx].y,
                 color);
    tft.drawLine(rim[next].x,   rim[next].y,
                 inner[next].x, inner[next].y,
                 color);
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
    tft.fillScreen(TFT_BLACK);
    computeTunnelGeometry(currentLevel);
drawTunnel(tft, currentLevel);
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
            // After (correct, uses the current level color)
drawSegmentFull(tft, prev_active_seg, tunnelLevelColors[(currentLevel-1)%10]);
            drawSegmentFull(tft, active_seg, COLOR_SEGMENT_ACTIVE);
            prev_active_seg = active_seg;
        }

// Super Zapper (short press) and Exit to Menu (long press ≥ 3s)
static unsigned long pushPressStart = 0;

int push = digitalRead(PIN_PUSH);

// Button just pressed
if (push == LOW && lastPush == HIGH) {
    pushPressStart = millis();
}

// Button just released
if (push == HIGH && lastPush == LOW) {
    unsigned long held = millis() - pushPressStart;

    if (held >= 3000) {
        // Long press: exit back to main menu
        tft.fillScreen(TFT_BLACK);
        return;   // leave run_Tempest_Gordo()
    } else if (!superzapperUsed) {
        // Short press: Super Zapper
        for (int i = 0; i < MAX_ALIENS; i++) aliens[i].alive = false;
        superzapperUsed = true;

        tft.fillScreen(TFT_BLACK);
        computeTunnelGeometry(currentLevel);
        drawTunnel(tft, currentLevel);
        superZapperEffect(tft);
        for (int i = 0; i < MAX_ALIENS; i++)
            if (aliens[i].alive) drawAlien(tft, aliens[i], false);
        delay(600);
        tft.fillScreen(TFT_BLACK);
        computeTunnelGeometry(currentLevel);
        drawTunnel(tft, currentLevel);
        drawScore(tft, playerScore);
        drawLives(tft, playerLives);
        drawLevel(tft, currentLevel);
    }
}

lastPush = push;

        int button = digitalRead(PIN_KO);
if (button == LOW && lastButton == HIGH) {
    for (int i=0; i<MAX_SHOTS; i++) {
        if (!shots[i].active) {
            shots[i].active      = true;
            shots[i].seg         = playerSeg; // bind to current segment
            shots[i].pos         = 0.0f;
            shots[i].last_pos    = 0.0f;
            shots[i].last_base_pos = 0.0f;
            shots[i].last_x      = -1;
            shots[i].last_y      = -1;
            break;
        }
    }
}
        lastButton = button;

static int prevBunnySeg = -1;
if (prevBunnySeg != -1 && prevBunnySeg != playerSeg) {
    // Erase old Gordo
    drawGordo(tft, prevBunnySeg, true);
    // Restore the tunnel graphics for that segment
    drawSegmentFull(tft,
                    prevBunnySeg,
                    tunnelLevelColors[(currentLevel - 1) % 10]);
}

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
                                computeTunnelGeometry(currentLevel);
drawTunnel(tft, currentLevel);
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
                                computeTunnelGeometry(currentLevel);
drawTunnel(tft, currentLevel);
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
// Alien position using rim/inner interpolation
int a_x = (int)(rim[aliens[i].seg].x   * (1.0f - aliens[i].depth) +
                inner[aliens[i].seg].x * aliens[i].depth);
int a_y = (int)(rim[aliens[i].seg].y   * (1.0f - aliens[i].depth) +
                inner[aliens[i].seg].y * aliens[i].depth);

// Carrot tip using same interpolation
int c_x = (int)(rim[shots[j].seg].x   * (1.0f - shots[j].pos) +
                inner[shots[j].seg].x * shots[j].pos);
int c_y = (int)(rim[shots[j].seg].y   * (1.0f - shots[j].pos) +
                inner[shots[j].seg].y * shots[j].pos);
if (abs(a_x - c_x) < 10 && abs(a_y - c_y) < 10) {
    // Erase alien
    drawAlien(tft, aliens[i], true);
    aliens[i].alive = false;

    // Erase the carrot that hit it
    drawVectorCarrot(tft, shots[j].seg, shots[j].pos, true, nullptr, nullptr);

    // Restore the tunnel graphics for that corridor segment
    drawSegmentFull(
        tft,
        shots[j].seg,
        tunnelLevelColors[(currentLevel - 1) % 10]
    );

    // Deactivate the shot
    shots[j].active = false;

    // Update score display
    playerScore += 100;
    tft.fillRect(0, 0, 40, 20, TFT_BLACK);
    drawScore(tft, playerScore);
}
            }
        }

        // --- Level up if all aliens dead ---
        if (aliveAliens == 0 && aliensReleased == aliensPerLevel) {
            tft.fillScreen(TFT_BLACK);
                        computeTunnelGeometry(currentLevel);
drawTunnel(tft, currentLevel);
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
                        computeTunnelGeometry(currentLevel);
drawTunnel(tft, currentLevel);
            drawScore(tft, playerScore);
            drawLives(tft, playerLives);
            drawLevel(tft, currentLevel);
            aliensPerLevel = 10 + (currentLevel-1);
            initAliens(aliensPerLevel);
        }

for (int i = 0; i < MAX_SHOTS; i++) {
    if (!shots[i].active) continue;

    // 1) Erase carrot at its current position
    drawVectorCarrot(tft, shots[i].seg, shots[i].pos, true, nullptr, nullptr);

    // Restore the tunnel graphics for this segment (walls + rim)
    drawSegmentFull(
        tft,
        shots[i].seg,
        tunnelLevelColors[(currentLevel - 1) % 10]
    );

    // 2) Advance the shot
    shots[i].pos += 0.06f;

    // 3) If it has gone past the center, stop it
    if (shots[i].pos > 1.0f) {
        shots[i].active = false;
        continue;
    }

    // 4) Draw carrot at the new position
    drawVectorCarrot(tft, shots[i].seg, shots[i].pos, false,
                     &shots[i].last_x, &shots[i].last_y);
}


        delay(20);
    }
}

#endif // TEMPEST_GORDO_H