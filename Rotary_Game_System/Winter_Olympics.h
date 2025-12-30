#ifndef WINTER_OLYMPICS_H
#define WINTER_OLYMPICS_H

#include <Arduino.h>
#include <TFT_eSPI.h>

extern void playSound(const char *path, bool stopCurrent);
extern void updateAudio();
extern volatile int rotaryPos;

#define SCREEN_W 320
#define SCREEN_H 240

// Pin definitions (from main .ino)
#ifndef PIN_KO
#define PIN_KO 22
#endif
#ifndef PIN_PUSH
#define PIN_PUSH 22
#endif

// Olympic Ring Colors
#define OLYMPIC_BLUE 0x001F
#define OLYMPIC_YELLOW 0xFFE0
#define OLYMPIC_BLACK 0x0000
#define OLYMPIC_GREEN 0x07E0
#define OLYMPIC_RED 0xF800

// Skiing game constants
#define NUM_SKI_LANES 5
#define MAX_GATES 15
#define GATE_SPACING 80
#define SKI_SPEED_BASE 1.5f

// Colors
#define COLOR_SNOW 0xFFFF
#define COLOR_SKY 0x5D9F  // Light blue
#define COLOR_TREE_TRUNK 0x4208
#define COLOR_TREE_GREEN 0x2444
#define COLOR_GATE_RED TFT_RED
#define COLOR_GATE_BLUE TFT_BLUE
#define COLOR_FINISH_LINE TFT_RED

// Game state
enum WO_GameMode {
    WO_GAME_SPLASH,
    WO_GAME_MENU,
    WO_GAME_SKIING,
    WO_GAME_SKI_JUMP,
    WO_GAME_LUGE
};

// Menu items
const char* wo_gameMenuTitles[] = {
    "Downhill Skiing",
    "Ski Jump",
    "Luge"
};
const int WO_NUM_OLYMPIC_GAMES = 3;

// Gate structure
struct WO_Gate {
    float z;           // Distance from player (0 = at player, 1 = far)
    int lane;          // Which lane (0-4, where 2 is center)
    bool active;
    bool passed;
    bool hit;          // Hit the gate vs. passed through cleanly
};

WO_Gate wo_gates[MAX_GATES];

// Player state
int wo_playerLane = 2;  // Center lane
int wo_lastPlayerLane = 2;
float wo_playerZ = 0.0f;
int wo_skiScore = 0;
float wo_skiSpeed = SKI_SPEED_BASE;
bool wo_gameActive = false;
unsigned long wo_startTime = 0;

// Perspective lane data for skiing
struct WO_SkiLane {
    int topX, topY;
    int bottomX, bottomY;
};
WO_SkiLane wo_skiLanes[NUM_SKI_LANES];

//=============================================================================
// VECTOR FONT FOR SPLASH SCREENS
//=============================================================================
struct WO_VecStroke { int x0, y0, x1, y1; };
struct WO_VecLetter { const WO_VecStroke* strokes; int n; int w; };

const WO_VecStroke wo_strokes_R[] = {{0,12,0,0},{0,0,8,0},{8,0,8,6},{8,6,0,6},{0,6,8,12}};
const WO_VecStroke wo_strokes_O[] = {{0,0,8,0},{8,0,8,12},{8,12,0,12},{0,12,0,0}};
const WO_VecStroke wo_strokes_T[] = {{4,0,4,12},{0,0,8,0}};
const WO_VecStroke wo_strokes_A[] = {{0,12,4,0},{4,0,8,12},{2,6,6,6}};
const WO_VecStroke wo_strokes_Y[] = {{0,0,4,6},{8,0,4,6},{4,6,4,12}};
const WO_VecStroke wo_strokes_L[] = {{0,0,0,12},{0,12,8,12}};
const WO_VecStroke wo_strokes_M[] = {{0,12,0,0},{0,0,4,6},{4,6,8,0},{8,0,8,12}};
const WO_VecStroke wo_strokes_P[] = {{0,12,0,0},{0,0,8,0},{8,0,8,6},{8,6,0,6}};
const WO_VecStroke wo_strokes_I[] = {{4,0,4,12},{2,0,6,0},{2,12,6,12}}; 
const WO_VecStroke wo_strokes_C[] = {{8,0,0,0},{0,0,0,12},{0,12,8,12}};
const WO_VecStroke wo_strokes_S[] = {{8,0,0,0},{0,0,0,6},{0,6,8,6},{8,6,8,12},{8,12,0,12}};
const WO_VecStroke wo_strokes_E[] = {{8,0,0,0},{0,0,0,12},{0,6,6,6},{0,12,8,12}};
const WO_VecStroke wo_strokes_G[] = {{8,0,0,0},{0,0,0,12},{0,12,8,12},{8,12,8,8},{8,8,5,8}};
const WO_VecStroke wo_strokes_W[] = {{0,0,0,12},{0,12,4,6},{4,6,8,12},{8,12,8,0}};
const WO_VecStroke wo_strokes_N[] = {{0,12,0,0},{0,0,8,12},{8,12,8,0}};

const WO_VecLetter wo_vecFont[] = {
    {wo_strokes_R,5,9}, {wo_strokes_O,4,9}, {wo_strokes_T,2,9}, {wo_strokes_A,3,9},
    {wo_strokes_Y,3,9}, {wo_strokes_L,2,9}, {wo_strokes_M,4,9}, {wo_strokes_P,4,9},
    {wo_strokes_I,3,9}, {wo_strokes_C,3,9}, {wo_strokes_S,5,9}, {wo_strokes_E,4,9}, 
    {wo_strokes_G,5,9}, {wo_strokes_W,4,9}, {wo_strokes_N,3,9}
};

int wo_getVecFontIdx(char c) {
    switch(c) {
        case 'R': return 0; case 'O': return 1; case 'T': return 2; case 'A': return 3;
        case 'Y': return 4; case 'L': return 5; case 'M': return 6; case 'P': return 7;
        case 'I': return 8; case 'C': return 9; case 'S': return 10; case 'E': return 11;
        case 'G': return 12; case 'W': return 13; case 'N': return 14;
        default: return -1;
    }
}

void wo_drawVecLetter(TFT_eSPI &tft, char c, int x, int y, int scale, uint16_t color) {
    int idx = wo_getVecFontIdx(c);
    if(idx < 0) return;
    for (int i = 0; i < wo_vecFont[idx].n; i++) {
        int x0 = x + wo_vecFont[idx].strokes[i].x0 * scale;
        int y0 = y + wo_vecFont[idx].strokes[i].y0 * scale;
        int x1 = x + wo_vecFont[idx].strokes[i].x1 * scale;
        int y1 = y + wo_vecFont[idx].strokes[i].y1 * scale;
        tft.drawLine(x0, y0, x1, y1, color);
    }
}

void wo_drawVecText(TFT_eSPI &tft, const char* text, int x, int y, int scale, uint16_t color) {
    int spacing = 10 * scale;
    int cx = x;
    for (int i = 0; text[i]; i++) {
        if (text[i] != ' ') {
            wo_drawVecLetter(tft, text[i], cx, y, scale, color);
            wo_drawVecLetter(tft, text[i], cx + 1, y, scale, color);  // Bold
        }
        cx += spacing;
    }
}

int wo_vecTextWidth(const char* text, int scale) {
    return strlen(text) * 10 * scale;
}

//=============================================================================
// OLYMPIC RINGS DRAWING
//=============================================================================
void wo_drawOlympicRing(TFT_eSPI &tft, int cx, int cy, int radius, uint16_t color) {
    // Draw thick ring
    for (int i = 0; i < 4; i++) {
        tft.drawCircle(cx, cy, radius + i, color);
    }
}

void wo_drawOlympicRings(TFT_eSPI &tft, int centerX, int centerY, int ringRadius) {
    int spacing = ringRadius * 2 + 8;
    int vertOffset = ringRadius / 2;
    
    // Top row: Blue, Black, Red
    wo_drawOlympicRing(tft, centerX - spacing, centerY, ringRadius, OLYMPIC_BLUE);
    wo_drawOlympicRing(tft, centerX, centerY, ringRadius, OLYMPIC_BLACK);
    wo_drawOlympicRing(tft, centerX + spacing, centerY, ringRadius, OLYMPIC_RED);
    
    // Bottom row: Yellow, Green
    wo_drawOlympicRing(tft, centerX - spacing/2, centerY + vertOffset, ringRadius, OLYMPIC_YELLOW);
    wo_drawOlympicRing(tft, centerX + spacing/2, centerY + vertOffset, ringRadius, OLYMPIC_GREEN);
}

void wo_drawMiniOlympicRings(TFT_eSPI &tft, int x, int y) {
    int r = 8;
    int spacing = r * 2 + 3;
    int vOff = r / 2;
    
    // Top row
    wo_drawOlympicRing(tft, x, y, r, OLYMPIC_BLUE);
    wo_drawOlympicRing(tft, x + spacing, y, r, OLYMPIC_BLACK);
    wo_drawOlympicRing(tft, x + spacing * 2, y, r, OLYMPIC_RED);
    
    // Bottom row
    wo_drawOlympicRing(tft, x + spacing/2, y + vOff, r, OLYMPIC_YELLOW);
    wo_drawOlympicRing(tft, x + spacing + spacing/2, y + vOff, r, OLYMPIC_GREEN);
}

//=============================================================================
// SPLASH SCREEN
//=============================================================================
void wo_showSplashScreen(TFT_eSPI &tft) {
    // Wait for button release
    while (digitalRead(PIN_KO) == LOW) delay(10);
    delay(300);
    
    tft.fillScreen(COLOR_SKY);
    
    // Start Olympics theme music
    playSound("/sounds/Olympics_Basic_Theme_and_Fanfare.wav", true);
    
    // "WINTER OLYMPICS" title
    const char* title1 = "WINTER";
    const char* title2 = "OLYMPICS";
    int scale = 4;
    int title1W = wo_vecTextWidth(title1, scale);
    int title2W = wo_vecTextWidth(title2, scale);
    
    wo_drawVecText(tft, title1, (SCREEN_W - title1W) / 2, 30, scale, TFT_WHITE);
    wo_drawVecText(tft, title2, (SCREEN_W - title2W) / 2, 80, scale, TFT_WHITE);
    
    // Olympic rings - moved down
    wo_drawOlympicRings(tft, SCREEN_W / 2, 165, 18);
    
    // Blinking prompt
    bool textVisible = true;
    unsigned long lastBlink = millis();
    int lastBtn = HIGH;
    bool buttonPressed = false;
    
    while (!buttonPressed) {
        // Keep music playing
        updateAudio();
        
        if (millis() - lastBlink > 500) {
            lastBlink = millis();
            textVisible = !textVisible;
            
            if (textVisible) {
                tft.setTextColor(TFT_YELLOW, COLOR_SKY);
                tft.setTextFont(2);
                tft.setTextDatum(BC_DATUM);
                tft.drawString("Press any button to continue", SCREEN_W/2, SCREEN_H - 10);
            } else {
                tft.fillRect(0, SCREEN_H - 25, SCREEN_W, 25, COLOR_SKY);
            }
        }
        
        int btn = digitalRead(PIN_KO);
        if (btn == LOW && lastBtn == HIGH) {
            buttonPressed = true;
        }
        lastBtn = btn;
        delay(50);
    }
    
    while (digitalRead(PIN_KO) == LOW) delay(10);
    delay(400);
    
    // DON'T stop music here - let it continue to menu
}

//=============================================================================
// GAME MENU
//=============================================================================
int wo_showGameMenu(TFT_eSPI &tft) {
    tft.fillScreen(TFT_BLACK);
    
    // Title - draw with Olympic colors (NO BLACK), one letter at a time
    const char* title = "SELECT GAME";
    int scale = 3;
    int spacing = 10 * scale;
    
    // Center the entire title
    int titleWidth = strlen(title) * spacing;
    int startX = (SCREEN_W - titleWidth) / 2;
    
    // Olympic ring colors array (without black)
    uint16_t olympicColors[4] = {OLYMPIC_BLUE, OLYMPIC_YELLOW, OLYMPIC_GREEN, OLYMPIC_RED};
    
    int cx = startX;
    int colorIndex = 0;
    for (int i = 0; title[i]; i++) {
        if (title[i] != ' ') {
            wo_drawVecLetter(tft, title[i], cx, 15, scale, olympicColors[colorIndex % 4]);
            wo_drawVecLetter(tft, title[i], cx + 1, 15, scale, olympicColors[colorIndex % 4]);  // Bold
            colorIndex++;
        }
        cx += spacing;
    }
    
    int selectedIndex = 0;
    int lastSelectedIndex = -1;
    int lastRotary = rotaryPos;
    int lastBtn = HIGH;
    
    while (true) {
        if (selectedIndex != lastSelectedIndex) {
            // Clear menu area
            tft.fillRect(0, 70, SCREEN_W, SCREEN_H - 70, TFT_BLACK);
            
            // Draw menu items with LARGER font
            for (int i = 0; i < WO_NUM_OLYMPIC_GAMES; i++) {
                int yPos = 85 + i * 50;
                
                if (i == selectedIndex) {
                    tft.fillRoundRect(20, yPos - 8, SCREEN_W - 40, 40, 8, TFT_WHITE);
                    tft.setTextColor(TFT_BLUE, TFT_WHITE);
                } else {
                    tft.fillRoundRect(20, yPos - 8, SCREEN_W - 40, 40, 8, TFT_DARKGREY);
                    tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
                }
                
                tft.setTextFont(4);  // Changed from 2 to 4 for larger font
                tft.setTextDatum(MC_DATUM);
                tft.drawString(wo_gameMenuTitles[i], SCREEN_W/2, yPos + 12);
            }
            
            lastSelectedIndex = selectedIndex;
        }
        
        // Handle rotary
        int rotDiff = rotaryPos - lastRotary;
        if (rotDiff > 2) {
            selectedIndex++;
            if (selectedIndex >= WO_NUM_OLYMPIC_GAMES) selectedIndex = WO_NUM_OLYMPIC_GAMES - 1;
            lastRotary = rotaryPos;
        } else if (rotDiff < -2) {
            selectedIndex--;
            if (selectedIndex < 0) selectedIndex = 0;
            lastRotary = rotaryPos;
        }
        
        // Handle button
        int btn = digitalRead(PIN_KO);
        if (btn == LOW && lastBtn == HIGH) {
            while (digitalRead(PIN_KO) == LOW) delay(10);
            delay(200);
            return selectedIndex;
        }
        lastBtn = btn;
        updateAudio(); 
        delay(50);
    }
}

//=============================================================================
// SKIING GAME - PERSPECTIVE & RENDERING
//=============================================================================
void ski_initLanes() {
    int vanishX = SCREEN_W / 2;
    int vanishY = 20;
    int bottomY = SCREEN_H - 30;
    int bottomSpacing = 35;
    
    for (int i = 0; i < NUM_SKI_LANES; i++) {
        wo_skiLanes[i].topX = vanishX;
        wo_skiLanes[i].topY = vanishY;
        wo_skiLanes[i].bottomX = vanishX + (i - 2) * bottomSpacing;
        wo_skiLanes[i].bottomY = bottomY;
    }
}

void ski_getLanePosition(int lane, float z, int &x, int &y) {
    x = wo_skiLanes[lane].topX + (wo_skiLanes[lane].bottomX - wo_skiLanes[lane].topX) * (1.0f - z);
    y = wo_skiLanes[lane].topY + (wo_skiLanes[lane].bottomY - wo_skiLanes[lane].topY) * (1.0f - z);
}

void ski_drawPineTree(TFT_eSPI &tft, int x, int y, int size) {
    // Draw trunk
    int trunkWidth = size / 5;
    int trunkHeight = size / 2;
    tft.fillRect(x - trunkWidth/2, y, trunkWidth, trunkHeight, COLOR_TREE_TRUNK);
    
    // Draw 3 layers of triangular foliage for pine tree look
    int layerHeight = size / 3;
    int topY = y - size / 3;
    
    // Bottom layer (widest)
    tft.fillTriangle(x, topY, 
                     x - size/2, topY + layerHeight,
                     x + size/2, topY + layerHeight, COLOR_TREE_GREEN);
    
    // Middle layer
    topY += layerHeight / 2;
    tft.fillTriangle(x, topY, 
                     x - size/3, topY + layerHeight,
                     x + size/3, topY + layerHeight, COLOR_TREE_GREEN);
    
    // Top layer (smallest)
    topY += layerHeight / 2;
    tft.fillTriangle(x, topY, 
                     x - size/4, topY + layerHeight,
                     x + size/4, topY + layerHeight, COLOR_TREE_GREEN);
}

void ski_drawMountainOnce(TFT_eSPI &tft) {
    // Sky gradient (draw once at start)
    tft.fillScreen(COLOR_SKY);
    
    // Draw snow surface with perspective
    int horizonY = 20;
    for (int y = horizonY; y < SCREEN_H; y++) {
        float t = (float)(y - horizonY) / (SCREEN_H - horizonY);
        uint16_t snowShade = tft.color565(255, 255, 255 - (int)(t * 40));
        tft.drawFastHLine(0, y, SCREEN_W, snowShade);
    }
    
    // Draw BIGGER pine trees on the sides
    for (int i = 0; i < 6; i++) {
        float z = 0.3f + (i * 0.12f);
        int treeX = (i % 2 == 0) ? 20 : SCREEN_W - 20;
        int treeY = 20 + (SCREEN_H - 50) * (1.0f - z);
        int treeSize = (int)(40 * (1.0f - z * 0.7f));  // Changed from 25 to 40
        
        ski_drawPineTree(tft, treeX, treeY, treeSize);
    }
}

void ski_drawGate(TFT_eSPI &tft, int lane, float z, bool isRed, bool erase = false) {
    int x, y;
    ski_getLanePosition(lane, z, x, y);
    
    int poleHeight = (int)(30 * (1.0f - z * 0.7f));
    int poleWidth = max(2, (int)(4 * (1.0f - z * 0.7f)));
    int gateWidth = max(15, (int)(25 * (1.0f - z * 0.7f)));
    
    if (erase) {
        // Calculate snow color at this Y position
        int horizonY = 20;
        float t = (float)(y - horizonY) / (SCREEN_H - horizonY);
        uint16_t snowShade = tft.color565(255, 255, 255 - (int)(t * 40));
        
        // Erase with appropriate snow shade
        tft.fillRect(x - gateWidth - 5, y - poleHeight - 5, gateWidth * 2 + 10, poleHeight + 10, snowShade);
        return;
    }
    
    uint16_t color = isRed ? COLOR_GATE_RED : COLOR_GATE_BLUE;
    
    // Left pole
    tft.fillRect(x - gateWidth, y - poleHeight, poleWidth, poleHeight, color);
    // Right pole
    tft.fillRect(x + gateWidth - poleWidth, y - poleHeight, poleWidth, poleHeight, color);
    
    // Flags
    int flagSize = poleHeight / 3;
    tft.fillTriangle(x - gateWidth, y - poleHeight,
                    x - gateWidth, y - poleHeight + flagSize,
                    x - gateWidth + flagSize, y - poleHeight + flagSize/2, color);
    tft.fillTriangle(x + gateWidth, y - poleHeight,
                    x + gateWidth, y - poleHeight + flagSize,
                    x + gateWidth - flagSize, y - poleHeight + flagSize/2, color);
}

void ski_drawSkier(TFT_eSPI &tft, int lane, bool erase = false) {
    int x, y;
    ski_getLanePosition(lane, 0.05f, x, y);
    
    if (erase) {
        // Calculate snow color at this Y position
        int horizonY = 20;
        float t = (float)(y - horizonY) / (SCREEN_H - horizonY);
        uint16_t snowShade = tft.color565(255, 255, 255 - (int)(t * 40));
        tft.fillRect(x - 15, y - 20, 30, 40, snowShade);
        return;
    }
    
    // Skier body (colorful outfit)
    uint16_t jacketColor = TFT_RED;
    uint16_t pantsColor = TFT_BLUE;
    
    // Head
    tft.fillCircle(x, y - 8, 4, TFT_YELLOW);  // Helmet
    
    // Body
    tft.fillRect(x - 4, y - 4, 8, 10, jacketColor);
    
    // Legs
    tft.fillRect(x - 5, y + 6, 3, 8, pantsColor);
    tft.fillRect(x + 2, y + 6, 3, 8, pantsColor);
    
    // Skis
    tft.fillRect(x - 6, y + 14, 12, 2, TFT_WHITE);
    tft.drawRect(x - 6, y + 14, 12, 2, TFT_BLACK);
}

void ski_drawFinishLine(TFT_eSPI &tft, float z, bool erase = false) {
    if (z < 0.0f || z > 1.0f) return;
    
    int x, y;
    ski_getLanePosition(2, z, x, y);  // Center lane
    
    // Red finish line banner across course
    int width = (int)(SCREEN_W * 0.6f * (1.0f - z * 0.5f));
    int lineY = y - (int)(30 * (1.0f - z * 0.7f));
    int lineHeight = max(4, (int)(8 * (1.0f - z * 0.5f)));
    
    if (erase) {
        // Calculate snow color at this Y position
        int horizonY = 20;
        float t = (float)(lineY - horizonY) / (SCREEN_H - horizonY);
        uint16_t snowShade = tft.color565(255, 255, 255 - (int)(t * 40));
        
        // Erase entire finish line area including spectators
        tft.fillRect(x - width/2 - 10, lineY - 30, width + 20, 60, snowShade);
        return;
    }
    
    // Checkered pattern
    int squareSize = max(4, (int)(8 * (1.0f - z * 0.5f)));
    for (int i = 0; i < width; i += squareSize) {
        uint16_t color = ((i / squareSize) % 2 == 0) ? COLOR_FINISH_LINE : TFT_BLACK;
        tft.fillRect(x - width/2 + i, lineY, squareSize, lineHeight, color);
    }
    
    // Spectators (simple colorful stick figures) - LARGER and more visible
    int specSize = max(3, (int)(5 * (1.0f - z * 0.6f)));
    int specSpacing = (int)(35 * (1.0f - z * 0.5f));
    for (int i = -4; i <= 4; i++) {
        if (i == 0) continue;
        int specX = x + i * specSpacing;
        int specY = y + (int)(5 * (1.0f - z * 0.5f));
        
        // Head - different colors for variety
        uint16_t headColor = (i % 3 == 0) ? TFT_YELLOW : ((i % 3 == 1) ? TFT_CYAN : TFT_MAGENTA);
        tft.fillCircle(specX, specY - specSize * 2, specSize, headColor);
        
        // Body
        int bodyHeight = specSize * 3;
        tft.drawLine(specX, specY - specSize * 2 + specSize, specX, specY + bodyHeight, TFT_WHITE);
        
        // Arms (raised cheering)
        tft.drawLine(specX, specY, specX - specSize, specY - specSize, TFT_WHITE);
        tft.drawLine(specX, specY, specX + specSize, specY - specSize, TFT_WHITE);
    }
}
void ski_drawUI(TFT_eSPI &tft, int score, unsigned long elapsedMillis, int countdown = -1) {
    // Olympic rings in upper left
    wo_drawMiniOlympicRings(tft, 15, 15);
    
    // Score in upper right
    static int lastScore = -1;
    if (score != lastScore) {
        tft.fillRect(SCREEN_W - 80, 5, 75, 20, COLOR_SKY);
        char scoreBuf[16];
        snprintf(scoreBuf, sizeof(scoreBuf), "%d", score);
        tft.setTextColor(TFT_WHITE, COLOR_SKY);
        tft.setTextFont(4);
        tft.setTextDatum(TR_DATUM);
        tft.drawString(scoreBuf, SCREEN_W - 10, 10);
        lastScore = score;
    }
    
    // Timer in upper center
    static unsigned long lastDispTime = 0;
    unsigned long dispTime = elapsedMillis / 100;  // Update every 100ms
    if (dispTime != lastDispTime) {
        tft.fillRect(SCREEN_W/2 - 40, 5, 80, 20, COLOR_SKY);
        char timeBuf[16];
        int seconds = elapsedMillis / 1000;
        int hundredths = (elapsedMillis % 1000) / 10;
        snprintf(timeBuf, sizeof(timeBuf), "%d.%02d", seconds, hundredths);
        tft.setTextColor(TFT_YELLOW, COLOR_SKY);
        tft.setTextFont(4);
        tft.setTextDatum(TC_DATUM);
        tft.drawString(timeBuf, SCREEN_W/2, 8);
        lastDispTime = dispTime;
    }
    
    // Countdown in center (if active)
    if (countdown > 0) {
        tft.setTextColor(TFT_YELLOW, COLOR_SKY);
        tft.setTextFont(7);
        tft.setTextDatum(MC_DATUM);
        char countBuf[8];
        snprintf(countBuf, sizeof(countBuf), "%d", countdown);
        tft.drawString(countBuf, SCREEN_W/2, SCREEN_H/2);
    }
}

//=============================================================================
// SKIING GAME - MAIN LOGIC
//=============================================================================
void ski_initGates() {
    // Spread gates much further apart - increase from 0.15f to 0.5f
    for (int i = 0; i < MAX_GATES; i++) {
        wo_gates[i].z = 1.0f + i * 0.5f;  // Much more spacing!
        wo_gates[i].lane = random(1, 4);  // Lanes 1, 2, or 3 (avoid edges)
        wo_gates[i].active = true;
        wo_gates[i].passed = false;
        wo_gates[i].hit = false;
    }
}

bool ski_checkGatePass(int playerLane) {
    bool scoredPoint = false;
    
    for (int i = 0; i < MAX_GATES; i++) {
        if (!wo_gates[i].active || wo_gates[i].passed) continue;
        
        // Check if gate is at player position
        if (wo_gates[i].z < 0.1f && wo_gates[i].z > -0.05f) {
            wo_gates[i].passed = true;
            
            // Check if player is in correct lane
            if (abs(playerLane - wo_gates[i].lane) <= 1) {  // Allow 1 lane tolerance
                wo_skiScore += 100;
                scoredPoint = true;
            } else {
                // Missed gate - small penalty
                wo_skiScore = max(0, wo_skiScore - 20);
            }
        }
    }
    
    return scoredPoint;
}

void ski_runDownhillSkiing(TFT_eSPI &tft) {
    // Initialize
    ski_initLanes();
    ski_initGates();
    wo_skiScore = 0;
    wo_playerLane = 2;  // Start in center
    wo_lastPlayerLane = 2;
    wo_skiSpeed = SKI_SPEED_BASE;
    float courseZ = 0.0f;
    bool finished = false;
    
// Draw initial scene ONCE
    tft.fillScreen(COLOR_SKY);
    ski_drawMountainOnce(tft);
    
// 4-second countdown with BEEP TONES (3, 2, 1, 0)
    wo_startTime = millis();
    for (int countdown = 3; countdown >= 0; countdown--) {
        if (countdown > 0) {
            ski_drawUI(tft, wo_skiScore, 0, countdown);
            // Play countdown beep for 3, 2, 1
            playSound("/sounds/beep.wav", false);
        } else {
            // Display "0" 
            ski_drawUI(tft, wo_skiScore, 0, -1);  // No countdown number in UI
            tft.setTextColor(TFT_YELLOW, COLOR_SKY);
            tft.setTextFont(7);
            tft.setTextDatum(MC_DATUM);
            tft.drawString("0", SCREEN_W/2, SCREEN_H/2);
            // Play GO beep for 0
            playSound("/sounds/beep_go.wav", false);
        }
        
        unsigned long countStart = millis();
        while (millis() - countStart < 1000) {
            updateAudio();
            delay(10);
        }
        // Clear countdown number
        tft.fillCircle(SCREEN_W/2, SCREEN_H/2, 40, COLOR_SNOW);
    }
    
    // Show "GO!"
    tft.setTextColor(TFT_GREEN, COLOR_SNOW);
    tft.setTextFont(7);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("GO!", SCREEN_W/2, SCREEN_H/2);
    
    delay(500);
    tft.fillCircle(SCREEN_W/2, SCREEN_H/2, 50, COLOR_SNOW);
    
    // Reset start time for actual race  <--- STOP HERE
    wo_startTime = millis();
    
    // Main game loop
    int lastRotary = rotaryPos;
    int lastBtn = HIGH;
    unsigned long lastFrame = millis();
    
    // Track previous positions for erasing
    int prevSkierLane = wo_playerLane;
    float prevGateZ[MAX_GATES];
    float prevFinishZ = -1.0f;
    
    // Initialize previous gate positions
    for (int i = 0; i < MAX_GATES; i++) {
        prevGateZ[i] = wo_gates[i].z;
    }
    
    while (!finished) {
        unsigned long now = millis();
        float deltaTime = (now - lastFrame) / 1000.0f;
        lastFrame = now;
        
        // Handle input
        int rotDiff = rotaryPos - lastRotary;
        if (rotDiff > 1 && wo_playerLane < NUM_SKI_LANES - 1) {
            wo_playerLane++;
            lastRotary = rotaryPos;
            // Physics: turning slows you down!
            wo_skiSpeed *= 0.85f;
        } else if (rotDiff < -1 && wo_playerLane > 0) {
            wo_playerLane--;
            lastRotary = rotaryPos;
            // Physics: turning slows you down!
            wo_skiSpeed *= 0.85f;
        }
        
        // Gradually increase speed when not turning
        if (wo_playerLane == wo_lastPlayerLane) {
            wo_skiSpeed += 0.01f;  // Accelerate slowly
            if (wo_skiSpeed > SKI_SPEED_BASE * 1.5f) {
                wo_skiSpeed = SKI_SPEED_BASE * 1.5f;  // Cap speed
            }
        }
        wo_lastPlayerLane = wo_playerLane;
        
        // Update course position (simulate skiing downhill)
        courseZ += wo_skiSpeed * deltaTime;
        
        // Erase and update gate positions
        for (int i = 0; i < MAX_GATES; i++) {
            if (wo_gates[i].active) {
                // Erase at old position
                if (prevGateZ[i] > 0.0f && prevGateZ[i] <= 1.0f) {
                    bool isRed = (i % 2 == 0);
                    ski_drawGate(tft, wo_gates[i].lane, prevGateZ[i], isRed, true);
                }
                
                // Update position
                wo_gates[i].z -= wo_skiSpeed * deltaTime * 0.3f;
                prevGateZ[i] = wo_gates[i].z;
                
                // Deactivate gates that are behind player
                if (wo_gates[i].z < -0.2f) {
                    wo_gates[i].active = false;
                }
            }
        }
        
        // Check for gate passes
        ski_checkGatePass(wo_playerLane);
        
        // Calculate finish line position (1.0 units after last gate) - MOVED UP HERE
        float finishZ = wo_gates[MAX_GATES - 1].z - 1.0f;
        
        // Erase previous finish line if it moved
        if (prevFinishZ > 0.0f && prevFinishZ <= 1.0f && abs(finishZ - prevFinishZ) > 0.01f) {
            ski_drawFinishLine(tft, prevFinishZ, true);
        }
        prevFinishZ = finishZ;
        
        // Check for finish
        if (finishZ < -0.1f) {
            finished = true;
        }
        
        // Draw finish line when approaching (and not passed)
        if (finishZ > 0.0f && finishZ <= 1.0f) {
            ski_drawFinishLine(tft, finishZ, false);
        }
        
        // Draw gates ONLY if they're BEFORE the finish line
        for (int i = 0; i < MAX_GATES; i++) {
            if (wo_gates[i].active && wo_gates[i].z > 0.0f && wo_gates[i].z <= 1.0f) {
                // Only draw if gate is clearly before finish line
                bool shouldDraw = false;
                if (finishZ > 1.0f) {
                    // Finish line not visible yet, draw all gates
                    shouldDraw = true;
                } else if (finishZ >= 0.0f && wo_gates[i].z > finishZ + 0.2f) {
                    // Finish line visible, only draw gates well ahead of it
                    shouldDraw = true;
                }
                
                if (shouldDraw) {
                    bool isRed = (i % 2 == 0);
                    ski_drawGate(tft, wo_gates[i].lane, wo_gates[i].z, isRed, false);
                }
            }
        }
        
        // Erase skier at previous position if lane changed
        if (prevSkierLane != wo_playerLane) {
            ski_drawSkier(tft, prevSkierLane, true);
        }
        
        // Draw skier at current position
        ski_drawSkier(tft, wo_playerLane, false);
        prevSkierLane = wo_playerLane;
        
        // Draw UI with timer
        unsigned long elapsed = millis() - wo_startTime;
        ski_drawUI(tft, wo_skiScore, elapsed, -1);
        
        // Check for quit (button press)
        int btn = digitalRead(PIN_KO);
        if (btn == LOW && lastBtn == HIGH) {
            finished = true;  // Allow early exit
        }
        lastBtn = btn;
        
        delay(30);  // ~33 FPS
    }
    
    // Calculate final time
    unsigned long finalTime = millis() - wo_startTime;
    int seconds = finalTime / 1000;
    int hundredths = (finalTime % 1000) / 10;
    
    // Game over screen
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextFont(4);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("FINISH!", SCREEN_W/2, SCREEN_H/2 - 50);
    
    char timeBuf[32];
    snprintf(timeBuf, sizeof(timeBuf), "Time: %d.%02d sec", seconds, hundredths);
    tft.setTextFont(2);
    tft.drawString(timeBuf, SCREEN_W/2, SCREEN_H/2 - 10);
    
    char scoreBuf[32];
    snprintf(scoreBuf, sizeof(scoreBuf), "Score: %d", wo_skiScore);
    tft.drawString(scoreBuf, SCREEN_W/2, SCREEN_H/2 + 20);
    
    tft.setTextFont(2);
    tft.drawString("Press button to continue", SCREEN_W/2, SCREEN_H - 30);
    
    // Wait for button press
    while (digitalRead(PIN_KO) == HIGH) {
        updateAudio();
        delay(50);
    }
    while (digitalRead(PIN_KO) == LOW) delay(10);
    delay(400);
}

//=============================================================================
// SKI JUMP - PLACEHOLDER
//=============================================================================
void ski_runSkiJump(TFT_eSPI &tft) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextFont(4);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("SKI JUMP", SCREEN_W/2, SCREEN_H/2 - 30);
    tft.setTextFont(2);
    tft.drawString("Coming Soon!", SCREEN_W/2, SCREEN_H/2 + 10);
    tft.drawString("Press button to return", SCREEN_W/2, SCREEN_H - 30);
    
    while (digitalRead(PIN_KO) == HIGH) {
        updateAudio();
        delay(50);
    }
    while (digitalRead(PIN_KO) == LOW) delay(10);
    delay(400);
}

//=============================================================================
// LUGE - PLACEHOLDER
//=============================================================================
void ski_runLuge(TFT_eSPI &tft) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextFont(4);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("LUGE", SCREEN_W/2, SCREEN_H/2 - 30);
    tft.setTextFont(2);
    tft.drawString("Coming Soon!", SCREEN_W/2, SCREEN_H/2 + 10);
    tft.drawString("Press button to return", SCREEN_W/2, SCREEN_H - 30);
    
    while (digitalRead(PIN_KO) == HIGH) {
        updateAudio();
        delay(50);
    }
    while (digitalRead(PIN_KO) == LOW) delay(10);
    delay(400);
}

//=============================================================================
// MAIN ENTRY POINT
//=============================================================================
void run_Winter_Olympics(TFT_eSPI &tft) {
    tft.setRotation(3);
    pinMode(PIN_KO, INPUT_PULLUP);
    pinMode(PIN_PUSH, INPUT_PULLUP);
    
    // Show splash screen (only once) - this starts the music
    wo_showSplashScreen(tft);
    
    // Main game loop
    while (true) {
        // Show game selection menu - music continues playing
        int selectedGame = wo_showGameMenu(tft);
        
        // Stop music before starting game
        stopAudio();
        
        // Run selected game
        switch (selectedGame) {
            case 0:  // Downhill Skiing
                ski_runDownhillSkiing(tft);
                break;
                
            case 1:  // Ski Jump
                ski_runSkiJump(tft);
                break;
                
            case 2:  // Luge
                ski_runLuge(tft);
                break;
        }
        
        // After game ends, restart Olympics theme for menu
        playSound("/sounds/Olympics_Basic_Theme_and_Fanfare.wav", true);
        
        tft.init();
        tft.setRotation(3);
        
        // Loop back to menu with music playing
    }
}

#endif // WINTER_OLYMPICS_H