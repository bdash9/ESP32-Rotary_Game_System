#ifndef RHYTHM_RUNNER_H
#define RHYTHM_RUNNER_H

#include <Arduino.h>
#include <TFT_eSPI.h>

extern void playSound(const char *path, bool stopCurrent);
extern void updateAudio();

#define SCREEN_W 320
#define SCREEN_H 240

// Game constants
#define NUM_LANES 3
#define LANE_LEFT 0
#define LANE_CENTER 1
#define LANE_RIGHT 2

#define MAX_NOTES 8
#define HIT_ZONE_Y 200  // Where player must hit notes (near bottom)

// Timing windows (in pixels from hit zone)
#define TIMING_GOLD 10  // Middle ground between 8 and 12
#define TIMING_GREEN 18
#define TIMING_YELLOW 28
#define TIMING_RED 40


// Colors
#define COLOR_BG TFT_BLACK
#define COLOR_LANE TFT_DARKGREY
#define COLOR_LANE_ACTIVE TFT_CYAN
#define COLOR_PLAYER TFT_WHITE
#define COLOR_NOTE TFT_MAGENTA
#define COLOR_NOTE_HOLD TFT_ORANGE  // Changed from YELLOW to ORANGE
#define COLOR_GOLD 0xFEA0  // Gold color
#define COLOR_SCORE TFT_WHITE
#define COLOR_NOTE_DOUBLE TFT_YELLOW
#define COLOR_MEDIUM_BLUE 0x4C9F  // Medium blue like The Who album


// Perspective settings for 3D effect
struct PerspectiveLane {
    int topX, topY;      // Vanishing point position
    int bottomLeftX, bottomLeftY;
    int bottomRightX, bottomRightY;
};

PerspectiveLane lanes[NUM_LANES];

// Fever Mode / Bonus System
int perfectHolds = 0;  // Count of perfect hold completions (0-3)
bool feverMode = false;
unsigned long feverModeStart = 0;
const unsigned long FEVER_DURATION = 10000;  // 10 seconds

struct Note {
    int lane;           // Which lane (0, 1, 2)
    float z;            // Distance from player (0 = at player, 1 = far away)
    bool active;
    bool isHold;        // Is this a hold note?
    bool isDouble;      // Is this a double-tap note?
    float holdLength;   // How long to hold (in z units)
    bool hit;           // Has player hit this note?
    bool holding;       // Is player currently holding this note?
    int hitAccuracy;    // Accuracy when first hit (1-4)
    int hitsReceived;   // For double notes - how many hits so far
};

Note notes[MAX_NOTES];

// Song selection
const char* songTitles[] = {
    "Going Mobile",
    "Baba O'Riley",
    "Bargain",
    "Behind Blue Eyes",
    "Getting in Tune",
    "The Song is Over",
    "Won't Get Fooled Again",
    "My Generation",
    "Magic Bus"
};

const char* songFiles[] = {
    "/sounds/Going_Mobile.wav",
    "/sounds/Baba_O'Riley.wav",
    "/sounds/Bargain.wav",
    "/sounds/Behind_Blue_Eyes.wav",
    "/sounds/Getting_in_Tune.wav",
    "/sounds/The_Song_is_Over.wav",
    "/sounds/Won't_Get_Fooled_Again.wav",
    "/sounds/My_Generation.wav",
    "/sounds/Magic_Bus.wav"
};

const int NUM_SONGS = 9;
int selectedSong = 0;

// Player state
int playerLane = LANE_CENTER;
int targetLane = LANE_CENTER;
float laneTransition = 0.0f;  // 0.0 to 1.0 for smooth movement
bool isMoving = false;

// Game state
int score = 0;
int lastAccuracy = 0;  // 0=none, 1=red, 2=yellow, 3=green, 4=gold
unsigned long lastHitTime = 0;
unsigned long gameStartTime = 0;
int currentActiveLane = LANE_CENTER;
unsigned long lastLaneChangeTime = 0;

// Initialize perspective lane positions
void rr_initLanes() {
    // Vanishing point at top center - TIGHTER CONVERGENCE
    int vanishX = SCREEN_W / 2;
    int vanishY = 40;
    
    // Bottom positions for 3 lanes
    int bottomY = HIT_ZONE_Y + 20;
    int laneSpacing = 60;
    int centerX = SCREEN_W / 2;
    
    // Left lane - NARROWER vanishing point spread
    lanes[LANE_LEFT].topX = vanishX - 5;  // Changed from -15 to -5
    lanes[LANE_LEFT].topY = vanishY;
    lanes[LANE_LEFT].bottomLeftX = centerX - laneSpacing - 25;
    lanes[LANE_LEFT].bottomLeftY = bottomY;
    lanes[LANE_LEFT].bottomRightX = centerX - laneSpacing + 25;
    lanes[LANE_LEFT].bottomRightY = bottomY;
    
    // Center lane
    lanes[LANE_CENTER].topX = vanishX;
    lanes[LANE_CENTER].topY = vanishY;
    lanes[LANE_CENTER].bottomLeftX = centerX - 25;
    lanes[LANE_CENTER].bottomLeftY = bottomY;
    lanes[LANE_CENTER].bottomRightX = centerX + 25;
    lanes[LANE_CENTER].bottomRightY = bottomY;
    
    // Right lane - NARROWER vanishing point spread
    lanes[LANE_RIGHT].topX = vanishX + 5;  // Changed from +15 to +5
    lanes[LANE_RIGHT].topY = vanishY;
    lanes[LANE_RIGHT].bottomLeftX = centerX + laneSpacing - 25;
    lanes[LANE_RIGHT].bottomLeftY = bottomY;
    lanes[LANE_RIGHT].bottomRightX = centerX + laneSpacing + 25;
    lanes[LANE_RIGHT].bottomRightY = bottomY;
}

// Interpolate position along a lane based on z (0=close, 1=far)
void rr_getLanePosition(int lane, float z, int &x, int &y, int &width) {
    PerspectiveLane &l = lanes[lane];
    
    // Interpolate from bottom to top based on z
    int bottomCenterX = (l.bottomLeftX + l.bottomRightX) / 2;
    x = bottomCenterX + (l.topX - bottomCenterX) * z;
    y = l.bottomLeftY + (l.topY - l.bottomLeftY) * z;
    
    // Width shrinks with distance
    int bottomWidth = l.bottomRightX - l.bottomLeftX;
    width = bottomWidth * (1.0f - z * 0.8f);  // Shrinks to 20% at far distance
}

// Draw a single lane with perspective
void rr_drawLane(TFT_eSPI &tft, int lane, bool active, bool erase = false) {
    PerspectiveLane &l = lanes[lane];
    uint16_t color = erase ? COLOR_BG : (active ? COLOR_LANE_ACTIVE : COLOR_LANE);
    
    // Draw left edge - use stored topX directly (no extra offset!)
    tft.drawLine(l.bottomLeftX, l.bottomLeftY, l.topX, l.topY, color);
    
    // Draw right edge - use stored topX directly (no extra offset!)
    tft.drawLine(l.bottomRightX, l.bottomRightY, l.topX, l.topY, color);
    
    // Draw some horizontal segments for depth
    for (float z = 0.0f; z <= 1.0f; z += 0.25f) {
        int x1 = l.bottomLeftX + (l.topX - l.bottomLeftX) * z;
        int x2 = l.bottomRightX + (l.topX - l.bottomRightX) * z;
        int y = l.bottomLeftY + (l.topY - l.bottomLeftY) * z;
        tft.drawLine(x1, y, x2, y, color);
    }
}

// Draw all lanes
void rr_drawAllLanes(TFT_eSPI &tft) {
    for (int i = 0; i < NUM_LANES; i++) {
        rr_drawLane(tft, i, i == currentActiveLane, false);
    }
}

// Draw diamond-shaped player
void rr_drawPlayer(TFT_eSPI &tft, int lane, float transition, bool erase = false) {
    // Calculate position between current and target lane
    int x1, y1, w1, x2, y2, w2;
    rr_getLanePosition(playerLane, 0.0f, x1, y1, w1);
    rr_getLanePosition(targetLane, 0.0f, x2, y2, w2);
    
    int x = x1 + (x2 - x1) * transition;
    int y = HIT_ZONE_Y;
    
    uint16_t color = erase ? COLOR_BG : COLOR_PLAYER;
    
    // Diamond shape
    int size = 12;
    tft.drawLine(x, y - size, x + size, y, color);
    tft.drawLine(x + size, y, x, y + size, color);
    tft.drawLine(x, y + size, x - size, y, color);
    tft.drawLine(x - size, y, x, y - size, color);
    
    // Inner diamond for thickness
    size = 10;
    tft.drawLine(x, y - size, x + size, y, color);
    tft.drawLine(x + size, y, x, y + size, color);
    tft.drawLine(x, y + size, x - size, y, color);
    tft.drawLine(x - size, y, x, y - size, color);
}

// Draw a note on a lane
void rr_drawNote(TFT_eSPI &tft, int lane, float z, bool isHold, bool isDouble, float holdLength, bool erase = false) {
    int x, y, width;
    rr_getLanePosition(lane, z, x, y, width);
    
    uint16_t color = erase ? COLOR_BG : (isHold ? COLOR_NOTE_HOLD : (isDouble ? COLOR_NOTE_DOUBLE : COLOR_NOTE));
    
// Draw diamond note at front
    int size = 8 * (1.0f - z * 0.7f);  // Shrinks with distance
    if (size < 3) size = 3;
    
if (isDouble) {
        // Draw TWO diamonds in sequence (one behind the other)
        float spacing = 0.15f;  // Distance between diamonds in Z
        
        if (erase) {
            // Clear area for both diamonds
            tft.fillRect(x - size - 2, y - size - 2, size * 2 + 4, size * 2 + 4, COLOR_BG);
            
            // Also erase second diamond
            int x2, y2, w2;
            float z2 = z + spacing;
            if (z2 <= 1.0f) {
                rr_getLanePosition(lane, z2, x2, y2, w2);
                int size2 = 8 * (1.0f - z2 * 0.7f);
                if (size2 < 3) size2 = 3;
                tft.fillRect(x2 - size2 - 2, y2 - size2 - 2, size2 * 2 + 4, size2 * 2 + 4, COLOR_BG);
            }
        } else {
            // Draw first diamond (front)
            tft.drawLine(x, y - size, x + size, y, color);
            tft.drawLine(x + size, y, x, y + size, color);
            tft.drawLine(x, y + size, x - size, y, color);
            tft.drawLine(x - size, y, x, y - size, color);
            
            // Draw second diamond (behind first)
            int x2, y2, w2;
            float z2 = z + spacing;
            if (z2 <= 1.0f) {
                rr_getLanePosition(lane, z2, x2, y2, w2);
                int size2 = 8 * (1.0f - z2 * 0.7f);
                if (size2 < 3) size2 = 3;
                
                tft.drawLine(x2, y2 - size2, x2 + size2, y2, color);
                tft.drawLine(x2 + size2, y2, x2, y2 + size2, color);
                tft.drawLine(x2, y2 + size2, x2 - size2, y2, color);
                tft.drawLine(x2 - size2, y2, x2, y2 - size2, color);
            }
        }
    } else if (isHold) {
        // Calculate back position
        int x2, y2, w2;
        float backZ = z + holdLength;
        if (backZ > 1.0f) backZ = 1.0f;
        rr_getLanePosition(lane, backZ, x2, y2, w2);
        
        int barWidth = size / 2;
        int backSize = 8 * (1.0f - backZ * 0.7f);
        if (backSize < 3) backSize = 3;
        
        if (erase) {
            // When erasing, clear a larger rectangular area
            int minX = min(x - size - 2, x2 - backSize - 2);
            int maxX = max(x + size + 2, x2 + backSize + 2);
            int minY = min(y - size - 2, y2 - backSize - 2);
            int maxY = max(y + size + 2, y2 + backSize + 2);
            tft.fillRect(minX, minY, maxX - minX, maxY - minY, COLOR_BG);
        } else {
            // Draw elongated bar from back to front
            
            // Draw connecting lines to create elongated shape
            tft.drawLine(x - barWidth, y, x2 - barWidth * 0.7f, y2, color);
            tft.drawLine(x + barWidth, y, x2 + barWidth * 0.7f, y2, color);
            
            // Fill the elongated area
            int steps = abs(y - y2);
            if (steps > 0) {
                for (int i = 0; i <= steps; i++) {
                    float t = (float)i / steps;
                    int ix = x + (x2 - x) * t;
                    int iy = y + (y2 - y) * t;
                    int iw = barWidth + (barWidth * 0.7f - barWidth) * t;
                    tft.drawLine(ix - iw, iy, ix + iw, iy, color);
                }
            }
            
            // Draw diamond at back end
            tft.drawLine(x2, y2 - backSize, x2 + backSize, y2, color);
            tft.drawLine(x2 + backSize, y2, x2, y2 + backSize, color);
            tft.drawLine(x2, y2 + backSize, x2 - backSize, y2, color);
            tft.drawLine(x2 - backSize, y2, x2, y2 - backSize, color);
            
            // Draw diamond at front
            tft.drawLine(x, y - size, x + size, y, color);
            tft.drawLine(x + size, y, x, y + size, color);
            tft.drawLine(x, y + size, x - size, y, color);
            tft.drawLine(x - size, y, x, y - size, color);
        }
    } else {
        // Regular note - just draw/erase diamond
        if (erase) {
            // Clear area around diamond
            tft.fillRect(x - size - 1, y - size - 1, size * 2 + 2, size * 2 + 2, COLOR_BG);
        } else {
            tft.drawLine(x, y - size, x + size, y, color);
            tft.drawLine(x + size, y, x, y + size, color);
            tft.drawLine(x, y + size, x - size, y, color);
            tft.drawLine(x - size, y, x, y - size, color);
        }
    }
}

// Draw hit zone indicator
void rr_drawHitZone(TFT_eSPI &tft) {
    // Draw a line across all lanes at hit zone
    tft.drawLine(40, HIT_ZONE_Y, SCREEN_W - 40, HIT_ZONE_Y, TFT_GREEN);
    tft.drawLine(40, HIT_ZONE_Y + 1, SCREEN_W - 40, HIT_ZONE_Y + 1, TFT_GREEN);
}

// Draw accuracy circle at bottom
void rr_drawAccuracyCircle(TFT_eSPI &tft, int accuracy) {
    int cx = SCREEN_W / 2;
    int cy = SCREEN_H - 15;
    int radius = 10;
    
    // Clear old circle
    tft.fillCircle(cx, cy, radius + 2, COLOR_BG);
    
    // Draw outline
    tft.drawCircle(cx, cy, radius, TFT_WHITE);
    
    // Fill with accuracy color
    uint16_t fillColor = COLOR_BG;
    if (accuracy == 4) fillColor = COLOR_GOLD;
    else if (accuracy == 3) fillColor = TFT_GREEN;
    else if (accuracy == 2) fillColor = TFT_YELLOW;
    else if (accuracy == 1) fillColor = TFT_RED;
    
    if (accuracy > 0) {
        tft.fillCircle(cx, cy, radius - 2, fillColor);
    }
}

// Draw score at top
void rr_drawScore(TFT_eSPI &tft, int score) {
    static int lastScore = -1;
    if (score == lastScore) return;
    
    // Clear score area
    tft.fillRect(SCREEN_W - 80, 5, 75, 20, COLOR_BG);
    
    // Draw score
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", score);
    tft.setTextColor(COLOR_SCORE, COLOR_BG);
    tft.setTextFont(2);
    tft.setTextDatum(TR_DATUM);
    tft.drawString(buf, SCREEN_W - 5, 8);
    
    lastScore = score;
}

// Draw lightning explosion effect
void rr_drawLightningExplosion(TFT_eSPI &tft, int x, int y, int frame) {
    // Radiating lightning bolts
    for (int i = 0; i < 8; i++) {
        float angle = (i * 45 + frame * 30) * 0.01745;
        int length = 15 + frame * 3;
        
        // Main bolt
        int x1 = x + cos(angle) * 5;
        int y1 = y + sin(angle) * 5;
        int x2 = x + cos(angle) * length;
        int y2 = y + sin(angle) * length;
        
        // Jagged lightning effect
        int midX = (x1 + x2) / 2 + random(-5, 6);
        int midY = (y1 + y2) / 2 + random(-5, 6);
        
        uint16_t color = (i % 2 == 0) ? TFT_YELLOW : TFT_WHITE;
        tft.drawLine(x1, y1, midX, midY, color);
        tft.drawLine(midX, midY, x2, y2, color);
    }
    
    // Center flash
    int flashSize = 8 - frame;
    if (flashSize > 0) {
        tft.fillCircle(x, y, flashSize, TFT_WHITE);
        tft.drawCircle(x, y, flashSize + 2, TFT_YELLOW);
    }
}

// Draw diamond indicator at bottom right
void rr_drawDiamondIndicator(TFT_eSPI &tft, int index, bool filled, bool electrified) {
    int spacing = 25;
    int startX = SCREEN_W - 60;
    int y = HIT_ZONE_Y + 20;  // Align with bottom of lanes
    int x = startX + index * spacing;
    
    int size = 8;
    
    // Clear area first to prevent artifacts
    tft.fillRect(x - size - 6, y - size - 2, (size + 6) * 2, (size + 2) * 2, COLOR_BG);
    
    if (filled) {
        if (electrified) {
            // Animated electric effect
            uint16_t color = ((millis() / 100) % 2 == 0) ? TFT_YELLOW : TFT_WHITE;
            tft.drawLine(x, y - size, x + size, y, color);
            tft.drawLine(x + size, y, x, y + size, color);
            tft.drawLine(x, y + size, x - size, y, color);
            tft.drawLine(x - size, y, x, y - size, color);
            
            // Inner filled diamond
            for (int i = 1; i < size - 1; i++) {
                tft.drawLine(x - i, y, x + i, y, color);
            }
            
            // Electric sparks
            if ((millis() / 50) % 3 == 0) {
                tft.drawLine(x - size - 2, y, x - size - 5, y + random(-3, 4), TFT_CYAN);
                tft.drawLine(x + size + 2, y, x + size + 5, y + random(-3, 4), TFT_CYAN);
            }
        } else {
            // Filled but not electrified yet
            tft.drawLine(x, y - size, x + size, y, TFT_YELLOW);
            tft.drawLine(x + size, y, x, y + size, TFT_YELLOW);
            tft.drawLine(x, y + size, x - size, y, TFT_YELLOW);
            tft.drawLine(x - size, y, x, y - size, TFT_YELLOW);
            
            for (int i = 1; i < size - 1; i++) {
                tft.drawLine(x - i, y, x + i, y, TFT_YELLOW);
            }
        }
    } else {
        // Empty outline
        tft.drawLine(x, y - size, x + size, y, TFT_DARKGREY);
        tft.drawLine(x + size, y, x, y + size, TFT_DARKGREY);
        tft.drawLine(x, y + size, x - size, y, TFT_DARKGREY);
        tft.drawLine(x - size, y, x, y - size, TFT_DARKGREY);
    }
}

// Draw all three diamond indicators
void rr_drawAllDiamondIndicators(TFT_eSPI &tft, int perfectCount, bool fever) {
    for (int i = 0; i < 3; i++) {
        rr_drawDiamondIndicator(tft, i, i < perfectCount, fever);
    }
}

// Draw electrified player (for fever mode)
void rr_drawPlayerElectrified(TFT_eSPI &tft, int lane, float transition) {
    // Calculate position between current and target lane
    int x1, y1, w1, x2, y2, w2;
    rr_getLanePosition(playerLane, 0.0f, x1, y1, w1);
    rr_getLanePosition(targetLane, 0.0f, x2, y2, w2);
    
    int x = x1 + (x2 - x1) * transition;
    int y = HIT_ZONE_Y;
    
    // Electric aura
    uint16_t auraColor = ((millis() / 80) % 2 == 0) ? TFT_YELLOW : TFT_CYAN;
    int auraSize = 16;
    tft.drawLine(x, y - auraSize, x + auraSize, y, auraColor);
    tft.drawLine(x + auraSize, y, x, y + auraSize, auraColor);
    tft.drawLine(x, y + auraSize, x - auraSize, y, auraColor);
    tft.drawLine(x - auraSize, y, x, y - auraSize, auraColor);
    
    // Main diamond (white with electric glow)
    int size = 12;
    tft.drawLine(x, y - size, x + size, y, TFT_WHITE);
    tft.drawLine(x + size, y, x, y + size, TFT_WHITE);
    tft.drawLine(x, y + size, x - size, y, TFT_WHITE);
    tft.drawLine(x - size, y, x, y - size, TFT_WHITE);
    
    size = 10;
    tft.drawLine(x, y - size, x + size, y, TFT_WHITE);
    tft.drawLine(x + size, y, x, y + size, TFT_WHITE);
    tft.drawLine(x, y + size, x - size, y, TFT_WHITE);
    tft.drawLine(x - size, y, x, y - size, TFT_WHITE);
    
    // Electric sparks
    if ((millis() / 60) % 4 == 0) {
        int sparkX = x + random(-20, 21);
        int sparkY = y + random(-20, 21);
        tft.drawPixel(sparkX, sparkY, TFT_CYAN);
        tft.drawPixel(sparkX + 1, sparkY, TFT_WHITE);
    }
}

// Vector font for splash screens
struct RR_VecStroke { int x0, y0, x1, y1; };
struct RR_VecLetter { const RR_VecStroke* strokes; int n; int w; };

const RR_VecStroke rr_strokes_R[] = {{0,12,0,0},{0,0,8,0},{8,0,8,6},{8,6,0,6},{0,6,8,12}};
const RR_VecStroke rr_strokes_H[] = {{0,0,0,12},{8,0,8,12},{0,6,8,6}};
const RR_VecStroke rr_strokes_Y[] = {{0,0,4,6},{8,0,4,6},{4,6,4,12}};
const RR_VecStroke rr_strokes_T[] = {{4,0,4,12},{0,0,8,0}};
const RR_VecStroke rr_strokes_M[] = {{0,12,0,0},{0,0,4,6},{4,6,8,0},{8,0,8,12}};
const RR_VecStroke rr_strokes_U[] = {{0,0,0,12},{0,12,8,12},{8,12,8,0}};
const RR_VecStroke rr_strokes_N[] = {{0,12,0,0},{0,0,8,12},{8,12,8,0}};
const RR_VecStroke rr_strokes_E[] = {{0,0,0,12},{0,0,8,0},{0,6,7,6},{0,12,8,12}};
const RR_VecStroke rr_strokes_W[] = {{0,0,0,12},{0,12,4,6},{4,6,8,12},{8,12,8,0}};
const RR_VecStroke rr_strokes_O[] = {{0,0,8,0},{8,0,8,12},{8,12,0,12},{0,12,0,0}};
const RR_VecStroke rr_strokes_D[] = {{0,12,0,0},{0,0,7,2},{7,2,8,6},{8,6,7,10},{7,10,0,12}};
const RR_VecStroke rr_strokes_I[] = {{4,0,4,12}};
const RR_VecStroke rr_strokes_C[] = {{8,0,0,0},{0,0,0,12},{0,12,8,12}};
const RR_VecStroke rr_strokes_S[] = {{8,0,0,0},{0,0,0,6},{0,6,8,6},{8,6,8,12},{8,12,0,12}};
const RR_VecStroke rr_strokes_G[] = {{8,0,0,0},{0,0,0,12},{0,12,8,12},{8,12,8,8},{8,8,5,8}};
// Lowercase letters
const RR_VecStroke rr_strokes_h[] = {{0,0,0,12},{0,6,0,8},{0,8,6,6},{6,6,6,12}};
const RR_VecStroke rr_strokes_e[] = {{6,12,0,12},{0,12,0,6},{0,6,6,6},{6,6,6,9},{6,9,1,9}};
const RR_VecStroke rr_strokes_o[] = {{2,6,6,6},{6,6,6,12},{6,12,2,12},{2,12,2,6}};

const RR_VecLetter rr_vecFont[] = {
    {rr_strokes_R,5,9}, {rr_strokes_H,3,9}, {rr_strokes_Y,3,9}, {rr_strokes_T,2,9},
    {rr_strokes_M,4,9}, {rr_strokes_U,3,9}, {rr_strokes_N,3,9}, {rr_strokes_E,4,9},
    {rr_strokes_W,4,9}, {rr_strokes_O,4,9}, {rr_strokes_D,5,9}, {rr_strokes_I,1,9},
    {rr_strokes_C,3,9}, {rr_strokes_S,5,9}, {rr_strokes_G,5,9},
    {rr_strokes_h,4,9}, {rr_strokes_e,5,9}, {rr_strokes_o,4,9}
};

int rr_getVecFontIdx(char c) {
    switch(c) {
        case 'R': return 0; case 'H': return 1; case 'Y': return 2; case 'T': return 3;
        case 'M': return 4; case 'U': return 5; case 'N': return 6; case 'E': return 7;
        case 'W': return 8; case 'O': return 9; case 'D': return 10; case 'I': return 11;
        case 'C': return 12; case 'S': return 13; case 'G': return 14;
        case 'h': return 15; case 'e': return 16; case 'o': return 17;
        default: return -1;
    }
}

void rr_drawVecLetter(TFT_eSPI &tft, char c, int x, int y, int scale, uint16_t color) {
    int idx = rr_getVecFontIdx(c);
    if(idx < 0) return;
    for (int i = 0; i < rr_vecFont[idx].n; i++) {
        int x0 = x + rr_vecFont[idx].strokes[i].x0 * scale;
        int y0 = y + rr_vecFont[idx].strokes[i].y0 * scale;
        int x1 = x + rr_vecFont[idx].strokes[i].x1 * scale;
        int y1 = y + rr_vecFont[idx].strokes[i].y1 * scale;
        tft.drawLine(x0, y0, x1, y1, color);
    }
}

void rr_drawVecText(TFT_eSPI &tft, const char* text, int x, int y, int scale, uint16_t color) {
    int spacing = 9 * scale;
    int cx = x;
    for (int i = 0; text[i]; i++) {
        if (text[i] != ' ') {
            // Draw letter multiple times for bold effect
            rr_drawVecLetter(tft, text[i], cx, y, scale, color);
            rr_drawVecLetter(tft, text[i], cx + 1, y, scale, color);
            rr_drawVecLetter(tft, text[i], cx, y + 1, scale, color);
        }
        cx += spacing;
    }
}

int rr_vecTextWidth(const char* text, int scale) {
    int len = strlen(text);
    return len * 9 * scale;
}

void rr_drawVecTextSpaced(TFT_eSPI &tft, const char* text, int x, int y, int scale, uint16_t color, int extraSpacing) {
    int spacing = 9 * scale + extraSpacing;
    int cx = x;
    for (int i = 0; text[i]; i++) {
        if (text[i] != ' ') {
            // Draw letter multiple times for bold effect
            rr_drawVecLetter(tft, text[i], cx, y, scale, color);
            rr_drawVecLetter(tft, text[i], cx + 1, y, scale, color);
            rr_drawVecLetter(tft, text[i], cx, y + 1, scale, color);
        }
        cx += spacing;
    }
}

int rr_vecTextWidthSpaced(const char* text, int scale, int extraSpacing) {
    int len = strlen(text);
    return len * (9 * scale + extraSpacing);
}

void rr_showSplash(TFT_eSPI &tft) {
    // Wait for any button press to be released first
    while (digitalRead(PIN_KO) == LOW) delay(10);
    delay(300);  // Debounce delay
    
    tft.fillScreen(COLOR_BG);
    
    // Scales
    int diagonalScale = 3;
    int logoScale = 6;
    int theScale = 3;
    
    // Draw "The Who" logo CENTERED and HIGHER
    int logoY = 30;  // Raised up
    
    // "The" with lowercase h and e
    const char* theText = "The";
    int theX = (SCREEN_W / 2) - 50;  // Position for connecting line
    rr_drawVecText(tft, theText, theX, logoY, theScale, TFT_WHITE);
    
// Calculate position of RIGHT SIDE of 'h' in "The" (the right leg at x=6 in the letter)
    int theHx = theX + (1 * 9 * theScale);      // Position of 'h' letter in "The" (index 1, not 2!)
    int theHrightLeg = theHx + (6 * theScale);  // Right leg of 'h' (at x=6 in letter coordinates)
    int theHy = logoY + (12 * theScale);        // Bottom of 'h'
    
// "Who" - spread out with lowercase h and o
    int whoScale = logoScale;
    int whoSpacing = 14 * whoScale;
    int whoY = logoY + 50;
    int whoShift = 17;  // Shift W and o right by 30 pixels (but NOT h)
    int whoStartX = (SCREEN_W / 2) - (whoSpacing * 1.5) + whoShift;
    
    // Draw 'W' - SHIFTED RIGHT
    for (int ox = 0; ox < 2; ox++) {
        for (int oy = 0; oy < 2; oy++) {
            rr_drawVecLetter(tft, 'W', whoStartX + ox, whoY + oy, whoScale, TFT_WHITE);
        }
    }
    
    // Draw 'h' (lowercase) - STAYS IN PLACE, aligned with "The" h RIGHT leg
    // The stem of lowercase 'h' is at x=0, so position letter at theHrightLeg
    int whoHx = theHrightLeg;  // NO SHIFT - keep connection to "The"
    for (int ox = 0; ox < 2; ox++) {
        for (int oy = 0; oy < 2; oy++) {
            rr_drawVecLetter(tft, 'h', whoHx + ox, whoY + oy, whoScale, TFT_WHITE);
        }
    }
    
    // Draw 'o' (lowercase) with arrow - positioned relative to 'h', THEN shifted right
    int whoOx = whoHx + (8 * whoScale) + whoShift;  // Space after 'h', then shift
    for (int ox = 0; ox < 2; ox++) {
        for (int oy = 0; oy < 2; oy++) {
            rr_drawVecLetter(tft, 'o', whoOx + ox, whoY + oy, whoScale, TFT_WHITE);
        }
    }
    
    // Calculate position of top of 'h' in "Who" for connection line
    int whoHtopX = whoHx;  // Left side of stem
    int whoHtopY = whoY;

// Draw connecting line from "The" h right leg to "Who" h left stem
    for (int i = 0; i < 3; i++) {
        tft.drawLine(theHrightLeg + i, theHy, whoHtopX + i, whoHtopY, TFT_WHITE);
    }
    
    // Draw arrow pointing up from the 'o' (attached to lowercase o)
    int arrowX = whoOx + (4 * whoScale);  // Center of lowercase 'o'
    int arrowBaseY = whoY + (6 * whoScale);  // Top of lowercase 'o'
    int arrowTipY = whoY - 15;
    
    // Arrow shaft (thicker)
    for (int i = 0; i < 5; i++) {
        tft.drawLine(arrowX + i, arrowBaseY, arrowX + i, arrowTipY, TFT_WHITE);
    }
    
    // Arrow head (thicker)
    int arrowSize = 14;
    for (int i = 0; i < 4; i++) {
        tft.drawLine(arrowX + 2, arrowTipY + i, arrowX + 2 - arrowSize, arrowTipY + arrowSize + i, TFT_WHITE);
        tft.drawLine(arrowX + 2, arrowTipY + i, arrowX + 2 + arrowSize, arrowTipY + arrowSize + i, TFT_WHITE);
    }
    
    // "Edition" text below
    tft.setTextColor(COLOR_MEDIUM_BLUE, COLOR_BG);
    tft.setTextFont(4);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("Edition", SCREEN_W/2, whoY + whoScale * 13 + 20);
    
    // NOW draw "RHYTHM" diagonally - ON TOP
    const char* rhythm = "RHYTHM";
    int rhythmLen = strlen(rhythm);
    for (int i = 0; i < rhythmLen; i++) {
        float t = (float)i / (rhythmLen - 1);
        int x = 10 + t * (SCREEN_W - 50);
        int y = 10 + t * (SCREEN_H - 50);
        char letter[2] = {rhythm[i], '\0'};
        rr_drawVecText(tft, letter, x, y, diagonalScale, TFT_RED);
    }
    
    // NOW draw "RUNNER" diagonally from lower-left to upper-right - ON TOP
    const char* runner = "RUNNER";
    int runnerLen = strlen(runner);
    for (int i = 0; i < runnerLen; i++) {
        float t = (float)i / (runnerLen - 1);
        int x = 10 + t * (SCREEN_W - 50);
        int y = (SCREEN_H - 50) - t * (SCREEN_H - 60);
        char letter[2] = {runner[i], '\0'};
        rr_drawVecText(tft, letter, x, y, diagonalScale, TFT_RED);
    }
    
    // Blinking "Press button to continue" at bottom
    int lastBtn = HIGH;
    bool buttonPressed = false;
    unsigned long lastBlinkTime = millis();
    bool textVisible = true;
    
    while (!buttonPressed) {
        if (millis() - lastBlinkTime > 500) {
            lastBlinkTime = millis();
            textVisible = !textVisible;
            
            if (textVisible) {
                tft.setTextColor(TFT_WHITE, COLOR_BG);
                tft.setTextFont(2);
                tft.setTextDatum(BC_DATUM);
                tft.drawString("Press button to continue", SCREEN_W/2, SCREEN_H - 10);
            } else {
                tft.fillRect(SCREEN_W/2 - 120, SCREEN_H - 25, 240, 25, COLOR_BG);
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
}  // ← CLOSING BRACE FOR rr_showSplash

// Draw mini "The Who" logo for gameplay
void rr_drawMiniWhoLogo(TFT_eSPI &tft, int x, int y) {
    int logoScale = 2;
    int theScale = 1;
    
    const char* theText = "The";
    int theX = x;
    rr_drawVecText(tft, theText, theX, y, theScale, TFT_WHITE);
    
    int theHx = theX + (1 * 9 * theScale);
    int theHrightLeg = theHx + (6 * theScale);
    int theHy = y + (12 * theScale);
    
    int whoY = y + 18;
    
    int whoWx = theX - 5;
    for (int ox = 0; ox < 2; ox++) {
        for (int oy = 0; oy < 2; oy++) {
            rr_drawVecLetter(tft, 'W', whoWx + ox, whoY + oy, logoScale, TFT_WHITE);
        }
    }
    
    int whoHx = theHrightLeg;
    for (int ox = 0; ox < 2; ox++) {
        for (int oy = 0; oy < 2; oy++) {
            rr_drawVecLetter(tft, 'h', whoHx + ox, whoY + oy, logoScale, TFT_WHITE);
        }
    }
    
// Draw 'o' - closer to 'h'
    int whoOx = whoHx + (8 * logoScale) - 2;  // Changed from +5 to -2 (7 pixels closer)
        for (int ox = 0; ox < 2; ox++) {
        for (int oy = 0; oy < 2; oy++) {
            rr_drawVecLetter(tft, 'o', whoOx + ox, whoY + oy, logoScale, TFT_WHITE);
        }
    }
    
    for (int i = 0; i < 2; i++) {
        tft.drawLine(theHrightLeg + i, theHy, whoHx + i, whoY, TFT_WHITE);
    }
    
    int arrowX = whoOx + (4 * logoScale);
    int arrowBaseY = whoY + (6 * logoScale);
    int arrowTipY = whoY - 8;
    
    for (int i = 0; i < 3; i++) {
        tft.drawLine(arrowX + i, arrowBaseY, arrowX + i, arrowTipY, TFT_WHITE);
    }
    
    int arrowSize = 7;
    for (int i = 0; i < 2; i++) {
        tft.drawLine(arrowX + 1, arrowTipY + i, arrowX + 1 - arrowSize, arrowTipY + arrowSize + i, TFT_WHITE);
        tft.drawLine(arrowX + 1, arrowTipY + i, arrowX + 1 + arrowSize, arrowTipY + arrowSize + i, TFT_WHITE);
    }
}


int rr_songSelectionMenu(TFT_eSPI &tft) {
    tft.fillScreen(COLOR_BG);
    
// Title with spacing
    int scale = 3;
    int extraSpacing = 1;  // Extra pixels between letters (reduced from 3 to 1)
    const char* title = "CHOOSE SONG";
    int titleW = rr_vecTextWidthSpaced(title, scale, extraSpacing);
    rr_drawVecTextSpaced(tft, title, (SCREEN_W - titleW) / 2, 10, scale, TFT_RED, extraSpacing);
    
    int selectedIndex = 0;
    int lastSelectedIndex = -1;  // Track last selection
    extern volatile int rotaryPos;
    int lastRotary = rotaryPos;
    int lastBtn = HIGH;
    
    while (true) {
        // Only redraw if selection changed
        if (selectedIndex != lastSelectedIndex) {
            // Clear list area
            tft.fillRect(0, 60, SCREEN_W, SCREEN_H - 60, COLOR_BG);
            
            // Show 5 songs at a time
            int startIdx = selectedIndex - 2;
            if (startIdx < 0) startIdx = 0;
            if (startIdx > NUM_SONGS - 5) startIdx = NUM_SONGS - 5;
            if (NUM_SONGS < 5) startIdx = 0;
            
            for (int i = 0; i < 5 && (startIdx + i) < NUM_SONGS; i++) {
                int idx = startIdx + i;
                int yPos = 70 + i * 30;
                
            if (idx == selectedIndex) {
                    // Selected - highlight
                    tft.fillRect(10, yPos - 2, SCREEN_W - 20, 24, TFT_WHITE);
                    tft.setTextColor(TFT_BLACK, TFT_WHITE);
                } else {
                    tft.setTextColor(COLOR_MEDIUM_BLUE, COLOR_BG);
                }
                
                tft.setTextFont(2);
                tft.setTextDatum(CL_DATUM);
                tft.drawString(songTitles[idx], 15, yPos + 10);
            }
            
            lastSelectedIndex = selectedIndex;
        }
        
        // Handle rotary input
        int rotDiff = rotaryPos - lastRotary;
        if (rotDiff > 2) {
            selectedIndex++;
            if (selectedIndex >= NUM_SONGS) selectedIndex = NUM_SONGS - 1;
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
            delay(200);  // Debounce before starting game
            return selectedIndex;
        }
        lastBtn = btn;
        
        delay(50);
    }
}

// Spawn a new note
void rr_spawnNote(int lane, bool isHold, bool isDouble) {
    for (int i = 0; i < MAX_NOTES; i++) {
        if (!notes[i].active) {
            notes[i].lane = lane;
            notes[i].z = 1.0f;  // Start at far distance
            notes[i].active = true;
            notes[i].isHold = isHold;
            notes[i].isDouble = isDouble;
            notes[i].holdLength = isHold ? 0.25f : 0.0f;
            notes[i].hit = false;
            notes[i].holding = false;
            notes[i].hitsReceived = 0;
            return;
        }
    }
}

// Check if player hit a note
int rr_checkNoteHit(TFT_eSPI &tft, int lane) {
    int bestAccuracy = 0;
    int bestNote = -1;
    float bestDistance = 999.0f;
    
    for (int i = 0; i < MAX_NOTES; i++) {
        if (!notes[i].active || notes[i].hit) continue;
        if (notes[i].lane != lane) continue;
        
        // Calculate note's Y position
        int nx, ny, nw;
        rr_getLanePosition(notes[i].lane, notes[i].z, nx, ny, nw);
        
        float distance = abs(ny - HIT_ZONE_Y);
        
        if (distance < bestDistance && distance < TIMING_RED) {
            bestDistance = distance;
            bestNote = i;
            
            if (distance <= TIMING_GOLD) bestAccuracy = 4;
            else if (distance <= TIMING_GREEN) bestAccuracy = 3;
            else if (distance <= TIMING_YELLOW) bestAccuracy = 2;
            else bestAccuracy = 1;
        }
    }
    
if (bestNote >= 0) {
        notes[bestNote].hitAccuracy = bestAccuracy;  // Store initial accuracy
        
        if (notes[bestNote].isDouble) {
            // Double note - increment hit count
            notes[bestNote].hitsReceived++;
            
            if (notes[bestNote].hitsReceived >= 2) {
                // Both hits received!
                rr_drawNote(tft, notes[bestNote].lane, notes[bestNote].z, notes[bestNote].isHold, notes[bestNote].isDouble, notes[bestNote].holdLength, true);
                notes[bestNote].active = false;
            }
            // Note stays visible for second hit
            notes[bestNote].hit = true;
        } else if (notes[bestNote].isHold) {
            // Hold note - DON'T erase it, just mark as holding
            notes[bestNote].hit = true;
            notes[bestNote].holding = true;
} else {
            // Regular note - erase immediately and deactivate
            rr_drawNote(tft, notes[bestNote].lane, notes[bestNote].z, notes[bestNote].isHold, notes[bestNote].isDouble, notes[bestNote].holdLength, true);
            notes[bestNote].active = false;
        }
        return bestAccuracy;
    }
    
    return 0;  // Miss
}

// Update note positions
void rr_updateNotes(TFT_eSPI &tft, float deltaZ) {
    for (int i = 0; i < MAX_NOTES; i++) {
        if (!notes[i].active) continue;
        
// Erase at old position (including hold notes that are being held)
        rr_drawNote(tft, notes[i].lane, notes[i].z, notes[i].isHold, notes[i].isDouble, notes[i].holdLength, true);
        
        // Move toward player
        notes[i].z -= deltaZ;
        
        // Check if note passed the hit zone (miss)
        // For hold notes being held, keep them active longer
        if (notes[i].isHold && notes[i].holding) {
            if (notes[i].z < -0.5f) {
                notes[i].active = false;
            }
        } else {
            if (notes[i].z < -0.2f) {
                notes[i].active = false;
                continue;
            }
        }
        
// Draw at new position (even if holding)
        if (notes[i].z >= -0.3f && notes[i].z <= 1.0f) {
            rr_drawNote(tft, notes[i].lane, notes[i].z, notes[i].isHold, notes[i].isDouble, notes[i].holdLength, false);
        }
    }
}

// Main game function
void run_Rhythm_Runner(TFT_eSPI &tft) {
    tft.setRotation(3);
    pinMode(PIN_KO, INPUT_PULLUP);
    pinMode(PIN_PUSH, INPUT_PULLUP);
    
    // Show splash screen with animation (only first time)
    rr_showSplash(tft);
    
    // Game state that persists across songs
    score = 0;
    
    // Main game loop - returns to song selection after each song
    while (true) {
        // Song selection menu
        selectedSong = rr_songSelectionMenu(tft);
        
        // Initialize game (but keep score!)
        tft.fillScreen(COLOR_BG);
        rr_initLanes();
        
        for (int i = 0; i < MAX_NOTES; i++) {
            notes[i].active = false;
        }
        
        lastAccuracy = 0;
        playerLane = LANE_CENTER;
        perfectHolds = 0;
        feverMode = false;
        targetLane = LANE_CENTER;
        laneTransition = 0.0f;
        currentActiveLane = LANE_CENTER;
        gameStartTime = millis();
        lastLaneChangeTime = gameStartTime;
        
        // Draw initial scene
        rr_drawAllLanes(tft);
        rr_drawHitZone(tft);
        rr_drawPlayer(tft, playerLane, 0.0f, false);
        rr_drawAccuracyCircle(tft, 0);
        rr_drawScore(tft, score);
        
        // Start selected song
        playSound(songFiles[selectedSong], true);
        delay(100);  // Give audio time to start
        
        // Game variables
        extern volatile int rotaryPos;
        int lastRotary = rotaryPos;
        int lastBtn = HIGH;
        unsigned long lastNoteSpawn = millis();
        unsigned long lastFrame = millis();
        bool buttonHeld = false;
        int prevPlayerX = -1;
        int prevPlayerY = -1;
        unsigned long perfectTextTime = 0;
        bool prevFeverMode = false;
        
        // Song play loop
        bool songEnded = false;
        while (!songEnded) {
            unsigned long now = millis();
            float deltaTime = (now - lastFrame) / 1000.0f;
            lastFrame = now;
            
            // Update audio and check if song ended
            extern AudioGeneratorWAV *wav;
            if (wav && wav->isRunning()) {
                updateAudio();
            } else {
                // Song ended - break to song selection
                songEnded = true;
                break;
            }
            
            // Check fever mode timer
            if (feverMode && (now - feverModeStart > FEVER_DURATION)) {
                feverMode = false;
            }
            
            // Change active lane periodically (simulating music changes)
            if (now - lastLaneChangeTime > random(2000, 4000)) {
                int oldLane = currentActiveLane;
                // Randomly move left or right
                if (currentActiveLane == LANE_CENTER) {
                    currentActiveLane = random(0, 2) ? LANE_LEFT : LANE_RIGHT;
                } else if (currentActiveLane == LANE_LEFT) {
                    currentActiveLane = random(0, 2) ? LANE_CENTER : LANE_RIGHT;
                } else {
                    currentActiveLane = random(0, 2) ? LANE_CENTER : LANE_LEFT;
                }
                
                // Redraw lanes with new active lane
                rr_drawLane(tft, oldLane, false, false);
                rr_drawLane(tft, currentActiveLane, true, false);
                
                lastLaneChangeTime = now;
            }
            
            // Spawn notes on active lane
            if (now - lastNoteSpawn > random(800, 1500)) {
                // Check if a hold note is already active
                bool holdNoteActive = false;
                for (int i = 0; i < MAX_NOTES; i++) {
                    if (notes[i].active && notes[i].isHold) {
                        holdNoteActive = true;
                        break;
                    }
                }
                
                // Determine note type: 20% hold (if none active), 20% double, 60% regular
                bool isHold = false;
                bool isDouble = false;
                
                if (!holdNoteActive) {
                    int noteType = random(0, 10);
                    if (noteType < 2) isHold = true;       // 20% hold
                    else if (noteType < 4) isDouble = true; // 20% double
                    // else regular (60%)
                } else {
                    // Can't spawn hold, so 25% double, 75% regular
                    if (random(0, 4) == 0) isDouble = true;
                }
                
                rr_spawnNote(currentActiveLane, isHold, isDouble);
                lastNoteSpawn = now;
            }
            
            // Handle rotary input (lane switching)
            int rotDiff = rotaryPos - lastRotary;
            lastRotary = rotaryPos;
            
            if (rotDiff != 0 && !isMoving) {
                int newLane = targetLane;
                if (rotDiff > 0 && targetLane < LANE_RIGHT) newLane = targetLane + 1;
                else if (rotDiff < 0 && targetLane > LANE_LEFT) newLane = targetLane - 1;
                
                if (newLane != targetLane) {
                    playerLane = targetLane;
                    targetLane = newLane;
                    isMoving = true;
                    laneTransition = 0.0f;
                }
            }
            
            // Update lane transition
            if (isMoving) {
                laneTransition += deltaTime * 8.0f;  // Fast transition
                if (laneTransition >= 1.0f) {
                    laneTransition = 0.0f;
                    playerLane = targetLane;
                    isMoving = false;
                }
            }
            
            // Handle button input
            int btn = digitalRead(PIN_KO);
            if (btn == LOW && lastBtn == HIGH) {
                // Button pressed - check for hit
                int accuracy = rr_checkNoteHit(tft, playerLane);
                if (accuracy > 0) {
                    lastAccuracy = accuracy;
                    
                    // Award points based on accuracy
                    int points = 0;
                    if (accuracy == 4) points = 100;
                    else if (accuracy == 3) points = 75;
                    else if (accuracy == 2) points = 50;
                    else if (accuracy == 1) points = 25;
                    
                    // Double points during fever mode
                    if (feverMode) {
                        points *= 2;
                    }
                    
                    score += points;
                    rr_drawScore(tft, score);
                    rr_drawAccuracyCircle(tft, accuracy);
                    lastHitTime = now;
                }
                buttonHeld = true;
            }
            
            if (btn == HIGH && lastBtn == LOW) {
                buttonHeld = false;
                
                // Check if any hold notes were being held - release them
                for (int i = 0; i < MAX_NOTES; i++) {
                    if (notes[i].active && notes[i].holding) {
                        // Player released button - erase and end the hold
                        rr_drawNote(tft, notes[i].lane, notes[i].z, notes[i].isHold, notes[i].isDouble, notes[i].holdLength, true);
                        notes[i].holding = false;
                        notes[i].active = false;
                    }
                }
            }
            
            lastBtn = btn;
            
            // Clear accuracy circle after a moment
            if (lastAccuracy > 0 && now - lastHitTime > 500) {
                lastAccuracy = 0;
                rr_drawAccuracyCircle(tft, 0);
            }
            
            // Update note positions
            float noteSpeed = 0.015f;  // Adjust for difficulty
            rr_updateNotes(tft, noteSpeed);
            
            // Update hold notes while button is held
            if (buttonHeld) {
                for (int i = 0; i < MAX_NOTES; i++) {
                    if (!notes[i].active || !notes[i].holding) continue;
                    
                    // Check if hold note FRONT has passed well beyond hit zone
                    int frontX, frontY, frontW;
                    rr_getLanePosition(notes[i].lane, notes[i].z, frontX, frontY, frontW);
                    
                    // Check if the hold note tail is approaching completion
                    int tailX, tailY, tailW;
                    float tailZ = notes[i].z + notes[i].holdLength;
                    rr_getLanePosition(notes[i].lane, tailZ, tailX, tailY, tailW);
                    
                    // Tail is near or past hit zone (very forgiving window)
                    if (tailY > HIT_ZONE_Y - 5) {
                        // Successfully held the entire note!
                        int holdPoints = 200;
                        
                        // Double points during fever mode
                        if (feverMode) {
                            holdPoints *= 2;
                        }
                        
                        // Check if it was a PERFECT hold (use stored initial accuracy)
                        if (notes[i].hitAccuracy == 4) {
                            // PERFECT HOLD!
                            perfectHolds++;
                            if (perfectHolds > 3) perfectHolds = 3;
                            
                            // Visual feedback
                            tft.setTextColor(TFT_YELLOW, COLOR_BG);
                            tft.setTextFont(2);
                            tft.setTextDatum(TC_DATUM);
                            tft.drawString("PERFECT!", SCREEN_W/2, 80);
                            perfectTextTime = millis();
                            
                            // Lightning explosion effect
                            int explosionX = frontX;
                            int explosionY = frontY;
                            
                            for (int frame = 0; frame < 6; frame++) {
                                if (frame > 0) {
                                    tft.fillCircle(explosionX, explosionY, 35, COLOR_BG);
                                }
                                rr_drawLightningExplosion(tft, explosionX, explosionY, frame);
                                delay(50);
                            }
                            
                            // Clean up final frame
                            tft.fillCircle(explosionX, explosionY, 35, COLOR_BG);
                            
                            // Check if fever mode should activate
                            if (perfectHolds == 3 && !feverMode) {
                                feverMode = true;
                                feverModeStart = millis();
                                perfectHolds = 0;
                                
                                // Visual feedback for fever mode start
                                tft.setTextColor(TFT_YELLOW, COLOR_BG);
                                tft.setTextFont(4);
                                tft.setTextDatum(MC_DATUM);
                                tft.drawString("FEVER MODE!", SCREEN_W/2, 60);
                                delay(500);
                                tft.fillRect(0, 40, SCREEN_W, 40, COLOR_BG);
                            }
                        }
                        
                        // Erase note and award points
                        rr_drawNote(tft, notes[i].lane, notes[i].z, notes[i].isHold, notes[i].isDouble, notes[i].holdLength, true);
                        notes[i].active = false;
                        notes[i].holding = false;
                        
                        score += holdPoints;
                        rr_drawScore(tft, score);
                    }
                }
            }
            
            // Redraw UI elements that notes may have erased
            rr_drawAllLanes(tft);
            
            // Draw mini "The Who" logo on left side
            rr_drawMiniWhoLogo(tft, 18, SCREEN_H / 3 - 10);
            
            rr_drawHitZone(tft);
            rr_drawAccuracyCircle(tft, lastAccuracy);
            
            // Calculate current player position
            int x1, y1, w1, x2, y2, w2;
            rr_getLanePosition(playerLane, 0.0f, x1, y1, w1);
            rr_getLanePosition(targetLane, 0.0f, x2, y2, w2);
            int playerX = x1 + (x2 - x1) * laneTransition;
            int playerY = HIT_ZONE_Y;
            
            // Clear previous player position if it moved OR fever mode changed
            if (prevPlayerX >= 0 && (prevPlayerX != playerX || prevPlayerY != playerY || prevFeverMode != feverMode)) {
                int clearSize = prevFeverMode ? 30 : 15;
                tft.fillRect(prevPlayerX - clearSize, prevPlayerY - clearSize, clearSize * 2, clearSize * 2, COLOR_BG);
            }
            
            // Redraw lanes and UI
            rr_drawAllLanes(tft);
            rr_drawHitZone(tft);
            
            // Draw player at new position
            if (feverMode) {
                rr_drawPlayerElectrified(tft, playerLane, laneTransition);
            } else {
                rr_drawPlayer(tft, playerLane, laneTransition, false);
            }
            
            // Store position for next frame
            prevPlayerX = playerX;
            prevPlayerY = playerY;
            prevFeverMode = feverMode;
            
            // Clear PERFECT text after delay
            if (perfectTextTime > 0 && (now - perfectTextTime) > 600) {
                tft.fillRect(0, 75, SCREEN_W, 20, COLOR_BG);
                rr_drawAllLanes(tft);
                rr_drawHitZone(tft);
                perfectTextTime = 0;
            }
            
            // Draw diamond indicators
            rr_drawAllDiamondIndicators(tft, perfectHolds, feverMode);
            
            delay(16);  // ~60 FPS
        }
        
        // Song ended - show "SONG COMPLETE!" message
        tft.fillRect(0, SCREEN_H/2 - 30, SCREEN_W, 60, COLOR_BG);
        tft.setTextColor(TFT_YELLOW, COLOR_BG);
        tft.setTextFont(4);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("SONG COMPLETE!", SCREEN_W/2, SCREEN_H/2 - 10);
        
        char scoreBuf[32];
        snprintf(scoreBuf, sizeof(scoreBuf), "Score: %d", score);
        tft.setTextFont(2);
        tft.drawString(scoreBuf, SCREEN_W/2, SCREEN_H/2 + 20);
        
        delay(2000);
        
        // Loop back to song selection with preserved score
    }
}

#endif // RHYTHM_RUNNER_H