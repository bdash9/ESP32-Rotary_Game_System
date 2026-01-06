#ifndef BIATHLON_H
#define BIATHLON_H

#include <Arduino.h>
#include <TFT_eSPI.h>

extern void playSound(const char *path, bool stopCurrent);
extern void updateAudio();
extern void stopAudio();
extern volatile int rotaryPos;

#ifndef PIN_KO
#define PIN_KO 22
#endif

#define SCREEN_W 320
#define SCREEN_H 240

// Colors
#define COLOR_SNOW 0xFFFF
#define COLOR_SKY 0x5D9F
#define COLOR_TREE_GREEN 0x2444
#define COLOR_TREE_TRUNK 0x4208
#define OLYMPIC_BLUE 0x001F
#define OLYMPIC_YELLOW 0xFFE0
#define OLYMPIC_BLACK 0x0000
#define OLYMPIC_GREEN 0x07E0
#define OLYMPIC_RED 0xF800

// Game states
enum BiathlonState {
  BIATH_SKIING,
  BIATH_APPROACHING_RANGE,
  BIATH_SHOOTING,
  BIATH_PENALTY,
  BIATH_DONE
};

// Global variables - ADD biathlon_ PREFIX
BiathlonState biathlonState = BIATH_SKIING;
float biathlon_skiDistance = 0.0f;
float biathlon_skiSpeed = 0.0f;
int biathlon_heartRate = 60;
int biathlon_targetBPM = 150;
int biathlon_shootingRound = 0;
bool biathlon_targets[5] = {false, false, false, false, false};
int biathlon_currentTarget = 0;
int biathlon_missedShots = 0;
unsigned long biathlon_gameStartTime = 0;  // RENAMED
unsigned long biathlon_finalTime = 0;
int biathlon_lastRotary = 0;  // RENAMED

//=============================================================================
// OLYMPIC RINGS
//=============================================================================
void biath_drawOlympicRing(TFT_eSPI &tft, int cx, int cy, int radius, uint16_t color) {
  for (int i = 0; i < 3; i++) {
    tft.drawCircle(cx, cy, radius + i, color);
  }
}

void biath_drawOlympicRings(TFT_eSPI &tft, int centerX, int centerY, int ringRadius) {
  int spacing = ringRadius * 2 + 6;
  int vertOffset = ringRadius / 2;
  
  biath_drawOlympicRing(tft, centerX - spacing, centerY, ringRadius, OLYMPIC_BLUE);
  biath_drawOlympicRing(tft, centerX, centerY, ringRadius, OLYMPIC_BLACK);
  biath_drawOlympicRing(tft, centerX + spacing, centerY, ringRadius, OLYMPIC_RED);
  biath_drawOlympicRing(tft, centerX - spacing/2, centerY + vertOffset, ringRadius, OLYMPIC_YELLOW);
  biath_drawOlympicRing(tft, centerX + spacing/2, centerY + vertOffset, ringRadius, OLYMPIC_GREEN);
}

//=============================================================================
// BIATHLETE SILHOUETTE
//=============================================================================
void drawBiathlete(TFT_eSPI &tft, int x, int y, int scale) {
  uint16_t silhouetteColor = TFT_BLACK;
  
  // Skier in motion - with rifle on back
  // Head
  tft.fillCircle(x, y, 4 * scale, silhouetteColor);
  
  // Body leaning forward
  tft.fillRect(x - 3 * scale, y + 4 * scale, 6 * scale, 10 * scale, silhouetteColor);
  
  // Rifle on back (diagonal)
  tft.drawLine(x - 2 * scale, y + 2 * scale, x + 6 * scale, y - 3 * scale, silhouetteColor);
  tft.drawLine(x - 2 * scale, y + 3 * scale, x + 6 * scale, y - 2 * scale, silhouetteColor);
  tft.drawLine(x - 1 * scale, y + 2 * scale, x + 7 * scale, y - 3 * scale, silhouetteColor);
  
  // Legs in skiing position
  tft.fillRect(x - 4 * scale, y + 14 * scale, 3 * scale, 8 * scale, silhouetteColor);
  tft.fillRect(x + 1 * scale, y + 14 * scale, 3 * scale, 8 * scale, silhouetteColor);
  
  // Ski poles
  tft.drawLine(x - 6 * scale, y + 8 * scale, x - 10 * scale, y + 18 * scale, silhouetteColor);
  tft.drawLine(x + 6 * scale, y + 8 * scale, x + 10 * scale, y + 18 * scale, silhouetteColor);
}

//=============================================================================
// CROSSED SKIS WITH RIFLE
//=============================================================================
void drawCrossedSkisRifle(TFT_eSPI &tft, int x, int y) {
  // Left ski
  tft.fillRect(x - 40, y - 5, 80, 8, tft.color565(200, 200, 220));
  tft.drawRect(x - 40, y - 5, 80, 8, TFT_BLACK);
  
  // Right ski (crossed)
  tft.fillRect(x - 5, y - 40, 8, 80, tft.color565(200, 200, 220));
  tft.drawRect(x - 5, y - 40, 8, 80, TFT_BLACK);
  
  // Rifle across
  tft.fillRect(x - 35, y + 15, 70, 5, tft.color565(60, 40, 20));
  tft.drawRect(x - 35, y + 15, 70, 5, TFT_BLACK);
  // Rifle scope
  tft.fillCircle(x - 10, y + 17, 4, TFT_BLACK);
  // Stock
  tft.fillRect(x + 25, y + 13, 10, 9, tft.color565(100, 60, 30));
}

//=============================================================================
// SPLASH SCREEN 1
//=============================================================================
void biath_splashScreen1(TFT_eSPI &tft) {
  tft.fillScreen(COLOR_SKY);
  
  // Title
  tft.setTextColor(TFT_WHITE, COLOR_SKY);
  tft.setTextFont(4);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("BIATHLON", SCREEN_W/2, 30);
  
  // Olympic rings
  biath_drawOlympicRings(tft, SCREEN_W/2, 70, 12);
  
  // Biathlete graphic
  drawBiathlete(tft, SCREEN_W/2 - 60, 120, 2);
  
  // Crossed skis and rifle
  drawCrossedSkisRifle(tft, SCREEN_W/2 + 60, 140);
  
  // Subtitle
  tft.setTextColor(TFT_BLACK, COLOR_SKY);
  tft.setTextFont(2);
  tft.drawString("Ski Fast. Shoot Straight.", SCREEN_W/2, 200);
  
  // Blinking prompt
  bool showPrompt = true;
  unsigned long lastBlink = millis();
  int lastBtn = HIGH;
  
  while (digitalRead(PIN_KO) == LOW) delay(10);
  delay(300);
  
  while (true) {
    updateAudio();
    
    if (millis() - lastBlink > 500) {
      lastBlink = millis();
      showPrompt = !showPrompt;
      
      if (showPrompt) {
        tft.setTextColor(OLYMPIC_RED, COLOR_SKY);
        tft.drawString("Press button to continue", SCREEN_W/2, 225);
      } else {
        tft.fillRect(0, 215, SCREEN_W, 20, COLOR_SKY);
      }
    }
    
    int btn = digitalRead(PIN_KO);
    if (btn == LOW && lastBtn == HIGH) {
      playSound("/sounds/beep.wav", false);
      break;
    }
    lastBtn = btn;
    delay(50);
  }
  
  while (digitalRead(PIN_KO) == LOW) delay(10);
  delay(400);
}

//=============================================================================
// SPLASH SCREEN 2 - INSTRUCTIONS (REFORMATTED TO FIT)
//=============================================================================
void biath_splashScreen2(TFT_eSPI &tft) {
  tft.fillScreen(COLOR_SKY);
  
  tft.setTextColor(TFT_BLUE, COLOR_SKY);
  tft.setTextFont(4);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("HOW TO PLAY", SCREEN_W/2, 15);
  
  // Mini Olympic rings
  biath_drawOlympicRings(tft, SCREEN_W/2, 42, 6);
  
  tft.setTextColor(TFT_BLACK, COLOR_SKY);
  tft.setTextFont(2);
  tft.setTextDatum(TL_DATUM);
  
  int y = 60;
tft.setTextFont(2);
  tft.drawString("SKIING:", 10, y); y += 18;
  tft.setTextFont(1);
  tft.drawString("* ROTATE knob back & forth to ski", 15, y); y += 11;
  tft.drawString("* Faster rotation = faster skiing", 15, y); y += 11;
  tft.drawString("* Target: 150 BPM (green zone)", 15, y); y += 16;
  tft.drawString("* Fast skiing = tired = shaky aim!", 15, y); y += 16;
  
  tft.setTextFont(2);
  tft.drawString("SHOOTING:", 10, y); y += 18;
  tft.setTextFont(1);
  tft.drawString("* Aim with rotary (5 targets)", 15, y); y += 11;
  tft.drawString("* Press button to fire", 15, y); y += 11;
  tft.drawString("* High heart = more wobble!", 15, y); y += 11;
  tft.drawString("* Miss = 10 sec penalty", 15, y); y += 16;
  
  tft.setTextFont(2);
  tft.drawString("2 ski legs + 2 shooting rounds", 10, y); y += 18;
  tft.drawString("Fastest total time wins!", 10, y);
  
  // Blinking prompt
  bool showPrompt = true;
  unsigned long lastBlink = millis();
  int lastBtn = HIGH;
  
  while (true) {
    updateAudio();
    
    if (millis() - lastBlink > 500) {
      lastBlink = millis();
      showPrompt = !showPrompt;
      
      if (showPrompt) {
        tft.setTextColor(OLYMPIC_RED, COLOR_SKY);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("Press to start!", SCREEN_W/2, SCREEN_H - 10);
      } else {
        tft.fillRect(0, SCREEN_H - 20, SCREEN_W, 20, COLOR_SKY);
      }
    }
    
    int btn = digitalRead(PIN_KO);
    if (btn == LOW && lastBtn == HIGH) {
      playSound("/sounds/beep_go.wav", false);
      break;
    }
    lastBtn = btn;
    delay(50);
  }
  
  while (digitalRead(PIN_KO) == LOW) delay(10);
  delay(400);
}

//=============================================================================
// DRAW PINE TREE
//=============================================================================
void drawPineTree(TFT_eSPI &tft, int x, int y, int size) {
  // Trunk
  tft.fillRect(x - size/6, y + size/2, size/3, size/2, COLOR_TREE_TRUNK);
  
  // Three triangle layers
  for (int layer = 0; layer < 3; layer++) {
    int layerY = y + (layer * size/4);
    int layerWidth = size - (layer * size/5);
    tft.fillTriangle(x, layerY, 
                     x - layerWidth/2, layerY + size/3,
                     x + layerWidth/2, layerY + size/3, 
                     COLOR_TREE_GREEN);
  }
}

//=============================================================================
// SKIING SCENE - STATIC ELEMENTS
//=============================================================================
void drawSkiingScene(TFT_eSPI &tft) {
  // Sky
  tft.fillScreen(COLOR_SKY);
  
  // Snow ground with perspective
  for (int y = 120; y < SCREEN_H; y++) {
    float t = (float)(y - 120) / (SCREEN_H - 120);
    uint16_t snowShade = tft.color565(255, 255, 255 - (int)(t * 20));
    tft.drawFastHLine(0, y, SCREEN_W, snowShade);
  }
  
  // Trees on sides
  drawPineTree(tft, 30, 80, 40);
  drawPineTree(tft, 290, 90, 35);
  drawPineTree(tft, 50, 140, 30);
  drawPineTree(tft, 270, 150, 28);
  
  // Ski tracks in snow
  for (int i = 0; i < SCREEN_H - 120; i += 8) {
    tft.drawLine(140, 120 + i, 145, 125 + i, tft.color565(200, 200, 220));
    tft.drawLine(175, 120 + i, 180, 125 + i, tft.color565(200, 200, 220));
  }
}

//=============================================================================
// CADENCE METER - SHOWS TARGET ZONE
//=============================================================================
void drawCadenceMeter(TFT_eSPI &tft, int heartRate) {
  int meterX = 10;
  int meterY = 35;
  int meterW = 80;
  int meterH = 12;
  
  // Background
  tft.fillRect(meterX, meterY, meterW, meterH, TFT_BLACK);
  tft.drawRect(meterX, meterY, meterW, meterH, TFT_WHITE);
  
  // Target zone (140-160 BPM) in green
  int targetStart = ((140 - 60) * meterW) / 140;  // Map 60-200 BPM to bar
  int targetWidth = ((160 - 140) * meterW) / 140;
  tft.fillRect(meterX + targetStart, meterY + 1, targetWidth, meterH - 2, TFT_DARKGREEN);
  
  // Current heart rate indicator
  int hrPos = ((heartRate - 60) * meterW) / 140;
  hrPos = constrain(hrPos, 0, meterW - 2);
  
  uint16_t color = TFT_GREEN;
  if (heartRate < 140 || heartRate > 160) color = OLYMPIC_YELLOW;
  if (heartRate < 100 || heartRate > 180) color = OLYMPIC_RED;
  
  tft.fillRect(meterX + hrPos, meterY + 1, 3, meterH - 2, color);
  
  // Label
  tft.setTextFont(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("CADENCE", meterX, meterY - 8);
}

//=============================================================================
// SKIING PHASE HUD
//=============================================================================
void drawSkiingHUD(TFT_eSPI &tft, float distance, int heartRate, unsigned long elapsedTime) {
  // Top bar background
  tft.fillRect(0, 0, SCREEN_W, 30, TFT_BLACK);
  
  // Distance
  tft.setTextFont(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  char buf[32];
  snprintf(buf, sizeof(buf), "%.0fm", distance);
  tft.drawString(buf, 5, 8);
  
  // Heart rate with color
  uint16_t hrColor = TFT_GREEN;
  if (heartRate > 180) hrColor = OLYMPIC_RED;
  else if (heartRate > 160) hrColor = OLYMPIC_YELLOW;
  
  tft.setTextColor(hrColor, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  snprintf(buf, sizeof(buf), "%d BPM", heartRate);
  tft.drawString(buf, SCREEN_W/2, 8);
  
  // Draw heart icon
  tft.fillCircle(SCREEN_W/2 - 35, 12, 3, hrColor);
  tft.fillCircle(SCREEN_W/2 - 41, 12, 3, hrColor);
  tft.fillTriangle(SCREEN_W/2 - 44, 12, SCREEN_W/2 - 32, 12, SCREEN_W/2 - 38, 18, hrColor);
  
  // Time
  int seconds = elapsedTime / 1000;
  int deciseconds = (elapsedTime % 1000) / 100;
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(TR_DATUM);
  snprintf(buf, sizeof(buf), "%d.%d s", seconds, deciseconds);
  tft.drawString(buf, SCREEN_W - 5, 8);
}

//=============================================================================
// ANIMATED SKIER
//=============================================================================
void drawAnimatedSkier(TFT_eSPI &tft, int frame) {
  int x = SCREEN_W / 2;
  int y = 160;
  
  // Erase previous (draw snow color)
  tft.fillRect(x - 25, y - 15, 50, 40, COLOR_SNOW);
  
  // Animate legs
  int legOffset = (frame % 2 == 0) ? -3 : 3;
  
  // Body
  tft.fillRect(x - 5, y - 10, 10, 15, TFT_BLUE);
  
  // Head with helmet
  tft.fillCircle(x, y - 18, 6, TFT_YELLOW);
  
  // Rifle on back
  tft.drawLine(x - 3, y - 15, x + 8, y - 22, TFT_BLACK);
  tft.drawLine(x - 3, y - 14, x + 8, y - 21, TFT_BLACK);
  
  // Arms with poles
  tft.drawLine(x - 8, y - 5, x - 15, y + 10, TFT_BLUE);
  tft.drawLine(x + 8, y - 5, x + 15, y + 10, TFT_BLUE);
  
  // Pole lines
  tft.drawLine(x - 15, y + 10, x - 18, y + 20, TFT_BLACK);
  tft.drawLine(x + 15, y + 10, x + 18, y + 20, TFT_BLACK);
  
  // Legs (animated)
  tft.fillRect(x - 6 + legOffset, y + 5, 4, 12, TFT_BLACK);
  tft.fillRect(x + 2 - legOffset, y + 5, 4, 12, TFT_BLACK);
  
  // Skis
  tft.fillRect(x - 15 + legOffset, y + 17, 12, 3, tft.color565(200, 200, 220));
  tft.fillRect(x + 3 - legOffset, y + 17, 12, 3, tft.color565(200, 200, 220));
}

//=============================================================================
// DRAW BIATHLON RIFLE - BOTTOM OF SCREEN
//=============================================================================
void drawBiathlonRifle(TFT_eSPI &tft, int x, int y) {
  // x, y is the center point of the rifle
  
  // Buttplate (left side - with adjustment knobs)
  tft.fillRect(x - 150, y - 15, 15, 30, tft.color565(40, 40, 50));
  tft.fillCircle(x - 148, y - 10, 3, tft.color565(80, 80, 90));
  tft.fillCircle(x - 148, y + 10, 3, tft.color565(80, 80, 90));
  tft.drawRect(x - 150, y - 15, 15, 30, TFT_BLACK);
  
  // Stock - thumbhole design (grey)
  uint16_t stockColor = tft.color565(100, 100, 120);
  tft.fillRect(x - 135, y - 12, 50, 24, stockColor);
  tft.drawRect(x - 135, y - 12, 50, 24, TFT_BLACK);
  
  // Thumbhole cutout (white oval)
  tft.fillEllipse(x - 110, y, 8, 6, COLOR_SNOW);
  tft.drawEllipse(x - 110, y, 8, 6, TFT_BLACK);
  
  // Cheek rest (top of stock)
  tft.fillRect(x - 120, y - 18, 30, 6, tft.color565(80, 80, 100));
  tft.drawRect(x - 120, y - 18, 30, 6, TFT_BLACK);
  
  // Trigger area
  tft.fillRect(x - 85, y - 8, 15, 16, stockColor);
  tft.drawRect(x - 85, y - 8, 15, 16, TFT_BLACK);
  // Trigger
  tft.fillRect(x - 78, y + 3, 6, 8, tft.color565(200, 200, 210));
  
  // Grip with textured pattern (black)
  tft.fillRect(x - 75, y + 8, 12, 20, TFT_BLACK);
  // Texture dots
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 3; j++) {
      tft.drawPixel(x - 73 + i * 3, y + 10 + j * 5, tft.color565(60, 60, 60));
    }
  }
  
  // Magazine area (RED - distinctive feature)
  tft.fillRoundRect(x - 60, y - 8, 35, 12, 3, OLYMPIC_RED);
  tft.drawRoundRect(x - 60, y - 8, 35, 12, 3, TFT_BLACK);
  
  // Action/receiver (dark grey rectangle)
  tft.fillRect(x - 65, y - 12, 45, 10, tft.color565(60, 60, 70));
  tft.drawRect(x - 65, y - 12, 45, 10, TFT_BLACK);
  
  // Scope mounting rail
  tft.fillRect(x - 50, y - 16, 40, 3, tft.color565(40, 40, 50));
  
  // Scope (black cylinder on top)
  tft.fillRoundRect(x - 45, y - 24, 30, 8, 4, TFT_BLACK);
  tft.drawRoundRect(x - 45, y - 24, 30, 8, 4, tft.color565(100, 100, 110));
  // Scope lens (front)
  tft.fillCircle(x - 15, y - 20, 3, tft.color565(150, 150, 200));
  // Scope lens (rear)
  tft.fillCircle(x - 45, y - 20, 2, tft.color565(150, 150, 200));
  
  // Barrel (very long and thin - grey metal)
  uint16_t barrelColor = tft.color565(120, 120, 140);
  tft.fillRect(x - 20, y - 4, 140, 5, barrelColor);
  tft.drawLine(x - 20, y - 4, x + 120, y - 4, TFT_BLACK);
  tft.drawLine(x - 20, y + 1, x + 120, y + 1, TFT_BLACK);
  
  // Barrel band
  tft.fillRect(x + 40, y - 5, 3, 7, tft.color565(80, 80, 90));
  
  // Front sight (end of barrel)
  tft.fillRect(x + 118, y - 6, 4, 9, TFT_BLACK);
  tft.drawLine(x + 120, y - 2, x + 120, y + 2, TFT_YELLOW); // Sight post
  
  // Muzzle (end cap)
  tft.fillCircle(x + 125, y - 1, 4, tft.color565(50, 50, 60));
  tft.drawCircle(x + 125, y - 1, 4, TFT_BLACK);
  tft.fillCircle(x + 125, y - 1, 2, TFT_BLACK); // Bore
  
  // Sling swivel (under stock)
  tft.fillRect(x - 100, y + 12, 4, 6, tft.color565(80, 80, 90));
  tft.drawCircle(x - 98, y + 20, 3, tft.color565(60, 60, 70));
}

//=============================================================================
// SHOOTING RANGE SCENE - WITH RIFLE AT BOTTOM
//=============================================================================
void drawShootingRange(TFT_eSPI &tft) {
  // Sky
  tft.fillScreen(COLOR_SKY);
  
  // Snow
  tft.fillRect(0, 180, SCREEN_W, SCREEN_H - 180, COLOR_SNOW);
  
  // Target range backdrop - LARGER AREA
  tft.fillRect(0, 35, SCREEN_W, 140, tft.color565(150, 100, 50));  // Brown backdrop
  
  // "RANGE" sign
  tft.setTextColor(TFT_WHITE, tft.color565(150, 100, 50));
  tft.setTextFont(2);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("SHOOTING RANGE", SCREEN_W/2, 40);
  
  // Draw rifle at bottom
  drawBiathlonRifle(tft, SCREEN_W/2, 210);
}

//=============================================================================
// REDRAW BACKDROP AREA TO CLEAR CROSSHAIR TRAILS
//=============================================================================
void clearShootingBackdrop(TFT_eSPI &tft) {
  // Redraw just the brown backdrop where targets are
  tft.fillRect(0, 60, SCREEN_W, 110, tft.color565(150, 100, 50));
}

//=============================================================================
// DRAW TARGETS - LARGER AND MORE SPREAD OUT
//=============================================================================
void drawTargets(TFT_eSPI &tft, bool hit[5], int aimTarget) {
  int startX = 32;  // Start further left
  int y = 115;      // Centered vertically in backdrop
  int spacing = 56; // Much larger spacing
  
  for (int i = 0; i < 5; i++) {
    int x = startX + i * spacing;
    
    if (hit[i]) {
      // Hit target - white X
      tft.fillCircle(x, y, 20, TFT_WHITE);  // Larger target
      tft.drawCircle(x, y, 20, TFT_BLACK);
      // Big red X
      tft.drawLine(x - 12, y - 12, x + 12, y + 12, OLYMPIC_RED);
      tft.drawLine(x - 12, y + 12, x + 12, y - 12, OLYMPIC_RED);
      tft.drawLine(x - 11, y - 12, x + 13, y + 12, OLYMPIC_RED);
      tft.drawLine(x - 12, y + 13, x + 12, y - 11, OLYMPIC_RED);
      tft.drawLine(x - 13, y - 12, x + 11, y + 12, OLYMPIC_RED);
      tft.drawLine(x - 12, y + 11, x + 12, y - 13, OLYMPIC_RED);
    } else {
      // Active target - black circle (larger)
      tft.fillCircle(x, y, 20, TFT_BLACK);
      tft.fillCircle(x, y, 16, TFT_WHITE);
      tft.fillCircle(x, y, 10, TFT_BLACK);
      tft.fillCircle(x, y, 4, TFT_WHITE);
      
      // Highlight current target
      if (i == aimTarget) {
        tft.drawCircle(x, y, 23, OLYMPIC_YELLOW);
        tft.drawCircle(x, y, 24, OLYMPIC_YELLOW);
        tft.drawCircle(x, y, 25, OLYMPIC_YELLOW);
      }
    }
  }
}

//=============================================================================
// DRAW CROSSHAIR WITH WOBBLE - LARGER
//=============================================================================
void drawCrosshair(TFT_eSPI &tft, int targetX, int wobbleX, int wobbleY) {
  int startX = 32;
  int spacing = 56;
  int y = 115;
  int x = startX + targetX * spacing + wobbleX;
  int crossY = y + wobbleY;
  
  uint16_t color = OLYMPIC_RED;
  
  // Larger crosshair
  tft.drawLine(x - 30, crossY, x - 10, crossY, color);
  tft.drawLine(x + 10, crossY, x + 30, crossY, color);
  tft.drawLine(x, crossY - 30, x, crossY - 10, color);
  tft.drawLine(x, crossY + 10, x, crossY + 30, color);
  
  // Thicker lines
  tft.drawLine(x - 30, crossY + 1, x - 10, crossY + 1, color);
  tft.drawLine(x + 10, crossY + 1, x + 30, crossY + 1, color);
  tft.drawLine(x + 1, crossY - 30, x + 1, crossY - 10, color);
  tft.drawLine(x + 1, crossY + 10, x + 1, crossY + 30, color);
  
  // Center dot
  tft.fillCircle(x, crossY, 3, color);
}

//=============================================================================
// SHOOTING HUD - WITH GREEN LIGHT INDICATOR
//=============================================================================
void drawShootingHUD(TFT_eSPI &tft, int round, int shots, int heartRate, bool greenLight) {
  tft.fillRect(0, 0, SCREEN_W, 30, TFT_BLACK);
  
  tft.setTextFont(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  char buf[32];
  snprintf(buf, sizeof(buf), "Round %d/2", round + 1);
  tft.drawString(buf, 5, 8);
  
  // Heart rate
  uint16_t hrColor = (heartRate > 140) ? OLYMPIC_RED : OLYMPIC_YELLOW;
  if (heartRate < 100) hrColor = TFT_GREEN;
  
  tft.setTextColor(hrColor, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  snprintf(buf, sizeof(buf), "%d BPM", heartRate);
  tft.drawString(buf, SCREEN_W/2, 8);
  
  // GREEN LIGHT INDICATOR - FIXED COLORS!
  int lightX = SCREEN_W/2 + 50;
  int lightY = 15;
  if (greenLight) {
    tft.fillCircle(lightX, lightY, 8, TFT_GREEN);
    tft.drawCircle(lightX, lightY, 9, TFT_WHITE);
    tft.setTextFont(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(ML_DATUM);
    tft.drawString("FIRE!", lightX + 12, lightY);
  } else {
    // Dark grey using RGB565
    tft.fillCircle(lightX, lightY, 8, tft.color565(60, 60, 60));
    tft.drawCircle(lightX, lightY, 9, tft.color565(100, 100, 100));
  }
  
  // Shots remaining
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(TR_DATUM);
  tft.setTextFont(2);
  snprintf(buf, sizeof(buf), "Shots: %d/5", shots);
  tft.drawString(buf, SCREEN_W - 5, 8);
}

//=============================================================================
// GOLD MEDAL
//=============================================================================
void biathlon_drawGoldMedal(TFT_eSPI &tft, int x, int y) {  // RENAMED
  uint16_t goldColor = 0xFEA0;
  
  tft.fillCircle(x, y, 25, goldColor);
  tft.drawCircle(x, y, 25, TFT_BLACK);
  tft.drawCircle(x, y, 24, TFT_BLACK);
  tft.drawCircle(x, y, 26, TFT_BLACK);
  tft.drawCircle(x, y, 18, TFT_BLACK);
  
  tft.setTextColor(TFT_BLACK, goldColor);
  tft.setTextFont(4);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("1", x, y);
  
  tft.fillRect(x - 3, y - 25, 6, 15, OLYMPIC_RED);
  tft.fillRect(x - 6, y - 35, 5, 15, OLYMPIC_RED);
  tft.fillRect(x + 1, y - 35, 5, 15, OLYMPIC_RED);
  
  tft.setTextColor(goldColor, TFT_BLACK);
  tft.setTextFont(2);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("GOLD", x, y + 32);
}

//=============================================================================
// MAIN GAME
//=============================================================================
void run_Biathlon(TFT_eSPI &tft) {
  tft.setRotation(3);
  pinMode(PIN_KO, INPUT_PULLUP);
  
  // Splash screens
  biath_splashScreen1(tft);
  biath_splashScreen2(tft);
  
  // Initialize game
  biathlonState = BIATH_SKIING;
biathlon_skiDistance = 0.0f;
  biathlon_skiSpeed = 0.0f;
  biathlon_heartRate = 60;
  biathlon_shootingRound = 0;
  biathlon_missedShots = 0;
  for (int i = 0; i < 5; i++) biathlon_targets[i] = false;
  biathlon_currentTarget = 0;
  
  biathlon_gameStartTime = millis();
  biathlon_lastRotary = rotaryPos;
  
  // Draw skiing scene once
  drawSkiingScene(tft);
  
  int animFrame = 0;
  int lastBtn = HIGH;
  
// SKIING PHASE 1
  int skiAnimFrame = 0;
  int rotationActivity = 0;  // Track recent rotation
  
  while (biathlonState == BIATH_SKIING) {
    unsigned long elapsed = millis() - biathlon_gameStartTime;
    
    // Handle rotary for cadence - MORE SENSITIVE
    int currentRotary = rotaryPos;
    int rotDiff = abs(currentRotary - biathlon_lastRotary);
    
    if (rotDiff > 0) {
      // Any rotation counts as activity
      rotationActivity += rotDiff * 5;  // Multiply for bigger effect
      biathlon_skiSpeed += rotDiff * 0.5f;  // Increased from 0.3f
      biathlon_heartRate = min(200, biathlon_heartRate + rotDiff * 3);  // Increased multiplier
      biathlon_lastRotary = currentRotary;
    }
    
    // Decay rotation activity
    if (rotationActivity > 0) {
      rotationActivity--;
    }
    
    // Update physics
    biathlon_skiSpeed *= 0.95f;
    biathlon_skiDistance += biathlon_skiSpeed;
    
    // Heart rate drops when not rotating
    if (rotDiff == 0) {
      biathlon_heartRate = max(60, biathlon_heartRate - 2);  // Faster drop
    }
    
    // Visual feedback for rotation activity
    if (rotationActivity > 10) {
      skiAnimFrame++;
    }
    
    // Draw animated skier
    if (skiAnimFrame % 3 == 0) {
      drawAnimatedSkier(tft, skiAnimFrame);
    }
    
    // Draw HUD
    drawSkiingHUD(tft, biathlon_skiDistance, biathlon_heartRate, elapsed);
    
    // Draw cadence meter
    drawCadenceMeter(tft, biathlon_heartRate);
    
    // Instruction text at bottom - MORE HELPFUL
    tft.setTextFont(2);
    tft.setTextColor(TFT_YELLOW, COLOR_SNOW);
    tft.setTextDatum(BC_DATUM);
    if (biathlon_skiDistance < 50.0f) {
      tft.drawString("SPIN knob fast!", SCREEN_W/2, SCREEN_H - 5);
    } else if (rotationActivity < 5 && biathlon_skiDistance < 200.0f) {
      // Show hint if not rotating enough
      tft.setTextColor(OLYMPIC_RED, COLOR_SNOW);
      tft.drawString("Keep rotating!", SCREEN_W/2, SCREEN_H - 5);
    } else {
      tft.fillRect(0, SCREEN_H - 20, SCREEN_W, 20, COLOR_SNOW);
    }
    
    // Check if reached shooting range (500m)
    if (biathlon_skiDistance >= 500.0f && biathlon_shootingRound == 0) {
      biathlonState = BIATH_APPROACHING_RANGE;
      playSound("/sounds/beep.wav", false);
      
      tft.setTextFont(4);
      tft.setTextColor(OLYMPIC_RED, COLOR_SNOW);
      tft.setTextDatum(MC_DATUM);
      tft.drawString("SHOOTING RANGE!", SCREEN_W/2, SCREEN_H/2);
      delay(1500);
      
      biathlonState = BIATH_SHOOTING;
      
      drawShootingRange(tft);
      drawTargets(tft, biathlon_targets, biathlon_currentTarget);
      
      biathlon_currentTarget = 0;
      for (int i = 0; i < 5; i++) biathlon_targets[i] = false;
    }
    
    // Second ski leg
    if (biathlon_skiDistance >= 1000.0f && biathlon_shootingRound == 1) {
      biathlonState = BIATH_APPROACHING_RANGE;
      playSound("/sounds/beep.wav", false);
      
      tft.setTextFont(4);
      tft.setTextColor(OLYMPIC_RED, COLOR_SNOW);
      tft.setTextDatum(MC_DATUM);
      tft.drawString("SHOOTING RANGE!", SCREEN_W/2, SCREEN_H/2);
      delay(1500);
      
      biathlonState = BIATH_SHOOTING;
      
      drawShootingRange(tft);
      drawTargets(tft, biathlon_targets, biathlon_currentTarget);
      
      biathlon_currentTarget = 0;
      for (int i = 0; i < 5; i++) biathlon_targets[i] = false;
    }
    
    // Finish
    if (biathlon_skiDistance >= 1500.0f) {
      biathlonState = BIATH_DONE;
      break;
    }
    
    animFrame++;
    updateAudio();
    delay(50);
  }
  
// SHOOTING PHASES
  int wobbleUpdateCounter = 0;  // For slower wobble
  int currentWobbleX = 0;       // Persistent wobble values
  int currentWobbleY = 0;
  
  while (biathlonState == BIATH_SHOOTING) {
    // Heart rate decreases while aiming - BUT NOT TOO LOW!
    biathlon_heartRate = max(100, biathlon_heartRate - 1);
    
    // Update wobble less frequently for smoother, slower movement
    wobbleUpdateCounter++;
    if (wobbleUpdateCounter >= 4) {  // CHANGED: Update every 4 frames (200ms instead of 150ms)
      wobbleUpdateCounter = 0;
      
      // Calculate wobble - ADJUSTED FOR LARGER TARGETS
      int wobbleAmount = max(8, (biathlon_heartRate - 80) / 6);
      currentWobbleX = random(-wobbleAmount, wobbleAmount + 1);
      currentWobbleY = random(-wobbleAmount, wobbleAmount + 1);
    }
    
    // Check if shot would be good - LARGER TOLERANCE FOR LARGER TARGETS
    int hitTolerance = 6;  // 6 pixels tolerance for 20-pixel radius targets
    bool greenLight = (abs(currentWobbleX) <= hitTolerance && abs(currentWobbleY) <= hitTolerance);
        
    // CLEAR BACKDROP TO REMOVE CROSSHAIR TRAILS
    clearShootingBackdrop(tft);
    
    // REDRAW TARGETS
    drawTargets(tft, biathlon_targets, biathlon_currentTarget);
    
    // Draw crosshair
    drawCrosshair(tft, biathlon_currentTarget, currentWobbleX, currentWobbleY);
    
    // Handle rotary to aim
    int rotDiff = rotaryPos - biathlon_lastRotary;
    if (abs(rotDiff) > 1) {
      if (rotDiff > 0) {
        biathlon_currentTarget = min(4, biathlon_currentTarget + 1);
      } else {
        biathlon_currentTarget = max(0, biathlon_currentTarget - 1);
      }
      biathlon_lastRotary = rotaryPos;
    }
    
    // Draw HUD with green light indicator
    int shotsLeft = 0;
    for (int i = 0; i < 5; i++) if (!biathlon_targets[i]) shotsLeft++;
    drawShootingHUD(tft, biathlon_shootingRound, 5 - shotsLeft, biathlon_heartRate, greenLight);
    
    // Handle button to shoot
    int btn = digitalRead(PIN_KO);
    if (btn == LOW && lastBtn == HIGH) {
      // SHOOT!
      playSound("/sounds/beep_go.wav", false);
      
      // Use the same hit detection
      if (greenLight) {
        biathlon_targets[biathlon_currentTarget] = true;
        biathlon_heartRate += 5;
      } else {
        biathlon_missedShots++;
        biathlon_heartRate += 15;
      }
      
      clearShootingBackdrop(tft);
      drawTargets(tft, biathlon_targets, biathlon_currentTarget);
      
      // Check if all targets shot
      bool allDone = true;
      for (int i = 0; i < 5; i++) {
        if (!biathlon_targets[i]) {
          allDone = false;
          break;
        }
      }
      
      if (allDone) {
        biathlon_shootingRound++;
        if (biathlon_shootingRound < 2) {
          // Back to skiing for second leg
          biathlonState = BIATH_SKIING;
          playSound("/sounds/beep.wav", false);
          delay(1000);
          drawSkiingScene(tft);
        } else {
          // After 2nd shooting round, ski final leg
          biathlonState = BIATH_SKIING;
          playSound("/sounds/beep.wav", false);
          
          tft.setTextFont(4);
          tft.setTextColor(OLYMPIC_YELLOW, TFT_BLACK);
          tft.setTextDatum(MC_DATUM);
          tft.fillScreen(TFT_BLACK);
          tft.drawString("FINAL LEG!", SCREEN_W/2, SCREEN_H/2);
          delay(1500);
          
          drawSkiingScene(tft);
        }
      }
      
      delay(300);
    }
    lastBtn = btn;
    
    updateAudio();
    delay(50);
  }
  
// FINAL RESULTS
  biathlon_finalTime = millis() - biathlon_gameStartTime + (biathlon_missedShots * 10000);
  
  playSound("/sounds/crowd-cheer-and-applause.wav", true);
  
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextFont(4);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("FINISH!", SCREEN_W/2, 30);
  
  biath_drawOlympicRings(tft, SCREEN_W/2, 70, 10);
  
  tft.setTextFont(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  char buf[32];
  
  int finalSec = biathlon_finalTime / 1000;
  int finalDeci = (biathlon_finalTime % 1000) / 100;
  snprintf(buf, sizeof(buf), "Total Time: %d.%d sec", finalSec, finalDeci);
  tft.drawString(buf, SCREEN_W/2, 110);
  
  snprintf(buf, sizeof(buf), "Missed Shots: %d", biathlon_missedShots);
  tft.drawString(buf, SCREEN_W/2, 130);
  
  if (biathlon_missedShots > 0) {
    tft.setTextColor(OLYMPIC_RED, TFT_BLACK);
    tft.setTextFont(1);
    snprintf(buf, sizeof(buf), "(+%d seconds penalty)", biathlon_missedShots * 10);
    tft.drawString(buf, SCREEN_W/2, 145);
  }
  
  // Gold medal for good performance - EASIER CRITERIA!
  if (biathlon_finalTime < 120000 && biathlon_missedShots <= 1) {  // CHANGED: 120 sec & 3 misses
    biathlon_drawGoldMedal(tft, SCREEN_W/2, 180);
  } else {
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setTextFont(4);
    tft.drawString("GREAT JOB!", SCREEN_W/2, 180);
  }
  
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextFont(2);
  tft.drawString("Press button to continue", SCREEN_W/2, 225);
  
  while (digitalRead(PIN_KO) == HIGH) {
    updateAudio();
    delay(50);
  }
  while (digitalRead(PIN_KO) == LOW) delay(10);
  delay(400);
}

#endif // BIATHLON_H