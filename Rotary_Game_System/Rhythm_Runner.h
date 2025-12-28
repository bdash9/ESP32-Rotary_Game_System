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
#define TIMING_GOLD 8
#define TIMING_GREEN 15
#define TIMING_YELLOW 25
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

// Note structure
struct Note {
    int lane;           // Which lane (0, 1, 2)
    float z;            // Distance from player (0 = at player, 1 = far away)
    bool active;
    bool isHold;        // Is this a hold note?
    float holdLength;   // How long to hold (in z units)
    bool hit;           // Has player hit this note?
    bool holding;       // Is player currently holding this note?
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

// Spawn a new note
void rr_spawnNote(int lane, bool isHold) {
    for (int i = 0; i < MAX_NOTES; i++) {
        if (!notes[i].active) {
            notes[i].lane = lane;
            notes[i].z = 1.0f;  // Start at far distance
            notes[i].active = true;
            notes[i].isHold = isHold;
notes[i].holdLength = isHold ? 0.4f : 0.0f;  // Longer hold notes
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
        
// Erase at old position
rr_drawNote(tft, notes[i].lane, notes[i].z, notes[i].isHold, notes[i].holdLength, true);      
        // Move toward player
        notes[i].z -= deltaZ;
        
        // Check if note passed the hit zone (miss)
        if (notes[i].z < -0.2f) {
            notes[i].active = false;
            continue;
        }
        
// Draw at new position
if (notes[i].z >= 0.0f && notes[i].z <= 1.0f) {
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
    
    // Main game loop
    while (true) {
        unsigned long now = millis();
        float deltaTime = (now - lastFrame) / 1000.0f;
        lastFrame = now;
        
        // Update audio if playing
        updateAudio();
        
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
            bool isHold = random(0, 5) == 0;  // 20% chance of hold note
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
            rr_drawPlayer(tft, playerLane, laneTransition, true);
            laneTransition += deltaTime * 8.0f;  // Fast transition
            if (laneTransition >= 1.0f) {
                laneTransition = 0.0f;
                playerLane = targetLane;
                isMoving = false;
            }
            rr_drawPlayer(tft, playerLane, laneTransition, false);
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
                
                // Check if hold note tail has passed hit zone
                int tailX, tailY, tailW;
                float tailZ = notes[i].z + notes[i].holdLength;
                rr_getLanePosition(notes[i].lane, tailZ, tailX, tailY, tailW);
                
if (tailY < HIT_ZONE_Y - 20) {
                    // Successfully held the entire note!
                    // Erase it before deactivating
                    rr_drawNote(tft, notes[i].lane, notes[i].z, notes[i].isHold, notes[i].holdLength, true);
                    notes[i].active = false;
                    notes[i].holding = false;
                    
                    // Bonus points for completing hold
                    score += 150;
                    rr_drawScore(tft, score);
                }
            }
        }
        
        // Redraw UI elements that notes may have erased
        rr_drawAllLanes(tft);  // ← ADD THIS LINE - Redraw lanes
        rr_drawHitZone(tft);
        rr_drawAccuracyCircle(tft, lastAccuracy);
        
        // Redraw player (in case notes erased it)
        rr_drawPlayer(tft, playerLane, laneTransition, false);
        
        delay(16);  // ~60 FPS
    }
}

#endif // RHYTHM_RUNNER_H