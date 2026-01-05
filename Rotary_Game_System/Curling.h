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
#define TEAM_GREEN 0x07E0
#define TEAM_RED 0xF800

// Game states
enum CurlingGameState {
  CURLING_READY,
  CURLING_AIMING,
  CURLING_POWER,
  CURLING_THROWING,
  CURLING_GAME_OVER
};

// Stone structure
struct CurlingStone {
  float x, y;
  float vx, vy;
  int team;
  bool active;
};

// Brusher struct
struct Brusher {
  float x, y;
  float prevX, prevY;
  bool movingRight;
  bool active;
};

// Global variables
CurlingGameState curlingState = CURLING_READY;
CurlingStone thrownStones[6];
int numThrownStones = 0;
float curlingStoneX = 160;
float curlingStoneY = 220;
float curlingStoneVX = 0;
float curlingStoneVY = 0;
float curlingAimAngle = 0;
int curlingPower = 50;
bool curlingStoneMoving = false;
unsigned long curlingStartTime = 0;
int currentTeam = 1;
int throwCount = 0;
int team1Score = 0;
int team2Score = 0;

Brusher brushers[2];

//=============================================================================
// BEER MUGS
//=============================================================================
void drawBeerMug(TFT_eSPI &tft, int x, int y, bool facingRight, int size) {
  uint16_t beerColor = tft.color565(255, 200, 0);
  uint16_t foamColor = tft.color565(255, 250, 230);
  uint16_t glassColor = tft.color565(100, 100, 120);
  
  if (facingRight) {
    int tiltOffset = 5;
    tft.fillRect(x - size/2 - tiltOffset, y, size, size*3/4, beerColor);
    tft.drawRect(x - size/2 - tiltOffset, y, size, size*3/4, glassColor);
    tft.drawRect(x - size/2 - tiltOffset + 1, y + 1, size - 2, size*3/4 - 2, glassColor);
    tft.fillRect(x - size/2 - tiltOffset + 3, y + size/6, size - 6, size/2, beerColor);
    
    for (int i = 0; i < 3; i++) {
      int foamY = y + size/6 - i * 3;
      tft.fillEllipse(x - tiltOffset, foamY, size/3, size/10, foamColor);
    }
    
    int handleX = x + size/2 + 5;
    tft.drawCircle(handleX, y + size/3, size/4, glassColor);
    tft.drawCircle(handleX, y + size/3, size/4 + 1, glassColor);
    tft.drawCircle(handleX, y + size/3, size/4 + 2, glassColor);
    tft.fillCircle(handleX + 2, y + size/3, size/5, COLOR_SKY);
    
    tft.fillCircle(x - 5, y + size/4, 2, foamColor);
    tft.fillCircle(x - 8, y + size/3, 2, foamColor);
    
  } else {
    int tiltOffset = 5;
    tft.fillRect(x - size/2 + tiltOffset, y, size, size*3/4, beerColor);
    tft.drawRect(x - size/2 + tiltOffset, y, size, size*3/4, glassColor);
    tft.drawRect(x - size/2 + tiltOffset + 1, y + 1, size - 2, size*3/4 - 2, glassColor);
    tft.fillRect(x - size/2 + tiltOffset + 3, y + size/6, size - 6, size/2, beerColor);
    
    for (int i = 0; i < 3; i++) {
      int foamY = y + size/6 - i * 3;
      tft.fillEllipse(x + tiltOffset, foamY, size/3, size/10, foamColor);
    }
    
    int handleX = x - size/2 - 5;
    tft.drawCircle(handleX, y + size/3, size/4, glassColor);
    tft.drawCircle(handleX, y + size/3, size/4 + 1, glassColor);
    tft.drawCircle(handleX, y + size/3, size/4 + 2, glassColor);
    tft.fillCircle(handleX - 2, y + size/3, size/5, COLOR_SKY);
    
    tft.fillCircle(x + 5, y + size/4, 2, foamColor);
    tft.fillCircle(x + 8, y + size/3, 2, foamColor);
  }
}

void drawBeerMugs(TFT_eSPI &tft, int x1, int y1, int x2, int y2, int size) {
  drawBeerMug(tft, x1, y1, false, size);
  drawBeerMug(tft, x2, y2, true, size);
  
  int midX = (x1 + x2) / 2;
  int midY = (y1 + y2) / 2 - 10;
  
  for (int i = 0; i < 5; i++) {
    tft.drawLine(midX - 8 + i, midY - 8, midX - 12 + i, midY - 15, OLYMPIC_YELLOW);
    tft.drawLine(midX + 8 - i, midY - 8, midX + 12 - i, midY - 15, OLYMPIC_YELLOW);
  }
  tft.fillCircle(midX, midY, 3, OLYMPIC_YELLOW);
  
  tft.setTextColor(OLYMPIC_YELLOW, COLOR_SKY);
  tft.setTextFont(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("CLINK!", midX, midY - 25);
}

//=============================================================================
// OLYMPIC-STYLE STADIUM SEATING - CONTINUOUS ROWS ALONG ICE
//=============================================================================
void drawFan(TFT_eSPI &tft, int x, int y, uint16_t shirtColor, int animFrame, int size) {
  uint16_t skinTone = tft.color565(220, 180, 140);
  
  // Head
  tft.fillCircle(x, y, size, skinTone);
  
  // Body
  tft.fillRect(x - size/2, y + size, size, size*2, shirtColor);
  
  // Arms (animated waving)
  if (animFrame % 3 == 0) {
    tft.drawLine(x - size/2, y + size, x - size*2, y, shirtColor);
    tft.drawLine(x + size/2, y + size, x + size*2, y, shirtColor);
  }
}

void drawOlympicBleachers(TFT_eSPI &tft, int animFrame) {
  int iceBottom = SCREEN_H - 50;
  int iceTop = 30;
  int iceLeftBottom = 60;
  int iceRightBottom = 260;
  int iceLeftTop = 130;
  int iceRightTop = 190;
  
  uint16_t seatColor = tft.color565(20, 80, 200);  // Blue seats
  uint16_t railColor = tft.color565(200, 200, 200);
  
  // LEFT BLEACHERS - 4 CONTINUOUS TIERS ALONG THE ICE
  for (int tier = 0; tier < 4; tier++) {
    int tierOffset = 12 + (tier * 10);  // Each tier gets further from ice and higher
    
    // Draw continuous row along the length of ice
    for (int segment = 0; segment < 10; segment++) {
      float t = segment / 10.0f;
      
      // Calculate position following ice perspective
      int segY = iceTop + (int)(t * (iceBottom - iceTop));
      int iceEdgeX = iceLeftTop + (int)(t * (iceLeftBottom - iceLeftTop));
      
      // Seat position - offset from ice edge, elevated by tier
      int seatX = iceEdgeX - tierOffset - 8;
      int seatY = segY - (tier * 8);  // Elevate as tier increases
      int seatWidth = 8;
      int seatHeight = (int)(20 * (1.0f + t * 0.6f));  // Grows with perspective
      
      // Draw seat
      tft.fillRect(seatX, seatY, seatWidth, seatHeight, seatColor);
      
      // Railing at front of tier
      if (tier == 0) {
        tft.drawFastHLine(seatX, seatY, seatWidth, railColor);
      }
      
      // Draw fans (every other segment, first 2 tiers only)
      if (tier < 2 && segment % 2 == 0) {
        int fanSize = 2 + (int)(t * 1.5f);
        drawFan(tft, seatX + seatWidth/2, seatY + 2, TEAM_GREEN, animFrame + segment + tier, fanSize);
      }
    }
    
    // Aisle/separator line between tiers
    if (tier < 3) {
      for (int segment = 0; segment < 10; segment++) {
        float t = segment / 10.0f;
        int segY = iceTop + (int)(t * (iceBottom - iceTop));
        int iceEdgeX = iceLeftTop + (int)(t * (iceLeftBottom - iceLeftTop));
        int separatorX = iceEdgeX - tierOffset - 8;
        int separatorY = segY - (tier * 8) - 2;
        tft.drawPixel(separatorX, separatorY, railColor);
      }
    }
  }
  
  // RIGHT BLEACHERS - 4 CONTINUOUS TIERS ALONG THE ICE
  for (int tier = 0; tier < 4; tier++) {
    int tierOffset = 12 + (tier * 10);
    
    // Draw continuous row along the length of ice
    for (int segment = 0; segment < 10; segment++) {
      float t = segment / 10.0f;
      
      int segY = iceTop + (int)(t * (iceBottom - iceTop));
      int iceEdgeX = iceRightTop + (int)(t * (iceRightBottom - iceRightTop));
      
      int seatX = iceEdgeX + tierOffset;
      int seatY = segY - (tier * 8);
      int seatWidth = 8;
      int seatHeight = (int)(20 * (1.0f + t * 0.6f));
      
      // Draw seat
      tft.fillRect(seatX, seatY, seatWidth, seatHeight, seatColor);
      
      // Railing
      if (tier == 0) {
        tft.drawFastHLine(seatX, seatY, seatWidth, railColor);
      }
      
      // Draw fans
      if (tier < 2 && segment % 2 == 0) {
        int fanSize = 2 + (int)(t * 1.5f);
        drawFan(tft, seatX + seatWidth/2, seatY + 2, TEAM_RED, animFrame + segment + tier, fanSize);
      }
    }
    
    // Aisle/separator
    if (tier < 3) {
      for (int segment = 0; segment < 10; segment++) {
        float t = segment / 10.0f;
        int segY = iceTop + (int)(t * (iceBottom - iceTop));
        int iceEdgeX = iceRightTop + (int)(t * (iceRightBottom - iceRightTop));
        int separatorX = iceEdgeX + tierOffset;
        int separatorY = segY - (tier * 8) - 2;
        tft.drawPixel(separatorX, separatorY, railColor);
      }
    }
  }
}

//=============================================================================
// SPECTATORS FOR SPLASH SCREEN
//=============================================================================
void drawSpectator(TFT_eSPI &tft, int x, int y, uint16_t shirtColor) {
  uint16_t skinTone = tft.color565(220, 180, 140);
  tft.fillCircle(x, y, 4, skinTone);
  tft.fillRect(x - 3, y + 4, 6, 8, shirtColor);
  tft.drawLine(x - 3, y + 6, x - 6, y + 2, shirtColor);
  tft.drawLine(x - 3, y + 7, x - 6, y + 3, shirtColor);
  tft.drawLine(x + 3, y + 6, x + 6, y + 2, shirtColor);
  tft.drawLine(x + 3, y + 7, x + 6, y + 3, shirtColor);
}

void drawSpectators(TFT_eSPI &tft) {
  uint16_t colors[] = {TFT_RED, TFT_BLUE, TFT_GREEN, TFT_YELLOW, TFT_CYAN, TFT_MAGENTA};
  for (int i = 0; i < 4; i++) {
    drawSpectator(tft, 20 + i * 15, 210 + (i % 2) * 5, colors[i % 6]);
  }
  for (int i = 0; i < 4; i++) {
    drawSpectator(tft, 260 + i * 15, 210 + (i % 2) * 5, colors[(i + 3) % 6]);
  }
}

//=============================================================================
// GET ICE COLOR AT POSITION
//=============================================================================
uint16_t getIceColorAt(TFT_eSPI &tft, int y) {
  int iceTop = 30;
  int iceBottom = SCREEN_H - 50;
  
  if (y < iceTop) return ARENA_BLUE;
  if (y >= iceBottom) return ARENA_BLUE;
  
  float t = (float)(y - iceTop) / (iceBottom - iceTop);
  uint16_t iceColor = tft.color565(240 - (int)(t * 30), 245 - (int)(t * 30), 255);
  return iceColor;
}

//=============================================================================
// BRUSHER/SWEEPER - WITH PROPER ICE COLOR ERASE (EXTENDED UP)
//=============================================================================
void eraseBrusher(TFT_eSPI &tft, float x, float y) {
  // Erase by drawing ice color at that Y position
  // Extended upward to cover top of head
  int eraseY = (int)y;
  int eraseX = (int)x;
  
  for (int dy = -8; dy < 30; dy++) {  // CHANGED: Start at -8 instead of -5 to cover helmet top
    uint16_t iceColor = getIceColorAt(tft, eraseY + dy);
    tft.drawFastHLine(eraseX - 12, eraseY + dy, 32, iceColor);
  }
}

void drawBrusher(TFT_eSPI &tft, float x, float y, int team, int animFrame) {
  uint16_t uniformColor = (team == 1) ? TEAM_GREEN : TEAM_RED;
  uint16_t skinTone = tft.color565(220, 180, 140);
  
  tft.fillRect((int)x - 4, (int)y + 12, 3, 10, uniformColor);
  tft.fillRect((int)x + 1, (int)y + 12, 3, 10, uniformColor);
  tft.fillRect((int)x - 4, (int)y + 21, 4, 2, TFT_BLACK);
  tft.fillRect((int)x + 1, (int)y + 21, 4, 2, TFT_BLACK);
  tft.fillRect((int)x - 5, (int)y + 2, 10, 12, uniformColor);
  
  int brushAngle = (animFrame % 2 == 0) ? 0 : 4;
  tft.fillRect((int)x - 8, (int)y + 5, 4, 8, uniformColor);
  tft.fillRect((int)x + 4, (int)y + 5, 4, 10, uniformColor);
  
  tft.drawLine((int)x + 6, (int)y + 14, (int)x + 10 + brushAngle, (int)y + 20, OLYMPIC_YELLOW);
  tft.drawLine((int)x + 6, (int)y + 15, (int)x + 10 + brushAngle, (int)y + 21, OLYMPIC_YELLOW);
  tft.fillRect((int)x + 9 + brushAngle, (int)y + 19, 8, 3, OLYMPIC_YELLOW);
  
  tft.fillCircle((int)x, (int)y, 4, skinTone);
  tft.fillCircle((int)x, (int)y - 2, 4, uniformColor);
}

//=============================================================================
// PLAYER
//=============================================================================
void drawCurlingPlayer(TFT_eSPI &tft, int x, int y, int team, bool hasStone) {
  uint16_t uniformColor = (team == 1) ? TEAM_GREEN : TEAM_RED;
  uint16_t skinTone = tft.color565(220, 180, 140);
  uint16_t stoneColor = (team == 1) ? TEAM_GREEN : TEAM_RED;
  
  tft.fillRect(x - 15, y + 15, 6, 20, uniformColor);
  tft.fillRect(x - 15, y + 33, 8, 4, TFT_BLACK);
  tft.fillRect(x - 5, y + 20, 5, 12, uniformColor);
  tft.fillRect(x - 5, y + 30, 6, 3, TFT_BLACK);
  tft.fillRect(x - 8, y, 12, 18, uniformColor);
  tft.fillRect(x - 3, y + 3, 15, 4, uniformColor);
  tft.fillCircle(x + 12, y + 5, 2, skinTone);
  
  if (hasStone) {
    tft.fillCircle(x + 18, y + 5, 6, stoneColor);
    tft.drawCircle(x + 18, y + 5, 6, TFT_BLACK);
    tft.fillRect(x + 16, y + 2, 4, 3, OLYMPIC_YELLOW);
  }
  
  tft.fillRect(x - 10, y + 8, 4, 8, uniformColor);
  tft.fillCircle(x, y - 3, 5, skinTone);
  tft.fillCircle(x, y - 5, 6, uniformColor);
}

//=============================================================================
// DRAW STONE
//=============================================================================
void drawStone(TFT_eSPI &tft, int x, int y, int team) {
  uint16_t stoneColor = (team == 1) ? TEAM_GREEN : TEAM_RED;
  tft.fillCircle(x, y, 8, stoneColor);
  tft.drawCircle(x, y, 8, TFT_BLACK);
  tft.drawCircle(x, y, 7, TFT_BLACK);
  tft.fillRect(x - 3, y - 2, 6, 3, OLYMPIC_YELLOW);
}

//=============================================================================
// STATIC RINK ELEMENTS
//=============================================================================
void drawStaticRink(TFT_eSPI &tft, int animFrame) {
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
  
  // Draw Olympic-style bleachers
  drawOlympicBleachers(tft, animFrame);
  
  // House
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
  
  // Hog line
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
// GOLD MEDAL DRAWING
//=============================================================================
void drawGoldMedal(TFT_eSPI &tft, int x, int y) {
  uint16_t goldColor = 0xFEA0;  // Gold color
  
  // Medal circle
  tft.fillCircle(x, y, 25, goldColor);
  tft.drawCircle(x, y, 25, TFT_BLACK);
  tft.drawCircle(x, y, 24, TFT_BLACK);
  tft.drawCircle(x, y, 26, TFT_BLACK);
  
  // Inner circle detail
  tft.drawCircle(x, y, 18, TFT_BLACK);
  
  // "1" in center
  tft.setTextColor(TFT_BLACK, goldColor);
  tft.setTextFont(4);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("1", x, y);
  
  // Ribbon/strap
  tft.fillRect(x - 3, y - 25, 6, 15, TFT_RED);
  tft.fillRect(x - 6, y - 35, 5, 15, TFT_RED);
  tft.fillRect(x + 1, y - 35, 5, 15, TFT_RED);
  
  // "GOLD" text below
  tft.setTextColor(goldColor, TFT_BLACK);
  tft.setTextFont(2);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("GOLD", x, y + 32);
}

//=============================================================================
// PHYSICS - BRUSHERS DON'T CROSS CENTER
//=============================================================================
void updateBrushers() {
  for (int i = 0; i < 2; i++) {
    if (!brushers[i].active) continue;
    
    brushers[i].prevX = brushers[i].x;
    brushers[i].prevY = brushers[i].y;
    
    if (brushers[i].movingRight) {
      brushers[i].x += 2;
      // Stop at center line instead of going past it
      if (brushers[i].x > SCREEN_W/2 - 2) {  // Stop 2 pixels before center
        brushers[i].x = SCREEN_W/2 - 2;
        brushers[i].movingRight = false;
      }
    } else {
      brushers[i].x -= 2;
      // Stop at reasonable distance from edge
      if (brushers[i].x < SCREEN_W/2 - 30) {
        brushers[i].movingRight = true;
      }
    }
  }
}

void updateStonePhysics() {
  if (!curlingStoneMoving) return;
  
  curlingStoneVX *= 0.98;
  curlingStoneVY *= 0.98;
  curlingStoneX += curlingStoneVX;
  curlingStoneY += curlingStoneVY;
  
  // Check collisions with thrown stones
  for (int i = 0; i < numThrownStones; i++) {
    if (!thrownStones[i].active) continue;
    
    float dx = thrownStones[i].x - curlingStoneX;
    float dy = thrownStones[i].y - curlingStoneY;
    float dist = sqrt(dx * dx + dy * dy);
    
    if (dist < 16) {
      float angle = atan2(dy, dx);
      float cos_a = cos(angle);
      float sin_a = sin(angle);
      
      // Transfer more momentum to knocked stone
      thrownStones[i].vx = cos_a * abs(curlingStoneVX) * 0.8;  // Increased from 0.7
      thrownStones[i].vy = sin_a * abs(curlingStoneVY) * 0.8;
      
      curlingStoneVX *= 0.3;
      curlingStoneVY *= 0.3;
      
      float overlap = 16 - dist;
      curlingStoneX -= cos_a * overlap * 0.5;
      curlingStoneY -= sin_a * overlap * 0.5;
      thrownStones[i].x += cos_a * overlap * 0.5;
      thrownStones[i].y += sin_a * overlap * 0.5;
    }
  }
  
  // Update thrown stones - KEEP MOVING EVEN OFF ICE
  for (int i = 0; i < numThrownStones; i++) {
    if (!thrownStones[i].active) continue;
    
    // Apply friction
    thrownStones[i].vx *= 0.96;  // Slightly less friction so they slide further
    thrownStones[i].vy *= 0.96;
    
    // Update position
    thrownStones[i].x += thrownStones[i].vx;
    thrownStones[i].y += thrownStones[i].vy;
    
    // Only deactivate when WAY off screen (not just out of play area)
    if (thrownStones[i].x < 0 || thrownStones[i].x > SCREEN_W ||
        thrownStones[i].y < 0 || thrownStones[i].y > SCREEN_H) {
      thrownStones[i].active = false;
    }
    
    // Stop if too slow
    if (abs(thrownStones[i].vx) < 0.05 && abs(thrownStones[i].vy) < 0.05) {
      thrownStones[i].vx = 0;
      thrownStones[i].vy = 0;
    }
  }
  
  if (curlingStoneX < 60 || curlingStoneX > SCREEN_W - 60) {
    curlingStoneMoving = false;
    return;
  }
  
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

int calculateFinalScore() {
  int score1 = 0;
  int score2 = 0;
  
  for (int i = 0; i < numThrownStones; i++) {
    if (!thrownStones[i].active) continue;
    
    float dx = thrownStones[i].x - SCREEN_W/2;
    float dy = thrownStones[i].y - 50;
    float dist = sqrt(dx*dx + dy*dy);
    
    if (dist < 20) {
      int points = 0;
      if (dist < 5) points = 10;
      else if (dist < 10) points = 7;
      else if (dist < 15) points = 5;
      else points = 3;
      
      if (thrownStones[i].team == 1) score1 += points;
      else score2 += points;
    }
  }
  
  team1Score = score1;
  team2Score = score2;
  return (score1 > score2) ? 1 : (score2 > score1) ? 2 : 0;
}

//=============================================================================
// MAIN GAME
//=============================================================================
void run_Curling(TFT_eSPI &tft) {
  tft.setRotation(3);
  pinMode(PIN_KO, INPUT_PULLUP);
  
  while (digitalRead(PIN_KO) == LOW) delay(10);
  delay(300);
  
  // SPLASH 1
  tft.fillScreen(COLOR_SKY);
  
  tft.setTextColor(TFT_BLUE, COLOR_SKY);
  tft.setTextFont(4);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("CURLING", SCREEN_W/2, 40);
  
  drawBeerMugs(tft, 100, 110, 220, 110, 40);
  
  tft.setTextColor(TFT_BLACK, COLOR_SKY);
  tft.setTextFont(2);
  tft.drawString("The Coolest Sport on Ice!", SCREEN_W/2, 180);
  
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
  
  // SPLASH 2
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
  tft.drawString("3 rounds x 2 teams!", SCREEN_W/2, 170);
  
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
  
while (digitalRead(PIN_KO) == LOW) delay(10);
  delay(400);
  
  // Init game - START IMMEDIATELY
  playSound("/sounds/beep_go.wav", false);
  
  // Init game
  curlingStartTime = millis();
  numThrownStones = 0;
  currentTeam = 1;
  throwCount = 0;
  team1Score = 0;
  team2Score = 0;
  
  for (int i = 0; i < 6; i++) {
    thrownStones[i].active = false;
    thrownStones[i].x = 0;
    thrownStones[i].y = 0;
    thrownStones[i].vx = 0;
    thrownStones[i].vy = 0;
    thrownStones[i].team = 0;
  }
  
  brushers[0].x = SCREEN_W/2 - 20;
  brushers[0].y = 100;
  brushers[0].prevX = brushers[0].x;
  brushers[0].prevY = brushers[0].y;
  brushers[0].movingRight = true;
  brushers[0].active = true;
  
  brushers[1].x = SCREEN_W/2 + 20;
  brushers[1].y = 110;
  brushers[1].prevX = brushers[1].x;
  brushers[1].prevY = brushers[1].y;
  brushers[1].movingRight = false;
  brushers[1].active = true;
  
  int lastRotary = rotaryPos;
  lastBtn = HIGH;
  int animFrame = 0;
  
  // MAIN GAME LOOP
  while (throwCount < 6) {
    currentTeam = (throwCount % 2 == 0) ? 1 : 2;
    
    curlingState = CURLING_AIMING;
    curlingAimAngle = 0;
    curlingPower = 50;
    curlingStoneMoving = false;
    
    brushers[0].active = true;
    brushers[1].active = true;
    
    animFrame++;
    drawStaticRink(tft, animFrame);
    
    // AIMING PHASE
    while (curlingState == CURLING_AIMING) {
      eraseBrusher(tft, brushers[0].prevX, brushers[0].prevY);
      eraseBrusher(tft, brushers[1].prevX, brushers[1].prevY);
      
      updateBrushers();
      
      int rotDiff = rotaryPos - lastRotary;
      if (abs(rotDiff) > 1) {
        curlingAimAngle += rotDiff * 0.03;
        curlingAimAngle = constrain(curlingAimAngle, -0.6, 0.6);
        lastRotary = rotaryPos;
        
        drawStaticRink(tft, animFrame);
      }
      
      for (int i = 0; i < numThrownStones; i++) {
        if (thrownStones[i].active) {
          drawStone(tft, (int)thrownStones[i].x, (int)thrownStones[i].y, thrownStones[i].team);
        }
      }
      
      drawBrusher(tft, brushers[0].x, brushers[0].y, currentTeam, animFrame);
      drawBrusher(tft, brushers[1].x, brushers[1].y, currentTeam, animFrame);
      drawCurlingPlayer(tft, SCREEN_W/2 - 30, SCREEN_H - 85, currentTeam, true);
      drawAimArrow(tft, SCREEN_W/2 - 12, SCREEN_H - 85, curlingAimAngle);
      
      tft.fillRect(0, 0, SCREEN_W, 20, ARENA_BLUE);
      tft.setTextColor((currentTeam == 1) ? TEAM_GREEN : TEAM_RED, ARENA_BLUE);
      tft.setTextFont(2);
      tft.setTextDatum(MC_DATUM);
      char buf[32];
      snprintf(buf, sizeof(buf), "Team %d - Throw %d/6", currentTeam, throwCount + 1);
      tft.drawString(buf, SCREEN_W/2, 8);
      
      tft.setTextColor(TFT_WHITE, ARENA_BLUE);
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
      delay(150);
    }
    
    // POWER PHASE
    while (curlingState == CURLING_POWER) {
      eraseBrusher(tft, brushers[0].prevX, brushers[0].prevY);
      eraseBrusher(tft, brushers[1].prevX, brushers[1].prevY);
      
      updateBrushers();
      
      int rotDiff = rotaryPos - lastRotary;
      if (abs(rotDiff) > 1) {
        curlingPower += rotDiff * 2;
        curlingPower = constrain(curlingPower, 10, 100);
        lastRotary = rotaryPos;
        
        drawStaticRink(tft, animFrame);
      }
      
      for (int i = 0; i < numThrownStones; i++) {
        if (thrownStones[i].active) {
          drawStone(tft, (int)thrownStones[i].x, (int)thrownStones[i].y, thrownStones[i].team);
        }
      }
      
      drawBrusher(tft, brushers[0].x, brushers[0].y, currentTeam, animFrame);
      drawBrusher(tft, brushers[1].x, brushers[1].y, currentTeam, animFrame);
      drawCurlingPlayer(tft, SCREEN_W/2 - 30, SCREEN_H - 85, currentTeam, true);
      drawAimArrow(tft, SCREEN_W/2 - 12, SCREEN_H - 85, curlingAimAngle);
      drawPowerBar(tft, curlingPower);
      
      tft.fillRect(0, 0, SCREEN_W, 20, ARENA_BLUE);
      tft.setTextColor((currentTeam == 1) ? TEAM_GREEN : TEAM_RED, ARENA_BLUE);
      tft.setTextFont(2);
      tft.setTextDatum(MC_DATUM);
      char buf[32];
      snprintf(buf, sizeof(buf), "Team %d - Throw %d/6", currentTeam, throwCount + 1);
      tft.drawString(buf, SCREEN_W/2, 8);
      
      tft.setTextColor(TFT_WHITE, ARENA_BLUE);
      tft.setTextDatum(BC_DATUM);
      tft.drawString("SET POWER with rotary", SCREEN_W/2, SCREEN_H - 5);
      
      int btn = digitalRead(PIN_KO);
      if (btn == LOW && lastBtn == HIGH) {
        curlingState = CURLING_THROWING;
        playSound("/sounds/beep_go.wav", false);
        
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
      delay(150);
    }
    
// THROWING ANIMATION - OPTIMIZED
    bool brusher0Done = false;
    bool brusher1Done = false;
    
    float prevStoneX = curlingStoneX;
    float prevStoneY = curlingStoneY;
    
    while (curlingStoneMoving) {
      animFrame++;
      
      // Erase stone at old position
      int oldX = (int)prevStoneX;
      int oldY = (int)prevStoneY;
      for (int dy = -10; dy <= 10; dy++) {
        uint16_t iceColor = getIceColorAt(tft, oldY + dy);
        tft.drawFastHLine(oldX - 10, oldY + dy, 20, iceColor);
      }
      
      // Erase ALL thrown stones at their old positions
      for (int i = 0; i < numThrownStones; i++) {
        if (thrownStones[i].active && (thrownStones[i].vx != 0 || thrownStones[i].vy != 0)) {
          int stoneOldX = (int)thrownStones[i].x;
          int stoneOldY = (int)thrownStones[i].y;
          for (int dy = -10; dy <= 10; dy++) {
            uint16_t iceColor = getIceColorAt(tft, stoneOldY + dy);
            tft.drawFastHLine(stoneOldX - 10, stoneOldY + dy, 20, iceColor);
          }
        }
      }
      
      if (brushers[0].active) eraseBrusher(tft, brushers[0].prevX, brushers[0].prevY);
      if (brushers[1].active) eraseBrusher(tft, brushers[1].prevX, brushers[1].prevY);
      
      updateStonePhysics();
      updateBrushers();
      
      if (curlingStoneY < brushers[0].y && !brusher0Done) {
        brushers[0].active = false;
        brusher0Done = true;
      }
      if (curlingStoneY < brushers[1].y && !brusher1Done) {
        brushers[1].active = false;
        brusher1Done = true;
      }
      
      // Draw ALL thrown stones (including those sliding off)
      for (int i = 0; i < numThrownStones; i++) {
        if (thrownStones[i].active) {
          drawStone(tft, (int)thrownStones[i].x, (int)thrownStones[i].y, thrownStones[i].team);
        }
      }
      
      if (brushers[0].active) drawBrusher(tft, brushers[0].x, brushers[0].y, currentTeam, animFrame);
      if (brushers[1].active) drawBrusher(tft, brushers[1].x, brushers[1].y, currentTeam, animFrame);
      
      drawStone(tft, (int)curlingStoneX, (int)curlingStoneY, currentTeam);
      
      prevStoneX = curlingStoneX;
      prevStoneY = curlingStoneY;
      
      updateAudio();
      delay(50);
    }
    
    if (numThrownStones < 6) {
      thrownStones[numThrownStones].x = curlingStoneX;
      thrownStones[numThrownStones].y = curlingStoneY;
      thrownStones[numThrownStones].vx = 0;
      thrownStones[numThrownStones].vy = 0;
      thrownStones[numThrownStones].team = currentTeam;
      thrownStones[numThrownStones].active = true;
      numThrownStones++;
    }
    
    delay(800);
    throwCount++;
  }
  
  
  // Calculate scores
  int winner = calculateFinalScore();
  unsigned long finalTime = millis() - curlingStartTime;
  
  playSound("/sounds/crowd-cheer-and-applause.wav", true);
  
// Game over
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextFont(4);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("GAME OVER!", SCREEN_W/2, 30);  // Moved up
  
  tft.setTextFont(2);
  tft.setTextColor(TEAM_GREEN, TFT_BLACK);
  char buf[32];
  snprintf(buf, sizeof(buf), "Team 1 (Green): %d", team1Score);
  tft.drawString(buf, SCREEN_W/2, 70);  // Moved up
  
  tft.setTextColor(TEAM_RED, TFT_BLACK);
  snprintf(buf, sizeof(buf), "Team 2 (Red): %d", team2Score);
  tft.drawString(buf, SCREEN_W/2, 90);  // Moved up
  
  snprintf(buf, sizeof(buf), "Time: %lu sec", finalTime / 1000);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(buf, SCREEN_W/2, 110);  // Moved up
  
  // Draw gold medal for winner
  if (winner == 1) {
    drawGoldMedal(tft, SCREEN_W/2, 155);
    tft.setTextFont(4);
    tft.setTextColor(TEAM_GREEN, TFT_BLACK);
    tft.drawString("GREEN WINS!", SCREEN_W/2, 200);
  } else if (winner == 2) {
    drawGoldMedal(tft, SCREEN_W/2, 155);
    tft.setTextFont(4);
    tft.setTextColor(TEAM_RED, TFT_BLACK);
    tft.drawString("RED WINS!", SCREEN_W/2, 200);
  } else {
    tft.setTextFont(4);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("TIE GAME!", SCREEN_W/2, 155);
  }
  
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextFont(2);
  tft.drawString("Press button to continue", SCREEN_W/2, 225);  // Moved down
  
  while (digitalRead(PIN_KO) == HIGH) {
    updateAudio();
    delay(50);
  }
  while (digitalRead(PIN_KO) == LOW) delay(10);
  delay(400);
}

#endif // CURLING_H