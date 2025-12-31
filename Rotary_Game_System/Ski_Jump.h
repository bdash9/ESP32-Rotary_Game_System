#ifndef SKI_JUMP_H
#define SKI_JUMP_H

#include <Arduino.h>
#include <TFT_eSPI.h>

extern void playSound(const char *path, bool stopCurrent);
extern void updateAudio();
extern volatile int rotaryPos;

#define SCREEN_W 320
#define SCREEN_H 240

#ifndef PIN_KO
#define PIN_KO 22
#endif

// Colors
#define COLOR_SKY 0x5D9F
#define COLOR_SNOW 0xFFFF
#define COLOR_TREE_TRUNK 0x4208
#define COLOR_TREE_GREEN 0x2444
#define COLOR_CLOUD 0xFFFF
#define JUMP_HORIZON_Y 45

// Olympic Ring Colors
#define OLYMPIC_BLUE 0x001F
#define OLYMPIC_YELLOW 0xFFE0
#define OLYMPIC_BLACK 0x0000
#define OLYMPIC_GREEN 0x07E0
#define OLYMPIC_RED 0xF800

// Game states
enum JumpPhase {
    PHASE_READY,
    PHASE_DESCENDING,
    PHASE_JUMPING,
    PHASE_FLYING,
    PHASE_LANDING,
    PHASE_SKIDDING
};

float jumperAngle = -0.35f;  // Player-controlled angle
float targetAngle = -0.35f;   // Target for best performance (about -20 degrees)

// SkiJumper
struct SkiJumper {
    float x, y;
    float velocityX, velocityY;
    float speed;
    float distance;
    float landMarkX;
    bool crouching;
    JumpPhase phase;
    float angleScore;  // Track how well they maintained angle
    int angleFrames;   // Number of frames in air
};

SkiJumper jumper;

// Cloud positions
struct Cloud {
    float x, y;
    int size;
};
Cloud clouds[5];

//=============================================================================
// VECTOR FONT (subset for "SKI JUMP")
//=============================================================================
struct SJ_VecStroke { int x0, y0, x1, y1; };
struct SJ_VecLetter { const SJ_VecStroke* strokes; int n; };

const SJ_VecStroke sj_strokes_S[] = {{8,0,0,0},{0,0,0,6},{0,6,8,6},{8,6,8,12},{8,12,0,12}};
const SJ_VecStroke sj_strokes_K[] = {{0,0,0,12},{8,0,0,6},{0,6,8,12}};
const SJ_VecStroke sj_strokes_I[] = {{4,0,4,12},{2,0,6,0},{2,12,6,12}};
const SJ_VecStroke sj_strokes_J[] = {{8,0,8,10},{8,10,6,12},{6,12,2,12},{2,12,0,10}};
const SJ_VecStroke sj_strokes_U[] = {{0,0,0,12},{0,12,8,12},{8,12,8,0}};
const SJ_VecStroke sj_strokes_M[] = {{0,12,0,0},{0,0,4,6},{4,6,8,0},{8,0,8,12}};
const SJ_VecStroke sj_strokes_P[] = {{0,12,0,0},{0,0,8,0},{8,0,8,6},{8,6,0,6}};

const SJ_VecLetter sj_vecFont[] = {
    {sj_strokes_S,5}, {sj_strokes_K,3}, {sj_strokes_I,3}, {sj_strokes_J,4},
    {sj_strokes_U,3}, {sj_strokes_M,4}, {sj_strokes_P,4}
};

int sj_getVecFontIdx(char c) {
    switch(c) {
        case 'S': return 0; case 'K': return 1; case 'I': return 2; case 'J': return 3;
        case 'U': return 4; case 'M': return 5; case 'P': return 6;
        default: return -1;
    }
}

void sj_drawVecLetter(TFT_eSPI &tft, char c, int x, int y, int scale, uint16_t color) {
    int idx = sj_getVecFontIdx(c);
    if(idx < 0) return;
    for (int i = 0; i < sj_vecFont[idx].n; i++) {
        int x0 = x + sj_vecFont[idx].strokes[i].x0 * scale;
        int y0 = y + sj_vecFont[idx].strokes[i].y0 * scale;
        int x1 = x + sj_vecFont[idx].strokes[i].x1 * scale;
        int y1 = y + sj_vecFont[idx].strokes[i].y1 * scale;
        tft.drawLine(x0, y0, x1, y1, color);
        tft.drawLine(x0+1, y0, x1+1, y1, color);
    }
}

void sj_drawVecText(TFT_eSPI &tft, const char* text, int x, int y, int scale, uint16_t color) {
    int spacing = 10 * scale;
    int cx = x;
    for (int i = 0; text[i]; i++) {
        if (text[i] != ' ') {
            sj_drawVecLetter(tft, text[i], cx, y, scale, color);
        }
        cx += spacing;
    }
}

//=============================================================================
// OLYMPIC RINGS
//=============================================================================
void sj_drawOlympicRing(TFT_eSPI &tft, int cx, int cy, int radius, uint16_t color) {
    for (int i = 0; i < 4; i++) {
        tft.drawCircle(cx, cy, radius + i, color);
    }
}

void sj_drawOlympicRings(TFT_eSPI &tft, int centerX, int centerY, int ringRadius) {
    int spacing = ringRadius * 2 + 8;
    int vertOffset = ringRadius / 2;
    
    sj_drawOlympicRing(tft, centerX - spacing, centerY, ringRadius, OLYMPIC_BLUE);
    sj_drawOlympicRing(tft, centerX, centerY, ringRadius, OLYMPIC_BLACK);
    sj_drawOlympicRing(tft, centerX + spacing, centerY, ringRadius, OLYMPIC_RED);
    sj_drawOlympicRing(tft, centerX - spacing/2, centerY + vertOffset, ringRadius, OLYMPIC_YELLOW);
    sj_drawOlympicRing(tft, centerX + spacing/2, centerY + vertOffset, ringRadius, OLYMPIC_GREEN);
}

void sj_drawMiniOlympicRings(TFT_eSPI &tft, int x, int y) {
    int r = 8;
    int spacing = r * 2 + 3;
    int vOff = r / 2;
    
    sj_drawOlympicRing(tft, x, y, r, OLYMPIC_BLUE);
    sj_drawOlympicRing(tft, x + spacing, y, r, OLYMPIC_BLACK);
    sj_drawOlympicRing(tft, x + spacing * 2, y, r, OLYMPIC_RED);
    sj_drawOlympicRing(tft, x + spacing/2, y + vOff, r, OLYMPIC_YELLOW);
    sj_drawOlympicRing(tft, x + spacing + spacing/2, y + vOff, r, OLYMPIC_GREEN);
}

//=============================================================================
// MEDAL DRAWING (with red necklace)
//=============================================================================
void sj_drawMedal(TFT_eSPI &tft, int x, int y, int medalType) {
    if (medalType == 0) return;
    
    uint16_t medalColor;
    const char* medalText;
    
    switch(medalType) {
        case 3: medalColor = 0xFEA0; medalText = "GOLD"; break;
        case 2: medalColor = 0xC618; medalText = "SILVER"; break;
        case 1: medalColor = 0xA285; medalText = "BRONZE"; break;
        default: return;
    }
    
    tft.fillRect(x - 3, y - 25, 6, 15, TFT_RED);
    tft.fillRect(x - 6, y - 35, 5, 15, TFT_RED);
    tft.fillRect(x + 1, y - 35, 5, 15, TFT_RED);
    
    tft.fillCircle(x, y, 25, medalColor);
    tft.drawCircle(x, y, 25, TFT_BLACK);
    tft.drawCircle(x, y, 24, TFT_BLACK);
    tft.drawCircle(x, y, 26, TFT_BLACK);
    tft.drawCircle(x, y, 18, TFT_BLACK);
    
    tft.setTextColor(TFT_BLACK, medalColor);
    tft.setTextFont(4);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(String(4 - medalType), x, y);
    
    tft.setTextColor(medalColor, TFT_BLACK);
    tft.setTextFont(2);
    tft.setTextDatum(TC_DATUM);
    tft.drawString(medalText, x, y + 32);
}

//=============================================================================
// ANGLE METER
void sj_drawAngleMeter(TFT_eSPI &tft, float angle) {
    int meterX = SCREEN_W - 30;
    int meterY = SCREEN_H / 2 - 60;
    int meterH = 120;
    int meterW = 15;
    
    // Background bar
    tft.fillRect(meterX, meterY, meterW, meterH, TFT_BLACK);
    tft.drawRect(meterX, meterY, meterW, meterH, TFT_WHITE);
    
    // Color zones (bottom to top)
    tft.fillRect(meterX + 2, meterY + meterH - 24, meterW - 4, 22, TFT_RED);
    tft.fillRect(meterX + 2, meterY + meterH - 52, meterW - 4, 26, TFT_YELLOW);
    tft.fillRect(meterX + 2, meterY + meterH - 80, meterW - 4, 26, TFT_GREEN);
    tft.fillRect(meterX + 2, meterY + meterH - 108, meterW - 4, 26, TFT_YELLOW);
    tft.fillRect(meterX + 2, meterY + 2, meterW - 4, 22, TFT_RED);
    
    // Calculate and draw indicator
    float normalizedAngle = (angle + 0.7f) / 0.7f;
    if (normalizedAngle < 0) normalizedAngle = 0;
    if (normalizedAngle > 1) normalizedAngle = 1;
    
    int indicatorY = meterY + meterH - (int)(normalizedAngle * meterH);
    
    // Erase old indicator by filling area with sky color BEFORE drawing new one
    tft.fillRect(meterX - 6, meterY - 5, 6, meterH + 10, COLOR_SKY);
    
    // Draw new indicator (white triangle)
    tft.fillTriangle(meterX - 5, indicatorY, 
                     meterX, indicatorY - 4,
                     meterX, indicatorY + 4, TFT_WHITE);
}

int sj_getAngleQuality(float angle) {
    // Return 0=red, 1=yellow, 2=green
    float diff = abs(angle - (-0.35f));  // Difference from ideal
    
    if (diff < 0.03f) return 2;  // Green - MUCH NARROWER (was 0.045f)
    else if (diff < 0.12f) return 1;  // Yellow - also narrower (was 0.15f)
    else return 0;  // Red - bad
}

//=============================================================================
// SCENERY
//=============================================================================
void sj_drawPineTree(TFT_eSPI &tft, int x, int y, int size) {
    int trunkWidth = size / 5;
    int trunkHeight = size / 2;
    tft.fillRect(x - trunkWidth/2, y, trunkWidth, trunkHeight, COLOR_TREE_TRUNK);
    
    int layerHeight = size / 3;
    int topY = y - size / 3;
    
    tft.fillTriangle(x, topY, x - size/2, topY + layerHeight, x + size/2, topY + layerHeight, COLOR_TREE_GREEN);
    topY += layerHeight / 2;
    tft.fillTriangle(x, topY, x - size/3, topY + layerHeight, x + size/3, topY + layerHeight, COLOR_TREE_GREEN);
    topY += layerHeight / 2;
    tft.fillTriangle(x, topY, x - size/4, topY + layerHeight, x + size/4, topY + layerHeight, COLOR_TREE_GREEN);
}

void sj_drawMountainPeaks(TFT_eSPI &tft) {
    int peakY = JUMP_HORIZON_Y;
    int numPeaks = 8;
    int peakSpacing = SCREEN_W / numPeaks;
    
    for (int i = 0; i < numPeaks; i++) {
        int peakX = i * peakSpacing + peakSpacing / 2;
        int peakHeight = 15 + (i % 3) * 10;
        int peakWidth = peakSpacing / 2;
        
        uint16_t snowPeak = tft.color565(255, 255, 255);
        tft.fillTriangle(peakX - peakWidth, peakY, peakX, peakY - peakHeight, peakX + peakWidth, peakY, snowPeak);
    }
}

void sj_drawMountainScene(TFT_eSPI &tft) {
    tft.fillScreen(COLOR_SKY);
    sj_drawMountainPeaks(tft);
    
    int horizonY = JUMP_HORIZON_Y;
    for (int y = horizonY; y < SCREEN_H; y++) {
        float t = (float)(y - horizonY) / (SCREEN_H - horizonY);
        uint16_t snowShade = tft.color565(255, 255, 255 - (int)(t * 40));
        tft.drawFastHLine(0, y, SCREEN_W, snowShade);
    }
    
    for (int i = 0; i < 6; i++) {
        int treeX = (i % 2 == 0) ? 20 + random(-10, 10) : SCREEN_W - 20 + random(-10, 10);
        int treeY = 50 + random(0, 150);
        int treeSize = 15 + random(0, 15);
        sj_drawPineTree(tft, treeX, treeY, treeSize);
    }
}

void sj_initClouds() {
    for (int i = 0; i < 5; i++) {
        clouds[i].x = random(0, SCREEN_W);
        clouds[i].y = random(20, 50);
        clouds[i].size = random(15, 30);
    }
}

void sj_drawClouds(TFT_eSPI &tft, float scrollOffset) {
    for (int i = 0; i < 5; i++) {
        int x = (int)(clouds[i].x - scrollOffset) % (SCREEN_W + 100);
        if (x < -50) x += SCREEN_W + 100;
        int y = clouds[i].y;
        int s = clouds[i].size;
        
        tft.fillCircle(x, y, s/2, COLOR_CLOUD);
        tft.fillCircle(x + s/2, y, s/3, COLOR_CLOUD);
        tft.fillCircle(x - s/2, y, s/3, COLOR_CLOUD);
    }
}

//=============================================================================
// RAMP DRAWING
//=============================================================================
void sj_drawRamp(TFT_eSPI &tft) {
    tft.fillRect(20, 60, 40, 5, TFT_DARKGREY);
    
    for (int i = 0; i < 3; i++) {
        tft.drawLine(40, 65 + i, 220, 180 + i, TFT_DARKGREY);
    }
    
    for (int x = 220; x < 280; x++) {
        float t = (x - 220) / 60.0f;
        int y = 180 - (int)(t * t * 40);
        tft.fillRect(x, y, 3, 3, TFT_WHITE);
        tft.drawPixel(x, y - 1, TFT_DARKGREY);
    }
    
    for (int i = 0; i < 5; i++) {
        int x = 220 + i * 12;
        int topY = 180 - (int)((i / 5.0f) * (i / 5.0f) * 40);
        tft.drawLine(x, topY, x, SCREEN_H - 20, TFT_DARKGREY);
    }
}

//=============================================================================
// JUMPER DRAWING
//=============================================================================
void sj_drawJumper(TFT_eSPI &tft, float x, float y, bool crouching, bool flying) {
    int ix = (int)x;
    int iy = (int)y;
    
    if (flying) {
        // Flying position - dynamic angle
        int skiLen = 24;
        float angle = jumperAngle;
        
        // Draw TWO skis at current angle
        int skiEndX = ix + (int)(skiLen * cos(angle));
        int skiEndY = iy + (int)(skiLen * sin(angle));
        
        // Left ski
        tft.drawLine(ix - 2, iy, skiEndX - 2, skiEndY, TFT_WHITE);
        tft.drawLine(ix - 2, iy + 1, skiEndX - 2, skiEndY + 1, TFT_WHITE);
        
        // Right ski
        tft.drawLine(ix + 2, iy, skiEndX + 2, skiEndY, TFT_WHITE);
        tft.drawLine(ix + 2, iy + 1, skiEndX + 2, skiEndY + 1, TFT_WHITE);
        
        // Body on top of skis, same angle
        int bodyLen = 16;
        int bodyStartX = ix + (int)(3 * cos(angle));
        int bodyStartY = iy + (int)(3 * sin(angle)) - 4;
        int bodyEndX = bodyStartX + (int)(bodyLen * cos(angle));
        int bodyEndY = bodyStartY + (int)(bodyLen * sin(angle));
        
        // Body (red jacket)
        for (int i = 0; i < 5; i++) {
            tft.drawLine(bodyStartX, bodyStartY + i, bodyEndX, bodyEndY + i, TFT_RED);
        }
        
        // Head at END
        tft.fillCircle(bodyEndX + 2, bodyEndY - 5, 4, TFT_YELLOW);
        
        // Arms extended forward
        int armX = bodyStartX + (int)(8 * cos(angle));
        int armY = bodyStartY + (int)(8 * sin(angle));
        tft.drawLine(armX, armY, armX - 8, armY - 10, TFT_RED);
        tft.drawLine(armX, armY + 3, armX - 8, armY - 7, TFT_RED);
        
    } else if (crouching) {
        // CROUCHING - lower and more compact
        tft.fillCircle(ix + 4, iy - 4, 3, TFT_YELLOW);  // Head closer to body
        tft.fillRect(ix - 2, iy - 2, 8, 5, TFT_RED);  // Body low and forward
        tft.drawLine(ix - 6, iy + 3, ix + 10, iy + 3, TFT_WHITE);  // Skis
        tft.drawLine(ix - 6, iy + 4, ix + 10, iy + 4, TFT_WHITE);
    } else {
        // UPRIGHT - clearly standing tall
        tft.fillCircle(ix, iy - 14, 4, TFT_YELLOW);  // Head high up
        tft.fillRect(ix - 3, iy - 10, 6, 10, TFT_RED);  // Torso upright
        tft.fillRect(ix - 3, iy, 3, 6, TFT_BLUE);  // Left leg
        tft.fillRect(ix, iy, 3, 6, TFT_BLUE);  // Right leg
        tft.drawLine(ix - 8, iy + 6, ix + 8, iy + 6, TFT_WHITE);  // Skis
        tft.drawLine(ix - 8, iy + 7, ix + 8, iy + 7, TFT_WHITE);
    }
}

//=============================================================================
// MAIN GAME
//=============================================================================
void run_Ski_Jump(TFT_eSPI &tft) {
    tft.setRotation(3);
    pinMode(PIN_KO, INPUT_PULLUP);
    
    // Splash screen
    sj_drawMountainScene(tft);
    
    // Title "SKI JUMP"
    const char* word1 = "SKI";
    const char* word2 = "JUMP";
    int scale = 3;
    int word1W = strlen(word1) * 10 * scale;
    int word2W = strlen(word2) * 10 * scale;
    int gap = 5;
    int totalW = word1W + gap + word2W;
    
    sj_drawVecText(tft, word1, (SCREEN_W - totalW) / 2, 75, scale, OLYMPIC_BLUE);
    sj_drawVecText(tft, word2, (SCREEN_W - totalW) / 2 + word1W + gap, 75, scale, OLYMPIC_BLUE);
    
    // Olympic rings
    sj_drawOlympicRings(tft, SCREEN_W / 2, 155, 18);
    
    // Wait for button
    tft.setTextColor(TFT_YELLOW, COLOR_SKY);
    tft.setTextFont(2);
    tft.setTextDatum(BC_DATUM);
    tft.drawString("Press button to start", SCREEN_W/2, SCREEN_H - 10);
    
    int lastBtn = HIGH;
    bool buttonPressed = false;
    while (!buttonPressed) {
        updateAudio();
        int btn = digitalRead(PIN_KO);
        if (btn == LOW && lastBtn == HIGH) {
            buttonPressed = true;
        }
        lastBtn = btn;
        delay(50);
    }
    
    while (digitalRead(PIN_KO) == LOW) delay(10);
    delay(300);
    
    // Draw game screen
    sj_drawMountainScene(tft);
    sj_drawRamp(tft);
    sj_drawMiniOlympicRings(tft, 15, 15);
    sj_drawJumper(tft, 50, 70, false, false);
    
    // Countdown
    for (int countdown = 3; countdown >= 0; countdown--) {
        if (countdown > 0) {
            tft.setTextColor(TFT_YELLOW, COLOR_SKY);
            tft.setTextFont(7);
            tft.setTextDatum(MC_DATUM);
            char countBuf[8];
            snprintf(countBuf, sizeof(countBuf), "%d", countdown);
            tft.drawString(countBuf, SCREEN_W/2, SCREEN_H/2);
            playSound("/sounds/beep.wav", false);
        } else {
            tft.setTextColor(TFT_YELLOW, COLOR_SKY);
            tft.setTextFont(7);
            tft.setTextDatum(MC_DATUM);
            tft.drawString("0", SCREEN_W/2, SCREEN_H/2);
            playSound("/sounds/beep_go.wav", false);
        }
        
        unsigned long countStart = millis();
        while (millis() - countStart < 1000) {
            updateAudio();
            delay(10);
        }
        
        tft.fillCircle(SCREEN_W/2, SCREEN_H/2, 40, COLOR_SNOW);
        sj_drawRamp(tft);
        sj_drawJumper(tft, 50, 70, false, false);
    }
    
    // GO!
    tft.setTextColor(TFT_GREEN, COLOR_SNOW);
    tft.setTextFont(7);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("GO!", SCREEN_W/2, SCREEN_H/2);
    delay(500);
    
    sj_drawMountainScene(tft);
    sj_drawRamp(tft);
    sj_drawMiniOlympicRings(tft, 15, 15);
    
    // Initialize
    jumper.x = 50;
    jumper.y = 70;
    jumper.speed = 3.75f;
    jumper.distance = 0;
    jumper.crouching = false;
    jumper.phase = PHASE_DESCENDING;
    jumper.landMarkX = 0;
    jumper.angleScore = 0;  
    jumper.angleFrames = 0;  
    
    jumperAngle = -0.35f;  // Reset angle
    
    sj_initClouds();
    
    unsigned long lastFrame = millis();
    lastBtn = HIGH;
    bool gameRunning = true;
    float cloudScroll = 0;
    
    float prevX = jumper.x;
    float prevY = jumper.y;
    
    float lastDisplayedSpeed = -1;  // Track last displayed speed to avoid blinking
    
    while (gameRunning) {
        unsigned long now = millis();
        float deltaTime = (now - lastFrame) / 1000.0f;
        if (deltaTime > 0.05f) deltaTime = 0.05f;
        lastFrame = now;
        
        int btn = digitalRead(PIN_KO);
        
        if (jumper.phase == PHASE_DESCENDING) {
            // Button pressed = crouching, not pressed = upright
            if (btn == LOW) {
                jumper.crouching = true;
                jumper.speed += 0.1f;
                if (jumper.speed > 12.5f) jumper.speed = 12.5f;
            } else {
                jumper.crouching = false;
            }
            
            jumper.x += jumper.speed * deltaTime * 18.75f;
            
            if (jumper.x < 220) {
                jumper.y = 65 + (jumper.x - 40) * 0.64f;
            } else {
                float t = (jumper.x - 220) / 60.0f;
                jumper.y = 180 - t * t * 40;
            }
            
if (jumper.x >= 280) {
                jumper.phase = PHASE_JUMPING;
                jumper.velocityX = jumper.speed;
                jumper.velocityY = -jumper.speed * 0.7f;
                
                // Score the jump timing - crouched at takeoff is BEST
                float jumpBonus = 1.0f;
                if (btn == LOW) {
                    // Perfect! Still crouched at takeoff
                    jumpBonus = 1.15f;  // 15% bonus
                    jumper.velocityY -= 2.0f;  // Extra height
                    playSound("/sounds/beep_go.wav", false);
                } else {
                    // Standing up at takeoff - penalty
                    jumpBonus = 0.9f;  // 10% penalty
                }
                
                // Store jump quality for final calculation
                jumper.angleScore = jumpBonus * 100;  // Start score with jump bonus
            }
            
            // Erase old position
            int horizonY = JUMP_HORIZON_Y;
            for (int ey = max(horizonY, (int)prevY - 20); ey < min(SCREEN_H, (int)prevY + 20); ey++) {
                float t = (float)(ey - horizonY) / (SCREEN_H - horizonY);
                uint16_t snowShade = tft.color565(255, 255, 255 - (int)(t * 40));
                tft.drawFastHLine((int)prevX - 20, ey, 40, snowShade);
            }
            
            // ALWAYS redraw FULL ramp sections
            if (prevX >= 35 && prevX <= 285) {
                // Redraw diagonal section
                for (int i = 0; i < 3; i++) {
                    tft.drawLine(40, 65 + i, 220, 180 + i, TFT_DARKGREY);
                }
                
                // Redraw curved jump section
                for (int x = 220; x < 280; x++) {
                    float t = (x - 220) / 60.0f;
                    int y = 180 - (int)(t * t * 40);
                    tft.fillRect(x, y, 3, 3, TFT_WHITE);
                    tft.drawPixel(x, y - 1, TFT_DARKGREY);
                }
            }
            
            sj_drawJumper(tft, jumper.x, jumper.y, jumper.crouching, false);
            
            // Only update speed display when it changes significantly
            if (abs(jumper.speed - lastDisplayedSpeed) > 0.2f) {
                char speedBuf[16];
                snprintf(speedBuf, sizeof(speedBuf), "%.1f m/s", jumper.speed);
                tft.setTextColor(TFT_WHITE, COLOR_SKY);
                tft.setTextFont(2);
                tft.setTextDatum(TR_DATUM);
                tft.fillRect(SCREEN_W - 90, 8, 80, 18, COLOR_SKY);
                tft.drawString(speedBuf, SCREEN_W - 10, 10);
                lastDisplayedSpeed = jumper.speed;
            }
            
            prevX = jumper.x;
            prevY = jumper.y;
            
} else if (jumper.phase == PHASE_JUMPING || jumper.phase == PHASE_FLYING) {
            jumper.phase = PHASE_FLYING;
            
            // Player controls angle with rotary knob - MORE SENSITIVE NOW
            static int lastRotaryFlying = rotaryPos;
            int rotDiff = rotaryPos - lastRotaryFlying;
            
            if (rotDiff > 0) {
                jumperAngle -= 0.03f;  // MORE SENSITIVE (was 0.02f)
                if (jumperAngle < -0.7f) jumperAngle = -0.7f;
                lastRotaryFlying = rotaryPos;
            } else if (rotDiff < 0) {
                jumperAngle += 0.03f;  // MORE SENSITIVE (was 0.02f)
                if (jumperAngle > 0.0f) jumperAngle = 0.0f;
                lastRotaryFlying = rotaryPos;
            }
            
// Add wind drift - angle slowly drifts requiring constant adjustment
            jumperAngle += 0.008f;  // MUCH FASTER drift (was 0.0055f)
            if (jumperAngle > 0.0f) jumperAngle = 0.0f;
            
// Track angle quality - STRICTER SCORING
            int quality = sj_getAngleQuality(jumperAngle);
            if (quality == 2) jumper.angleScore += 5;  // Green: 5 points
            else if (quality == 1) jumper.angleScore += 1;  // Yellow: 1 point
            // Red: 0 points (nothing)
            jumper.angleFrames++;
            
            jumper.velocityY += 0.2f;
            jumper.x += jumper.velocityX * deltaTime * 18.75f;
            jumper.y += jumper.velocityY * deltaTime * 18.75f;
            
            jumper.distance = jumper.x - 280;
            
// Angle affects distance - BRUTAL PENALTIES
            float angleBonus = 1.0f;
            if (quality == 2) angleBonus = 1.03f;  // Green: only 3% bonus (was 5%)
            else if (quality == 1) angleBonus = 0.75f;  // Yellow: 25% penalty! (was 15%)
            else angleBonus = 0.5f;  // Red: 50% penalty!! (was 40%)
            
            jumper.distance *= angleBonus;
            
// Flying view - redraw sky to clear clouds
            tft.fillScreen(COLOR_SKY);
            
            cloudScroll += jumper.velocityX * 3;
            sj_drawClouds(tft, cloudScroll);
            
            // Draw jumper with CURRENT angle
            int jx = SCREEN_W/2;
            int jy = SCREEN_H/2;
            int skiLen = 24;
            
            int skiEndX = jx + (int)(skiLen * cos(jumperAngle));
            int skiEndY = jy + (int)(skiLen * sin(jumperAngle));
            
            tft.drawLine(jx - 2, jy, skiEndX - 2, skiEndY, TFT_WHITE);
            tft.drawLine(jx - 2, jy + 1, skiEndX - 2, skiEndY + 1, TFT_WHITE);
            tft.drawLine(jx + 2, jy, skiEndX + 2, skiEndY, TFT_WHITE);
            tft.drawLine(jx + 2, jy + 1, skiEndX + 2, skiEndY + 1, TFT_WHITE);
            
            int bodyLen = 16;
            int bodyStartX = jx + (int)(3 * cos(jumperAngle));
            int bodyStartY = jy + (int)(3 * sin(jumperAngle)) - 4;
            int bodyEndX = bodyStartX + (int)(bodyLen * cos(jumperAngle));
            int bodyEndY = bodyStartY + (int)(bodyLen * sin(jumperAngle));
            
            for (int i = 0; i < 5; i++) {
                tft.drawLine(bodyStartX, bodyStartY + i, bodyEndX, bodyEndY + i, TFT_RED);
            }
            
            tft.fillCircle(bodyEndX + 2, bodyEndY - 5, 4, TFT_YELLOW);
            
            int armX = bodyStartX + (int)(8 * cos(jumperAngle));
            int armY = bodyStartY + (int)(8 * sin(jumperAngle));
            tft.drawLine(armX, armY, armX - 8, armY - 10, TFT_RED);
            tft.drawLine(armX, armY + 3, armX - 8, armY - 7, TFT_RED);
            
// Draw angle meter
sj_drawAngleMeter(tft, jumperAngle);
            
            // Distance - only update when changed significantly
            static float lastDisplayedDist = -1;
            if (abs(jumper.distance - lastDisplayedDist) > 1.0f) {
                char distBuf[16];
                snprintf(distBuf, sizeof(distBuf), "%.1fm", jumper.distance);
                tft.setTextColor(TFT_YELLOW, COLOR_SKY);
                tft.setTextFont(4);
                tft.setTextDatum(TL_DATUM);
                tft.fillRect(10, 100, 100, 30, COLOR_SKY);  // Clear background
                tft.drawString(distBuf, 10, 100);
                lastDisplayedDist = jumper.distance;
            }
            
            sj_drawMiniOlympicRings(tft, 15, SCREEN_H - 30);
            
if (jumper.y > SCREEN_H / 2 + 50) {
                jumper.phase = PHASE_LANDING;
                jumper.landMarkX = jumper.distance;
                
// Calculate final distance based on angle maintenance - BRUTAL
                float avgAngleQuality = jumper.angleScore / (float)jumper.angleFrames;
                jumper.distance = jumper.distance * (0.4f + avgAngleQuality * 0.06f);  // Was 0.5f + 0.08f
                
                playSound("/sounds/beep.wav", false);
            }
            
        } else if (jumper.phase == PHASE_LANDING) {
            // Landing view
            tft.fillScreen(COLOR_SKY);
            sj_drawMountainPeaks(tft);
            
            int horizonY = JUMP_HORIZON_Y;
            for (int y = horizonY; y < SCREEN_H; y++) {
                float t = (float)(y - horizonY) / (SCREEN_H - horizonY);
                uint16_t snowShade = tft.color565(255, 255, 255 - (int)(t * 40));
                tft.drawFastHLine(0, y, SCREEN_W, snowShade);
            }
            
            // Distance markers
            for (int d = 50; d <= 150; d += 25) {
                int x = 30 + (d - 50) * 2;
                tft.drawFastVLine(x, SCREEN_H- 60, 10, TFT_BLUE);
                tft.setTextColor(TFT_BLACK, COLOR_SNOW);
                tft.setTextFont(1);
                tft.setTextDatum(TC_DATUM);
                tft.drawString(String(d) + "m", x, SCREEN_H - 48);
            }
            
            // Trees on sides
            sj_drawPineTree(tft, 15, SCREEN_H - 50, 20);
            sj_drawPineTree(tft, SCREEN_W - 15, SCREEN_H - 50, 20);
            
            // Random trees in the distance
            for (int i = 0; i < 8; i++) {
                int treeX = 30 + random(0, SCREEN_W - 60);
                int treeY = JUMP_HORIZON_Y + 10 + random(0, 40);
                int treeSize = 8 + random(0, 8);
                sj_drawPineTree(tft, treeX, treeY, treeSize);
            }
            
            // Red landing mark
            int landX = 30 + (int)((jumper.landMarkX - 50) * 2);
            if (landX < 30) landX = 30;
            if (landX > 230) landX = 230;
            tft.fillRect(landX - 2, SCREEN_H - 60, 4, 10, TFT_RED);
            
            // Animate skier coming in from LEFT at an ANGLE
            for (int i = 0; i < 15; i++) {
                float t = i / 15.0f;
                int skyX = 20 + (int)((landX - 20) * t);
                int skyY = 30 + (int)((SCREEN_H - 60 - 30) * t);
                
                // Erase previous
                if (i > 0) {
                    int prevSkyX = 20 + (int)((landX - 20) * ((i-1) / 15.0f));
                    int prevSkyY = 30 + (int)((SCREEN_H - 60 - 30) * ((i-1) / 15.0f));
                    
                    if (prevSkyY < JUMP_HORIZON_Y) {
                        tft.fillRect(prevSkyX - 20, prevSkyY - 20, 40, 40, COLOR_SKY);
                    } else {
                        for (int ey = prevSkyY - 20; ey < prevSkyY + 20; ey++) {
                            if (ey >= JUMP_HORIZON_Y && ey < SCREEN_H) {
                                float st = (float)(ey - JUMP_HORIZON_Y) / (SCREEN_H - JUMP_HORIZON_Y);
                                uint16_t snowShade = tft.color565(255, 255, 255 - (int)(st * 40));
                                tft.drawFastHLine(prevSkyX - 20, ey, 40, snowShade);
                            }
                        }
                    }
                    
                    if (prevSkyY < JUMP_HORIZON_Y + 30) {
                        sj_drawMountainPeaks(tft);
                    }
                }
                
                sj_drawJumper(tft, skyX, skyY, false, true);
                delay(40);
            }
            
            // Final landing - erase flying position
            for (int ey = SCREEN_H - 85; ey < SCREEN_H - 45; ey++) {
                if (ey >= JUMP_HORIZON_Y) {
                    float st = (float)(ey - JUMP_HORIZON_Y) / (SCREEN_H - JUMP_HORIZON_Y);
                    uint16_t snowShade = tft.color565(255, 255, 255 - (int)(st * 40));
                    tft.drawFastHLine(landX - 20, ey, 40, snowShade);
                }
            }
            
            sj_drawJumper(tft, landX, SCREEN_H - 60, false, false);
            
            delay(500);
            
            // Ski away
            for (int i = 0; i < 40; i++) {
                // Erase completely
                for (int ey = SCREEN_H - 85; ey < SCREEN_H - 45; ey++) {
                    if (ey >= JUMP_HORIZON_Y) {
                        float st = (float)(ey - JUMP_HORIZON_Y) / (SCREEN_H - JUMP_HORIZON_Y);
                        uint16_t snowShade = tft.color565(255, 255, 255 - (int)(st * 40));
                        tft.drawFastHLine(landX - 15, ey, 35, snowShade);
                    }
                }
                
                landX += 3;
                if (landX > SCREEN_W + 20) break;
                
                sj_drawJumper(tft, landX, SCREEN_H - 60, false, false);
                delay(50);
            }
            
            gameRunning = false;
        }
        
        lastBtn = btn;
        updateAudio();
        delay(16);
    }
    
    // Results screen
    delay(500);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextFont(4);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("JUMP COMPLETE!", SCREEN_W/2, 50);
    
    char distBuf[32];
    snprintf(distBuf, sizeof(distBuf), "Distance: %.1fm", jumper.distance);
    tft.setTextFont(2);
    tft.drawString(distBuf, SCREEN_W/2, 90);
    
// Medal thresholds - VERY HARD
    int medal = 0;
    if (jumper.distance >= 155) medal = 3;       // Gold: 155m+ (was 145m)
    else if (jumper.distance >= 130) medal = 2;  // Silver: 130m+ (was 125m)
    else if (jumper.distance >= 110) medal = 1;  // Bronze: 110m+ (was 105m)
    
    if (medal > 0) {
        sj_drawMedal(tft, SCREEN_W/2, 150, medal);
    } else {
        tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
        tft.drawString("No Medal", SCREEN_W/2, 150);
    }
    
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextFont(2);
    tft.drawString("Press button to continue", SCREEN_W/2, SCREEN_H - 20);
    
    // Wait for button
    while (digitalRead(PIN_KO) == HIGH) {
        updateAudio();
        delay(50);
    }
    while (digitalRead(PIN_KO) == LOW) delay(10);
    delay(400);
}

#endif // SKI_JUMP_H