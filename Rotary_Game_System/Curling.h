#ifndef CURLING_H
#define CURLING_H

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
#define COLOR_ICE 0xDEFB
#define COLOR_SKY 0x5D9F
#define COLOR_HOUSE_BLUE 0x001F
#define COLOR_HOUSE_WHITE 0xFFFF
#define COLOR_HOUSE_RED 0xF800
#define COLOR_STONE 0x7BEF
#define ARENA_BLUE 0x1C9F
#define DARK_BLUE 0x0010
#define OLYMPIC_YELLOW 0xFFE0

// Game states
enum CurlingGameState {
  CURLING_READY,
  CURLING_AIMING,
  CURLING_POWER,
  CURLING_THROWING,
  CURLING_GAME_OVER
};

// Brusher struct
struct Brusher {
  float x, y;
  bool movingRight;
  bool active;
};

// Global variables
CurlingGameState curlingState = CURLING_READY;
float curlingStoneX = 160;
float curlingStoneY = 220;
float curlingStoneVX = 0;
float curlingStoneVY = 0;
float curlingAimAngle = 0;
int curlingPower = 50;
bool curlingStoneMoving = false;
unsigned long curlingStartTime = 0;

Brusher brushers[2];

//=============================================================================
// BEER MUGS - ANGLED AND CLINKING
//=============================================================================
void drawBeerMug(TFT_eSPI &tft, int x, int y, bool facingRight, int size) {
  uint16_t beerColor = tft.color565(200, 150, 50);
  uint16_t foamColor = tft.color565(255, 250, 230);
  uint16_t glassColor = tft.color565(100, 100, 120);
  
  if (facingRight) {
    // Right mug - tilted LEFT toward center
    // Mug body (tilted)
    int tiltOffset = 5;
    tft.fillRect(x - size/2 - tiltOffset, y, size, size*3/4, beerColor);
    tft.drawRect(x - size/2 - tiltOffset, y, size, size*3/4, glassColor);
    tft.drawRect(x - size/2 - tiltOffset + 1, y + 1, size - 2, size*3/4 - 2, glassColor);
    
    // Beer inside
    tft.fillRect(x - size/2 - tiltOffset + 3, y + size/6, size - 6, size/2, beerColor);
    
    // Foam on top
    for (int i = 0; i < 3; i++) {
      int foamY = y + size/6 - i * 3;
      tft.fillEllipse(x - tiltOffset, foamY, size/3, size/10, foamColor);
    }
    
    // Handle on FAR RIGHT side
    int handleX = x + size/2 + 5;
    tft.drawCircle(handleX, y + size/3, size/4, glassColor);
    tft.drawCircle(handleX, y + size/3, size/4 + 1, glassColor);
    tft.drawCircle(handleX, y + size/3, size/4 + 2, glassColor);
    // Erase inner part
    tft.fillCircle(handleX + 2, y + size/3, size/5, COLOR_SKY);
    
    // Bubbles
    tft.fillCircle(x - 5, y + size/4, 2, foamColor);
    tft.fillCircle(x - 8, y + size/3, 2, foamColor);
    
  } else {
    // Left mug - tilted RIGHT toward center
    // Mug body (tilted)
    int tiltOffset = 5;
    tft.fillRect(x - size/2 + tiltOffset, y, size, size*3/4, beerColor);
    tft.drawRect(x - size/2 + tiltOffset, y, size, size*3/4, glassColor);
    tft.drawRect(x - size/2 + tiltOffset + 1, y + 1, size - 2, size*3/4 - 2, glassColor);
    
    // Beer inside
    tft.fillRect(x - size/2 + tiltOffset + 3, y + size/6, size - 6, size/2, beerColor);
    
    // Foam on top
    for (int i = 0; i < 3; i++) {
      int foamY = y + size/6 - i * 3;
      tft.fillEllipse(x + tiltOffset, foamY, size/3, size/10, foamColor);
    }
    
    // Handle on FAR LEFT side
    int handleX = x - size/2 - 5;
    tft.drawCircle(handleX, y + size/3, size/4, glassColor);
    tft.drawCircle(handleX, y + size/3, size/4 + 1, glassColor);
    tft.drawCircle(handleX, y + size/3, size/4 + 2, glassColor);
    // Erase inner part
    tft.fillCircle(handleX - 2, y + size/3, size/5, COLOR_SKY);
    
    // Bubbles
    tft.fillCircle(x + 5, y + size/4, 2, foamColor);
    tft.fillCircle(x + 8, y + size/3, 2, foamColor);
  }
}

void drawBeerMugs(TFT_eSPI &tft, int x1, int y1, int x2, int y2, int size) {
  drawBeerMug(tft, x1, y1, false, size);  // Left mug facing right
  drawBeerMug(tft, x2, y2, true, size);   // Right mug facing left
  
  // Clink sparkle effect
  int midX = (x1 + x2) / 2;
  int midY = (y1 + y2) / 2 - 10;
  
  // Multiple sparkle lines
  for (int i = 0; i < 5; i++) {
    tft.drawLine(midX - 8 + i, midY - 8, midX - 12 + i, midY - 15, OLYMPIC_YELLOW);
    tft.drawLine(midX + 8 - i, midY - 8, midX + 12 - i, midY - 15, OLYMPIC_YELLOW);
  }
  tft.fillCircle(midX, midY, 3, OLYMPIC_YELLOW);
  
  // "CLINK!" text
  tft.setTextColor(OLYMPIC_YELLOW, COLOR_SKY);
  tft.setTextFont(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("CLINK!", midX, midY - 25);
}

//=============================================================================
// SPECTATORS FOR SPLASH SCREEN
//=============================================================================
void drawSpectator(TFT_eSPI &tft, int x, int y, uint16_t shirtColor) {
  uint16_t skinTone = tft.color565(220, 180, 140);
  
  // Head
  tft.fillCircle(x, y, 4, skinTone);
  
  // Body
  tft.fillRect(x - 3, y + 4, 6, 8, shirtColor);
  
  // Arms raised (cheering)
  tft.drawLine(x - 3, y + 6, x - 6, y + 2, shirtColor);
  tft.drawLine(x - 3, y + 7, x - 6, y + 3, shirtColor);
  tft.drawLine(x + 3, y + 6, x + 6, y + 2, shirtColor);
  tft.drawLine(x + 3, y + 7, x + 6, y + 3, shirtColor);
}

void drawSpectators(TFT_eSPI &tft) {
  // Draw crowd of spectators
  uint16_t colors[] = {TFT_RED, TFT_BLUE, TFT_GREEN, TFT_YELLOW, TFT_CYAN, TFT_MAGENTA};
  
  // Left side crowd
  for (int i = 0; i < 4; i++) {
    drawSpectator(tft, 20 + i * 15, 210 + (i % 2) * 5, colors[i % 6]);
  }
  
  // Right side crowd
  for (int i = 0; i < 4; i++) {
    drawSpectator(tft, 260 + i * 15, 210 + (i % 2) * 5, colors[(i + 3) % 6]);
  }
}

//=============================================================================
// BRUSHER/SWEEPER - IMPROVED
//=============================================================================
void drawBrusher(TFT_eSPI &tft, float x, float y, int team, int animFrame) {
  uint16_t uniformColor = (team == 1) ? DARK_BLUE : TFT_RED;
  uint16_t skinTone = tft.color565(220, 180, 140);
  
  // Legs
  tft.fillRect((int)x - 4, (int)y + 12, 3, 10, uniformColor);
  tft.fillRect((int)x + 1, (int)y + 12, 3, 10, uniformColor);
  tft.fillRect((int)x - 4, (int)y + 21, 4, 2, TFT_BLACK);
  tft.fillRect((int)x + 1, (int)y + 21, 4, 2, TFT_BLACK);
  
  // Body
  tft.fillRect((int)x - 5, (int)y + 2, 10, 12, uniformColor);
  
  // Arms with broom - animated
  int brushAngle = (animFrame % 2 == 0) ? 0 : 4;
  tft.fillRect((int)x - 8, (int)y + 5, 4, 8, uniformColor);
  tft.fillRect((int)x + 4, (int)y + 5, 4, 10, uniformColor);
  
  // Broom
  tft.drawLine((int)x + 6, (int)y + 14, (int)x + 10 + brushAngle, (int)y + 20, OLYMPIC_YELLOW);
  tft.drawLine((int)x + 6, (int)y + 15, (int)x + 10 + brushAngle, (int)y + 21, OLYMPIC_YELLOW);
  tft.fillRect((int)x + 9 + brushAngle, (int)y + 19, 8, 3, OLYMPIC_YELLOW);
  
  // Head
  tft.fillCircle((int)x, (int)y, 4, skinTone);
  tft.fillCircle((int)x, (int)y - 2, 4, uniformColor);
}

//=============================================================================
// PLAYER
//=============================================================================
void drawCurlingPlayer(TFT_eSPI &tft, int x, int y, int team, bool hasStone) {
  uint16_t uniformColor = (team == 1) ? DARK_BLUE : TFT_RED;
  uint16_t skinTone = tft.color565(220, 180, 140);
  uint16_t stoneColor = (team == 1) ? DARK_BLUE : TFT_RED;
  
  // Crouched legs
  tft.fillRect(x - 15, y + 15, 6, 20, uniformColor);
  tft.fillRect(x - 15, y + 33, 8, 4, TFT_BLACK);
  tft.fillRect(x - 5, y + 20, 5, 12, uniformColor);
  tft.fillRect(x - 5, y + 30, 6, 3, TFT_BLACK);
  
  // Body
  tft.fillRect(x - 8, y, 12, 18, uniformColor);
  
  // Extended arm
  tft.fillRect(x - 3, y + 3, 15, 4, uniformColor);
  tft.fillCircle(x + 12, y + 5, 2, skinTone);
  
  // Stone in hand
  if (hasStone) {
    tft.fillCircle(x + 18, y + 5, 6, stoneColor);
    tft.drawCircle(x + 18, y + 5, 6, TFT_BLACK);
    tft.fillRect(x + 16, y + 2, 4, 3, OLYMPIC_YELLOW);
  }
  
  // Other arm
  tft.fillRect(x - 10, y + 8, 4, 8, uniformColor);
  
  // Head
  tft.fillCircle(x, y - 3, 5, skinTone);
  tft.fillCircle(x, y - 5, 6, uniformColor);
}

//=============================================================================
// PERSPECTIVE RINK VIEW
//=============================================================================
void drawPerspectiveRink(TFT_eSPI &tft) {
  tft.fillScreen(ARENA_BLUE);
  
  // Ice with perspective
  int iceBottom = SCREEN_H - 50;
  int iceTop = 30;
  int iceLeftBottom = 60;
  int iceRightBottom = 260;
  int iceLeftTop = 130;
  int iceRightTop = 190;
  
  for (int y = iceTop; y < iceBottom; y++) {
    float t = (float)(y - iceTop) / (iceBottom - iceTop);
    int left = iceLeftTop + (int)(t * (iceLeftBottom - iceLeftTop));
    int right = iceRightTop + (int)(t * (iceRightBottom - iceRightTop));
    uint16_t iceColor = tft.color565(240 - (int)(t * 30), 245 - (int)(t * 30), 255);
    tft.drawFastHLine(left, y, right - left, iceColor);
  }
  
  // Walls
  for (int y = iceTop; y < iceBottom; y++) {
    float t = (float)(y - iceTop) / (iceBottom - iceTop);
    int left = iceLeftTop + (int)(t * (iceLeftBottom - iceLeftTop));
    int right = iceRightTop + (int)(t * (iceRightBottom - iceRightTop));
    tft.drawFastVLine(left - 1, y, 1, ARENA_BLUE);
    tft.drawFastVLine(left - 2, y, 1, tft.color565(20, 60, 120));
    tft.drawFastVLine(right + 1, y, 1, ARENA_BLUE);
    tft.drawFastVLine(right + 2, y, 1, tft.color565(20, 60, 120));
  }
  
  // House at far end
  int houseX = SCREEN_W / 2;
  int houseY = 50;
  tft.fillCircle(houseX, houseY, 20, COLOR_HOUSE_BLUE);
  tft.fillCircle(houseX, houseY, 15, COLOR_HOUSE_WHITE);
  tft.fillCircle(houseX, houseY, 10, COLOR_HOUSE_RED);
  tft.fillCircle(houseX, houseY, 5, COLOR_HOUSE_WHITE);
  tft.fillCircle(houseX, houseY, 2, TFT_BLACK);
  
  // Center line
  for (int y = iceTop + 10; y < iceBottom - 20; y += 6) {
    tft.drawPixel(houseX, y, tft.color565(100, 150, 255));
    tft.drawPixel(houseX + 1, y, tft.color565(100, 150, 255));
  }
  
  // Hog lines
  int hogLine1 = iceTop + 35;
  for (int x = iceLeftTop; x < iceRightTop; x += 3) {
    tft.drawPixel(x, hogLine1, TFT_RED);
  }
}

//=============================================================================
// DRAW AIMING ARROW
//=============================================================================
void drawAimArrow(TFT_eSPI &tft, int x, int y, float angle) {
  int length = 60;
  int endX = x + (int)(sin(angle) * length);
  int endY = y - (int)(cos(angle) * length);
  
  tft.drawLine(x, y, endX, endY, OLYMPIC_YELLOW);
  tft.drawLine(x + 1, y, endX + 1, endY, OLYMPIC_YELLOW);
  
  // Arrow head
  float arrowAngle = 0.4;
  int arrowLen = 8;
  int ax1 = endX + (int)(sin(angle - arrowAngle) * arrowLen);
  int ay1 = endY + (int)(-cos(angle - arrowAngle) * arrowLen);
  int ax2 = endX + (int)(sin(angle + arrowAngle) * arrowLen);
  int ay2 = endY + (int)(-cos(angle + arrowAngle) * arrowLen);
  
  tft.drawLine(endX, endY, ax1, ay1, OLYMPIC_YELLOW);
  tft.drawLine(endX, endY, ax2, ay2, OLYMPIC_YELLOW);
}

//=============================================================================
// POWER BAR
//=============================================================================
void drawPowerBar(TFT_eSPI &tft, int power) {
  int barX = SCREEN_W - 25;
  int barY = 60;
  int barW = 15;
  int barH = 100;
  
  tft.fillRect(barX, barY, barW, barH, TFT_BLACK);
  tft.drawRect(barX, barY, barW, barH, TFT_WHITE);
  
  int fillH = (power * barH) / 100;
  uint16_t color = (power < 40) ? TFT_GREEN : (power < 70) ? OLYMPIC_YELLOW : TFT_RED;
  tft.fillRect(barX + 2, barY + barH - fillH, barW - 4, fillH, color);
  
  tft.setTextFont(1);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, ARENA_BLUE);
  tft.drawString("PWR", barX + barW/2, barY - 8);
}

//=============================================================================
// DRAW STONE
//=============================================================================
void drawStone(TFT_eSPI &tft, int x, int y, int team) {
  uint16_t stoneColor = (team == 1) ? DARK_BLUE : TFT_RED;
  tft.fillCircle(x, y, 8, stoneColor);
  tft.drawCircle(x, y, 8, TFT_BLACK);
  tft.drawCircle(x, y, 7, TFT_BLACK);
  tft.fillRect(x - 3, y - 2, 6, 3, OLYMPIC_YELLOW);
}

//=============================================================================
// UPDATE PHYSICS
//=============================================================================
void updateBrushers() {
  for (int i = 0; i < 2; i++) {
    if (!brushers[i].active) continue;
    if (brushers[i].movingRight) {
      brushers[i].x += 2;
      if (brushers[i].x > SCREEN_W/2 + 30) brushers[i].movingRight = false;
    } else {
      brushers[i].x -= 2;
      if (brushers[i].x < SCREEN_W/2 - 30) brushers[i].movingRight = true;
    }
  }
}

void updateStonePhysics() {
  if (!curlingStoneMoving) return;
  
  curlingStoneVX *= 0.98;
  curlingStoneVY *= 0.98;
  curlingStoneX += curlingStoneVX;
  curlingStoneY += curlingStoneVY;
  
  if (curlingStoneX < 60) curlingStoneX = 60;
  if (curlingStoneX > SCREEN_W - 60) curlingStoneX = SCREEN_W - 60;
  
  if (abs(curlingStoneVX) < 0.1 && abs(curlingStoneVY) < 0.1) {
    curlingStoneMoving = false;
    curlingStoneVX = 0;
    curlingStoneVY = 0;
  }
  
  if (curlingStoneY < 50) {
    curlingStoneMoving = false;
    curlingStoneVY = 0;
  }
}

int calculateScore() {
  float dx = curlingStoneX - SCREEN_W/2;
  float dy = curlingStoneY - 50;
  float dist = sqrt(dx*dx + dy*dy);
  
  if (dist < 10) return 100;
  if (dist < 20) return 75;
  if (dist < 30) return 50;
  if (dist < 40) return 25;
  return 0;
}

//=============================================================================
// MAIN GAME
//=============================================================================
void run_Curling(TFT_eSPI &tft) {
  tft.setRotation(3);
  pinMode(PIN_KO, INPUT_PULLUP);
  
  while (digitalRead(PIN_KO) == LOW) delay(10);
  delay(300);
  
  // SPLASH 1 - Beer mugs
  tft.fillScreen(COLOR_SKY);
  tft.setTextColor(TFT_BLUE, COLOR_SKY);
  tft.setTextFont(4);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("CURLING", SCREEN_W/2, 40);
  
  drawBeerMugs(tft, 100, 100, 220, 100, 40);
  
  tft.setTextColor(TFT_BLACK, COLOR_SKY);
  tft.setTextFont(2);
  tft.drawString("The Coolest Sport on Ice!", SCREEN_W/2, 170);
  
  bool showPrompt = true;
  unsigned long lastBlink = millis();
  bool buttonPressed = false;
  int lastBtn = HIGH;
  
  while (!buttonPressed) {
    updateAudio();
    if (millis() - lastBlink > 500) {
      lastBlink = millis();
      showPrompt = !showPrompt;
      if (showPrompt) {
        tft.setTextColor(TFT_RED, COLOR_SKY);
        tft.drawString("Press button to continue", SCREEN_W/2, 210);
      } else {
        tft.fillRect(0, 200, SCREEN_W, 20, COLOR_SKY);
      }
    }
    int btn = digitalRead(PIN_KO);
    if (btn == LOW && lastBtn == HIGH) {
      buttonPressed = true;
      playSound("/sounds/beep.wav", false);
    }
    lastBtn = btn;
    delay(50);
  }
  
  while (digitalRead(PIN_KO) == LOW) delay(10);
  delay(400);
  
  // SPLASH 2 - Instructions with spectators
  tft.fillScreen(COLOR_SKY);
  tft.setTextColor(TFT_BLUE, COLOR_SKY);
  tft.setTextFont(4);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("HOW TO PLAY", SCREEN_W/2, 30);
  
  tft.setTextColor(TFT_BLACK, COLOR_SKY);
  tft.setTextFont(2);
  tft.drawString("1. Aim with rotary knob", SCREEN_W/2, 70);
  tft.drawString("2. Press button to lock aim", SCREEN_W/2, 95);
  tft.drawString("3. Set power with rotary", SCREEN_W/2, 120);
  tft.drawString("4. Press button to throw!", SCREEN_W/2, 145);
  tft.drawString("Get closest to center!", SCREEN_W/2, 170);
  
  // Draw spectators at bottom
  drawSpectators(tft);
  
  showPrompt = true;
  lastBlink = millis();
  buttonPressed = false;
  lastBtn = HIGH;
  
  while (!buttonPressed) {
    updateAudio();
    if (millis() - lastBlink > 500) {
      lastBlink = millis();
      showPrompt = !showPrompt;
      if (showPrompt) {
        tft.setTextColor(TFT_RED, COLOR_SKY);
        tft.drawString("Press to start", SCREEN_W/2, 195);
      } else {
        tft.fillRect(0, 185, SCREEN_W, 20, COLOR_SKY);
      }
    }
    int btn = digitalRead(PIN_KO);
    if (btn == LOW && lastBtn == HIGH) {
      buttonPressed = true;
      playSound("/sounds/beep.wav", false);
    }
    lastBtn = btn;
    delay(50);
  }
  
  while (digitalRead(PIN_KO) == LOW) delay(10);
  delay(400);
  
  // Countdown
  drawPerspectiveRink(tft);
  for (int countdown = 3; countdown >= 1; countdown--) {
    tft.setTextColor(TFT_YELLOW, ARENA_BLUE);
    tft.setTextFont(7);
    tft.setTextDatum(MC_DATUM);
    if (countdown == 3) tft.drawString("Ready", SCREEN_W/2, SCREEN_H/2);
    else if (countdown == 2) tft.drawString("Set", SCREEN_W/2, SCREEN_H/2);
    else tft.drawString("Go!", SCREEN_W/2, SCREEN_H/2);
    playSound("/sounds/beep.wav", false);
    unsigned long countStart = millis();
    while (millis() - countStart < 800) {
      updateAudio();
      delay(10);
    }
    tft.fillRect(0, SCREEN_H/2 - 40, SCREEN_W, 80, ARENA_BLUE);
  }
  playSound("/sounds/beep_go.wav", false);
  delay(300);
  
  // Init game
  curlingState = CURLING_AIMING;
  curlingStartTime = millis();
  curlingStoneX = SCREEN_W/2;
  curlingStoneY = SCREEN_H - 85;
  curlingAimAngle = 0;
  curlingPower = 50;
  curlingStoneMoving = false;
  
  brushers[0].x = SCREEN_W/2 - 20;
  brushers[0].y = 100;
  brushers[0].movingRight = true;
  brushers[0].active = true;
  
  brushers[1].x = SCREEN_W/2 + 20;
  brushers[1].y = 110;
  brushers[1].movingRight = false;
  brushers[1].active = true;
  
  int lastRotary = rotaryPos;
  lastBtn = HIGH;
  static int sweepFrame = 0;
  
  // AIMING PHASE
  while (curlingState == CURLING_AIMING) {
    sweepFrame++;
    drawPerspectiveRink(tft);
    drawBrusher(tft, brushers[0].x, brushers[0].y, 1, sweepFrame);
    drawBrusher(tft, brushers[1].x, brushers[1].y, 1, sweepFrame);
    updateBrushers();
    
    int rotDiff = rotaryPos - lastRotary;
    if (abs(rotDiff) > 1) {
      curlingAimAngle += rotDiff * 0.03;
      curlingAimAngle = constrain(curlingAimAngle, -0.6, 0.6);
      lastRotary = rotaryPos;
    }
    
    drawCurlingPlayer(tft, SCREEN_W/2 - 30, SCREEN_H - 85, 1, true);
    drawAimArrow(tft, SCREEN_W/2 - 12, SCREEN_H - 85, curlingAimAngle);
    
    tft.setTextColor(TFT_WHITE, ARENA_BLUE);
    tft.setTextFont(2);
    tft.setTextDatum(BC_DATUM);
    tft.drawString("AIM with rotary", SCREEN_W/2, SCREEN_H - 5);
    
    int btn = digitalRead(PIN_KO);
    if (btn == LOW && lastBtn == HIGH) {
      curlingState = CURLING_POWER;
      playSound("/sounds/beep.wav", false);
      delay(200);
    }
    lastBtn = btn;
    updateAudio();
    delay(30);
  }
  
  // POWER PHASE
  while (curlingState == CURLING_POWER) {
    sweepFrame++;
    drawPerspectiveRink(tft);
    drawBrusher(tft, brushers[0].x, brushers[0].y, 1, sweepFrame);
drawBrusher(tft, brushers[1].x, brushers[1].y, 1, sweepFrame);
    updateBrushers();
    
    int rotDiff = rotaryPos - lastRotary;
    if (abs(rotDiff) > 1) {
      curlingPower += rotDiff * 2;
      curlingPower = constrain(curlingPower, 10, 100);
      lastRotary = rotaryPos;
    }
    
    drawCurlingPlayer(tft, SCREEN_W/2 - 30, SCREEN_H - 85, 1, true);
    drawAimArrow(tft, SCREEN_W/2 - 12, SCREEN_H - 85, curlingAimAngle);
    drawPowerBar(tft, curlingPower);
    
    tft.setTextColor(TFT_WHITE, ARENA_BLUE);
    tft.setTextFont(2);
    tft.setTextDatum(BC_DATUM);
    tft.drawString("SET POWER with rotary", SCREEN_W/2, SCREEN_H - 5);
    
    int btn = digitalRead(PIN_KO);
    if (btn == LOW && lastBtn == HIGH) {
      curlingState = CURLING_THROWING;
      playSound("/sounds/beep_go.wav", false);
      
      // Launch stone
      float speed = curlingPower / 10.0;
      curlingStoneVX = sin(curlingAimAngle) * speed;
      curlingStoneVY = -cos(curlingAimAngle) * speed;
      curlingStoneMoving = true;
      curlingStoneX = SCREEN_W/2 - 12;
      curlingStoneY = SCREEN_H - 85;
      
      delay(200);
    }
    lastBtn = btn;
    updateAudio();
    delay(30);
  }
  
  // THROWING ANIMATION
  bool brusher0Done = false;
  bool brusher1Done = false;
  
  while (curlingStoneMoving) {
    sweepFrame++;
    updateStonePhysics();
    updateBrushers();
    
    drawPerspectiveRink(tft);
    
    // Check if stone passed brushers
    if (curlingStoneY < brushers[0].y && !brusher0Done) {
      brushers[0].active = false;
      brusher0Done = true;
    }
    if (curlingStoneY < brushers[1].y && !brusher1Done) {
      brushers[1].active = false;
      brusher1Done = true;
    }
    
    // Draw active brushers
    if (brushers[0].active) drawBrusher(tft, brushers[0].x, brushers[0].y, 1, sweepFrame);
    if (brushers[1].active) drawBrusher(tft, brushers[1].x, brushers[1].y, 1, sweepFrame);
    
    // Draw stone
    drawStone(tft, (int)curlingStoneX, (int)curlingStoneY, 1);
    
    updateAudio();
    delay(30);
  }
  
  // Stone at rest - stays visible
  delay(1000);
  
  // Calculate score
  int finalScore = calculateScore();
  unsigned long finalTime = millis() - curlingStartTime;
  
  playSound("/sounds/crowd-cheer-and-applause.wav", true);
  
  // Game over screen
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextFont(4);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("GAME OVER!", SCREEN_W/2, 60);
  
  tft.setTextFont(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  char buf[32];
  snprintf(buf, sizeof(buf), "Score: %d", finalScore);
  tft.drawString(buf, SCREEN_W/2, 100);
  
  snprintf(buf, sizeof(buf), "Time: %lu sec", finalTime / 1000);
  tft.drawString(buf, SCREEN_W/2, 120);
  
  if (finalScore >= 75) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextFont(4);
    tft.drawString("EXCELLENT!", SCREEN_W/2, 160);
  } else if (finalScore >= 50) {
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setTextFont(4);
    tft.drawString("GOOD SHOT!", SCREEN_W/2, 160);
  } else if (finalScore >= 25) {
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextFont(4);
    tft.drawString("NICE TRY!", SCREEN_W/2, 160);
  } else {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setTextFont(4);
    tft.drawString("MISSED!", SCREEN_W/2, 160);
  }
  
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextFont(2);
  tft.drawString("Press button to continue", SCREEN_W/2, 210);
  
  while (digitalRead(PIN_KO) == HIGH) {
    updateAudio();
    delay(50);
  }
  while (digitalRead(PIN_KO) == LOW) delay(10);
  delay(400);
}

#endif // CURLING_H