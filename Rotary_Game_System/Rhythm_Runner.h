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
    float holdLength;   // How long to hold (in z units)
    bool hit;           // Has player hit this note?
    bool holding;       // Is player currently holding this note?
    int hitAccuracy;    // Accuracy when first hit (1-4)
};

Note notes[MAX_NOTES];

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
void rr_drawNote(TFT_eSPI &tft, int lane, float z, bool isHold, float holdLength, bool erase = false) {
    int x, y, width;
    rr_getLanePosition(lane, z, x, y, width);
    
    uint16_t color = erase ? COLOR_BG : (isHold ? COLOR_NOTE_HOLD : COLOR_NOTE);
    
    // Draw diamond note at front
    int size = 8 * (1.0f - z * 0.7f);  // Shrinks with distance
    if (size < 3) size = 3;
    
    if (isHold) {
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
    int y = SCREEN_H - 25;
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

// Spawn a new note
void rr_spawnNote(int lane, bool isHold) {
    for (int i = 0; i < MAX_NOTES; i++) {
        if (!notes[i].active) {
            notes[i].lane = lane;
            notes[i].z = 1.0f;  // Start at far distance
            notes[i].active = true;
            notes[i].isHold = isHold;
notes[i].holdLength = isHold ? 0.25f : 0.0f;  // Match visual to actual duration
            notes[i].hit = false;
            notes[i].holding = false;
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
        notes[bestNote].hit = true;
        notes[bestNote].hitAccuracy = bestAccuracy;  // Store initial accuracy
        
        if (notes[bestNote].isHold) {
            // Hold note - DON'T erase it, just mark as holding
            // It will continue to render as it moves through hit zone
            notes[bestNote].holding = true;
        } else {
            // Regular note - erase immediately and deactivate
            rr_drawNote(tft, notes[bestNote].lane, notes[bestNote].z, notes[bestNote].isHold, notes[bestNote].holdLength, true);
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
        rr_drawNote(tft, notes[i].lane, notes[i].z, notes[i].isHold, notes[i].holdLength, true);
        
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
            rr_drawNote(tft, notes[i].lane, notes[i].z, notes[i].isHold, notes[i].holdLength, false);
        }
    }
}

// Main game function
void run_Rhythm_Runner(TFT_eSPI &tft) {
    tft.setRotation(3);
    pinMode(PIN_KO, INPUT_PULLUP);
    pinMode(PIN_PUSH, INPUT_PULLUP);
    
    // Splash screen
    tft.fillScreen(COLOR_BG);
    tft.setTextColor(TFT_CYAN, COLOR_BG);
    tft.setTextFont(4);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("RHYTHM RUNNER", SCREEN_W/2, SCREEN_H/2 - 20);
    tft.setTextFont(2);
    tft.drawString("Press button to start", SCREEN_W/2, SCREEN_H/2 + 20);
    
    // Wait for button
    while (digitalRead(PIN_KO) == HIGH) delay(10);
    while (digitalRead(PIN_KO) == LOW) delay(10);
    
    // Initialize game
    tft.fillScreen(COLOR_BG);
    rr_initLanes();
    
    for (int i = 0; i < MAX_NOTES; i++) {
        notes[i].active = false;
    }
    
    score = 0;
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
    
    // Main game loop
    while (true) {
        unsigned long now = millis();
        float deltaTime = (now - lastFrame) / 1000.0f;
        lastFrame = now;
        
        // Update audio if playing
        updateAudio();
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
            
            // Only spawn hold notes if none are active (prevent overlap)
            bool isHold = !holdNoteActive && (random(0, 5) == 0);  // 20% chance if no hold active
            rr_spawnNote(currentActiveLane, isHold);
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
                    rr_drawNote(tft, notes[i].lane, notes[i].z, notes[i].isHold, notes[i].holdLength, true);
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
                // (meaning the entire elongated note has been held through)
                int frontX, frontY, frontW;
                rr_getLanePosition(notes[i].lane, notes[i].z, frontX, frontY, frontW);
                
// Check if the hold note tail is approaching completion
                int tailX, tailY, tailW;
                float tailZ = notes[i].z + notes[i].holdLength;
                rr_getLanePosition(notes[i].lane, tailZ, tailX, tailY, tailW);
                
                // Tail is near or past hit zone (very forgiving window)
                if (tailY > HIT_ZONE_Y - 5) {  // Can release slightly early
                    // Successfully held the entire note!
                    int holdPoints = 200;
                    
                    // Double points during fever mode
                    if (feverMode) {
                        holdPoints *= 2;
                    }
                    
// Check if it was a PERFECT hold (use stored initial accuracy)
                    if (notes[i].hitAccuracy == 4) {  // Check stored accuracy
                        // PERFECT HOLD!
                        perfectHolds++;
                        if (perfectHolds > 3) perfectHolds = 3;
                        
                        // Visual feedback
                        tft.setTextColor(TFT_YELLOW, COLOR_BG);
                        tft.setTextFont(2);
                        tft.setTextDatum(TC_DATUM);
                        tft.drawString("PERFECT!", SCREEN_W/2, 80);
                        perfectTextTime = millis();  // Record when text was shown
                        
                        // Lightning explosion effect
                        int explosionX = frontX;
                        int explosionY = frontY;
                        
                        for (int frame = 0; frame < 6; frame++) {
                            // Erase previous frame
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
                            perfectHolds = 0;  // Reset counter
                            
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
                    rr_drawNote(tft, notes[i].lane, notes[i].z, notes[i].isHold, notes[i].holdLength, true);
                    notes[i].active = false;
                    notes[i].holding = false;
                    
                    score += holdPoints;
                    rr_drawScore(tft, score);
                }
            }
        }
        
// Redraw UI elements that notes may have erased
        rr_drawAllLanes(tft);  // Redraw lanes
        
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
            // Use larger clear size if we were in fever mode (to clear electric effects)
            int clearSize = prevFeverMode ? 30 : 15;  // Use PREVIOUS fever state
            tft.fillRect(prevPlayerX - clearSize, prevPlayerY - clearSize, clearSize * 2, clearSize * 2, COLOR_BG);
        }
        
        // Redraw lanes and UI (these draw under the player)
        rr_drawAllLanes(tft);
        rr_drawHitZone(tft);
        
        // Draw player at new position (electrified during fever mode)
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
            // Redraw lanes to clean up any remnants
            rr_drawAllLanes(tft);
            rr_drawHitZone(tft);
            perfectTextTime = 0;
        }
        
        // Draw diamond indicators (after clearing perfect text)
        rr_drawAllDiamondIndicators(tft, perfectHolds, feverMode);
        
        delay(16);  // ~60 FPS
    }
}

#endif // RHYTHM_RUNNER_H