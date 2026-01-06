#ifndef HOCKEY_H
#define HOCKEY_H

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

// Hockey Colors
#define HOCKEY_ICE 0xEF7D
#define HOCKEY_BLUE_LINE 0x001F
#define HOCKEY_RED_LINE 0xF800
#define HOCKEY_WHITE TFT_WHITE
#define HOCKEY_BLACK TFT_BLACK
#define HOCKEY_YELLOW 0x07FF     
#define HOCKEY_NET_GRAY 0x8410
#define HOCKEY_CROWD_1 0xF800
#define HOCKEY_CROWD_2 0x07E0
#define HOCKEY_CROWD_3 0x001F
#define HOCKEY_CROWD_4 0xFFE0
#define HOCKEY_PUCK_BLACK 0x0000

// Game states
enum HockeyState {
  HOCKEY_GOALIE_ROUND,
  HOCKEY_SHOOTER_ROUND,
  HOCKEY_GAME_OVER
};

// Global variables with hockey_ prefix
HockeyState hockey_gameState = HOCKEY_GOALIE_ROUND;
int hockey_goalieScore = 0;
int hockey_shooterScore = 0;
int hockey_currentShot = 0;
int hockey_puckX = 0;
int hockey_puckY = 0;
int hockey_goalieX = 160;
int hockey_shootDirection = 0;
bool hockey_shotInProgress = false;
unsigned long hockey_shotStartTime = 0;
int hockey_lastRotaryPos = 0;
int hockey_crowdAnimFrame = 0;
unsigned long hockey_lastCrowdUpdate = 0;

//=============================================================================
// CROWD ANIMATION
//=============================================================================
void hockey_drawCrowd(TFT_eSPI &tft, int frame) {
  uint16_t colors[4] = {HOCKEY_CROWD_1, HOCKEY_CROWD_2, HOCKEY_CROWD_3, HOCKEY_CROWD_4};
  
  for (int row = 0; row < 2; row++) {
    for (int col = 0; col < 16; col++) {
      int x = 10 + col * 20;
      int y = 5 + row * 15;
      int bobble = (frame + col) % 2;
      
      uint16_t color = colors[(row + col) % 4];
      tft.fillCircle(x, y + bobble, 6, color);
    }
  }
}

//=============================================================================
// DRAW HOCKEY STICK
//=============================================================================
void hockey_drawStick(TFT_eSPI &tft, int x, int y, int rotation) {
  uint16_t woodColor = tft.color565(139, 90, 43);
  
  if (rotation == 0) {
    tft.fillRect(x - 3, y - 60, 6, 80, woodColor);
    tft.drawRect(x - 3, y - 60, 6, 80, HOCKEY_BLACK);
    tft.fillRoundRect(x - 15, y + 20, 30, 8, 2, HOCKEY_BLACK);
    tft.drawRoundRect(x - 15, y + 20, 30, 8, 2, TFT_DARKGREY);
  } else if (rotation == 1) {
    tft.fillRect(x - 50, y - 3, 80, 6, woodColor);
    tft.drawRect(x - 50, y - 3, 80, 6, HOCKEY_BLACK);
    tft.fillRoundRect(x + 30, y - 10, 8, 30, 2, HOCKEY_BLACK);
    tft.drawRoundRect(x + 30, y - 10, 8, 30, 2, TFT_DARKGREY);
  } else {
    tft.fillRect(x - 30, y - 3, 80, 6, woodColor);
    tft.drawRect(x - 30, y - 3, 80, 6, HOCKEY_BLACK);
    tft.fillRoundRect(x - 38, y - 10, 8, 30, 2, HOCKEY_BLACK);
    tft.drawRoundRect(x - 38, y - 10, 8, 30, 2, TFT_DARKGREY);
  }
}

//=============================================================================
// DRAW BEAT-UP HOCKEY PLAYER
//=============================================================================
void hockey_drawBeatUpPlayer(TFT_eSPI &tft, int x, int y, int scale) {
  uint16_t jerseyColor = tft.color565(0, 50, 150);
  uint16_t skinColor = tft.color565(255, 220, 177);
  
  tft.fillCircle(x, y, 10 * scale, TFT_DARKGREY);
  tft.fillRect(x - 12 * scale, y, 24 * scale, 6 * scale, TFT_DARKGREY);
  
  tft.drawLine(x + 5 * scale, y - 8 * scale, x + 8 * scale, y - 2 * scale, TFT_BLACK);
  tft.drawLine(x + 6 * scale, y - 8 * scale, x + 9 * scale, y - 2 * scale, TFT_BLACK);
  
  tft.drawLine(x - 8 * scale, y + 5 * scale, x - 6 * scale, y + 10 * scale, HOCKEY_BLACK);
  tft.drawLine(x + 8 * scale, y + 5 * scale, x + 6 * scale, y + 10 * scale, HOCKEY_BLACK);
  
  tft.fillRect(x - 10 * scale, y - 2 * scale, 20 * scale, 14 * scale, skinColor);
  
  tft.fillCircle(x - 4 * scale, y + 2 * scale, 3 * scale, tft.color565(80, 0, 80));
  tft.fillCircle(x - 4 * scale, y + 2 * scale, 2 * scale, tft.color565(40, 0, 40));
  
  tft.fillCircle(x + 4 * scale, y + 2 * scale, 2 * scale, TFT_WHITE);
  tft.fillCircle(x + 4 * scale, y + 2 * scale, 1 * scale, HOCKEY_BLACK);
  
  tft.fillTriangle(x, y + 4 * scale, x - 2 * scale, y + 7 * scale, x + 2 * scale, y + 7 * scale, skinColor);
  tft.drawTriangle(x, y + 4 * scale, x - 2 * scale, y + 7 * scale, x + 2 * scale, y + 7 * scale, HOCKEY_BLACK);
  
  tft.fillRect(x - 5 * scale, y + 8 * scale, 10 * scale, 4 * scale, tft.color565(100, 0, 0));
  
  tft.fillRect(x - 4 * scale, y + 8 * scale, 2 * scale, 2 * scale, TFT_WHITE);
  tft.fillRect(x + 2 * scale, y + 8 * scale, 2 * scale, 2 * scale, TFT_WHITE);
  
  tft.drawLine(x + 6 * scale, y + 5 * scale, x + 8 * scale, y + 7 * scale, HOCKEY_RED_LINE);
  
  tft.fillRect(x - 12 * scale, y + 12 * scale, 24 * scale, 20 * scale, jerseyColor);
  tft.drawRect(x - 12 * scale, y + 12 * scale, 24 * scale, 20 * scale, HOCKEY_WHITE);
  
  tft.setTextColor(HOCKEY_WHITE, jerseyColor);
  tft.setTextFont(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("99", x, y + 22 * scale);
  
  tft.fillRect(x - 18 * scale, y + 14 * scale, 6 * scale, 18 * scale, jerseyColor);
  tft.fillRect(x + 12 * scale, y + 14 * scale, 6 * scale, 18 * scale, jerseyColor);
  
  tft.fillCircle(x - 15 * scale, y + 34 * scale, 4 * scale, HOCKEY_BLACK);
  tft.fillCircle(x + 15 * scale, y + 34 * scale, 4 * scale, HOCKEY_BLACK);
  
  tft.fillRect(x - 10 * scale, y + 32 * scale, 8 * scale, 15 * scale, HOCKEY_BLACK);
  tft.fillRect(x + 2 * scale, y + 32 * scale, 8 * scale, 15 * scale, HOCKEY_BLACK);
  
  tft.fillRect(x - 12 * scale, y + 47 * scale, 10 * scale, 4 * scale, HOCKEY_BLACK);
  tft.fillRect(x + 2 * scale, y + 47 * scale, 10 * scale, 4 * scale, HOCKEY_BLACK);
  tft.drawLine(x - 12 * scale, y + 51 * scale, x - 2 * scale, y + 51 * scale, TFT_LIGHTGREY);
  tft.drawLine(x + 2 * scale, y + 51 * scale, x + 12 * scale, y + 51 * scale, TFT_LIGHTGREY);
  
  tft.fillRect(x + 16 * scale, y + 18 * scale, 3 * scale, 35 * scale, tft.color565(139, 90, 43));
  tft.fillRoundRect(x + 10 * scale, y + 51 * scale, 15 * scale, 5 * scale, 2, HOCKEY_BLACK);
}

//=============================================================================
// SPLASH SCREEN 1
//=============================================================================
void hockey_splashScreen1(TFT_eSPI &tft) {
  tft.fillScreen(HOCKEY_ICE);
  
  tft.setTextColor(HOCKEY_RED_LINE, HOCKEY_ICE);
  tft.setTextFont(4);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("ICE HOCKEY", SCREEN_W/2, 25);
  tft.drawString("SHOOTOUT", SCREEN_W/2, 50);
  
  tft.fillRect(0, 70, SCREEN_W, 4, HOCKEY_RED_LINE);
  
  hockey_drawBeatUpPlayer(tft, SCREEN_W/2, 120, 2);
  
  tft.setTextColor(HOCKEY_BLACK, HOCKEY_ICE);
  tft.setTextFont(2);
  tft.drawString("Block Shots. Score Goals.", SCREEN_W/2, 215);
    
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
        tft.setTextColor(HOCKEY_RED_LINE, HOCKEY_ICE);
        tft.drawString("Press button to continue", SCREEN_W/2, 235);
      } else {
        tft.fillRect(0, 225, SCREEN_W, 15, HOCKEY_ICE);
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
// SPLASH SCREEN 2
//=============================================================================
void hockey_splashScreen2(TFT_eSPI &tft) {
  tft.fillScreen(HOCKEY_ICE);
  
  hockey_drawStick(tft, 30, 40, 2);
  hockey_drawStick(tft, 290, 40, 1);
  
  tft.setTextColor(HOCKEY_BLUE_LINE, HOCKEY_ICE);
  tft.setTextFont(4);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("HOW TO PLAY", SCREEN_W/2, 15);
  
  tft.fillRect(0, 35, SCREEN_W, 3, HOCKEY_RED_LINE);
  
  tft.setTextColor(HOCKEY_BLACK, HOCKEY_ICE);
  tft.setTextFont(2);
  tft.setTextDatum(TL_DATUM);
  
  int y = 45;
  tft.drawString("BEST OF 5 SHOTS!", 10, y); y += 20;
  
  tft.setTextFont(2);
  tft.drawString("AS GOALIE:", 10, y); y += 18;
  tft.setTextFont(1);
  tft.drawString("* ROTATE knob to move left/right", 15, y); y += 11;
  tft.drawString("* Press BUTTON to dive/poke check", 15, y); y += 11;
  tft.drawString("* Block the puck!", 15, y); y += 16;
  
  tft.setTextFont(2);
  tft.drawString("AS SHOOTER:", 10, y); y += 18;
  tft.setTextFont(1);
  tft.drawString("* ROTATE to aim left/center/right", 15, y); y += 11;
  tft.drawString("* Press BUTTON to shoot", 15, y); y += 11;
  tft.drawString("* Beat the goalie!", 15, y); y += 16;
  
  tft.setTextFont(2);
  tft.setTextColor(HOCKEY_RED_LINE, HOCKEY_ICE);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Score more than you allow", SCREEN_W/2, y); y += 18;
  tft.drawString("to WIN!", SCREEN_W/2, y);
  
  bool showPrompt = true;
  unsigned long lastBlink = millis();
  int lastBtn = HIGH;
  
  while (true) {
    updateAudio();
    
    if (millis() - lastBlink > 500) {
      lastBlink = millis();
      showPrompt = !showPrompt;
      
      if (showPrompt) {
        tft.setTextColor(HOCKEY_RED_LINE, HOCKEY_ICE);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("Press to start!", SCREEN_W/2, SCREEN_H - 10);
      } else {
        tft.fillRect(0, SCREEN_H - 20, SCREEN_W, 20, HOCKEY_ICE);
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
// DRAW ICE RINK
//=============================================================================
void hockey_drawRink(TFT_eSPI &tft) {
  tft.fillScreen(HOCKEY_ICE);
  
  tft.fillRect(0, SCREEN_H/2 - 2, SCREEN_W, 4, HOCKEY_RED_LINE);
  
  tft.fillRect(0, 60, SCREEN_W, 3, HOCKEY_BLUE_LINE);
  tft.fillRect(0, 180, SCREEN_W, 3, HOCKEY_BLUE_LINE);
  
  tft.drawCircle(SCREEN_W/2, SCREEN_H/2, 30, HOCKEY_BLUE_LINE);
  tft.drawCircle(SCREEN_W/2, SCREEN_H/2, 31, HOCKEY_BLUE_LINE);
  
  tft.fillCircle(SCREEN_W/2, SCREEN_H/2, 8, HOCKEY_BLUE_LINE);
}

//=============================================================================
// DRAW GOAL NET
//=============================================================================
void hockey_drawGoal(TFT_eSPI &tft, int y, bool isPlayerGoal) {
  int goalWidth = 120;
  int goalHeight = 40;
  int centerX = SCREEN_W / 2;
  
  tft.drawRect(centerX - goalWidth/2, y, goalWidth, goalHeight, HOCKEY_WHITE);
  tft.drawRect(centerX - goalWidth/2 + 1, y + 1, goalWidth - 2, goalHeight - 2, HOCKEY_WHITE);
  
  tft.drawFastVLine(centerX - goalWidth/2, y, goalHeight, HOCKEY_WHITE);
  tft.drawFastVLine(centerX - goalWidth/2 + 1, y, goalHeight, HOCKEY_WHITE);
  tft.drawFastVLine(centerX + goalWidth/2 - 1, y, goalHeight, HOCKEY_WHITE);
  tft.drawFastVLine(centerX + goalWidth/2, y, goalHeight, HOCKEY_WHITE);
  
  for (int i = 0; i < goalWidth; i += 10) {
    tft.drawLine(centerX - goalWidth/2 + i, y, centerX - goalWidth/2 + i, y + goalHeight, HOCKEY_NET_GRAY);
  }
  for (int i = 0; i < goalHeight; i += 10) {
    tft.drawLine(centerX - goalWidth/2, y + i, centerX + goalWidth/2, y + i, HOCKEY_NET_GRAY);
  }
  
  tft.fillCircle(centerX - goalWidth/2, y, 4, HOCKEY_RED_LINE);
  tft.fillCircle(centerX + goalWidth/2, y, 4, HOCKEY_RED_LINE);
}

//=============================================================================
// DRAW GOALIE
//=============================================================================
void hockey_drawGoalie(TFT_eSPI &tft, int x, int y, bool isPlayer, bool isDiving) {
  uint16_t color = isPlayer ? tft.color565(0, 100, 200) : HOCKEY_RED_LINE;
  
  if (isDiving) {
    tft.fillRect(x - 20, y - 5, 40, 15, color);
    tft.fillCircle(x - 25, y, 8, color);
    tft.fillRect(x + 20, y - 8, 15, 16, HOCKEY_BLACK);
  } else {
    tft.fillRect(x - 14, y + 8, 12, 25, HOCKEY_WHITE);
    tft.fillRect(x + 2, y + 8, 12, 25, HOCKEY_WHITE);
    tft.drawRect(x - 14, y + 8, 12, 25, HOCKEY_BLACK);
    tft.drawRect(x + 2, y + 8, 12, 25, HOCKEY_BLACK);
    
    tft.fillRect(x - 12, y + 15, 8, 2, color);
    tft.fillRect(x + 4, y + 15, 8, 2, color);
    tft.fillRect(x - 12, y + 25, 8, 2, color);
    tft.fillRect(x + 4, y + 25, 8, 2, color);
    
    tft.fillRect(x - 15, y - 8, 30, 20, color);
    tft.drawRect(x - 15, y - 8, 30, 20, HOCKEY_WHITE);
    
    tft.fillCircle(x, y - 16, 8, color);
    tft.fillRect(x - 8, y - 18, 16, 6, color);
    tft.drawLine(x - 6, y - 16, x - 6, y - 10, HOCKEY_BLACK);
    tft.drawLine(x, y - 16, x, y - 10, HOCKEY_BLACK);
    tft.drawLine(x + 6, y - 16, x + 6, y - 10, HOCKEY_BLACK);
    
    tft.fillRect(x + 15, y - 2, 12, 8, HOCKEY_BLACK);
    tft.drawRect(x + 15, y - 2, 12, 8, HOCKEY_RED_LINE);
    
    tft.fillCircle(x - 18, y + 3, 6, tft.color565(139, 90, 43));
    tft.drawCircle(x - 18, y + 3, 6, HOCKEY_BLACK);
    
    tft.fillRect(x + 10, y + 8, 3, 28, tft.color565(139, 90, 43));
    tft.fillRoundRect(x + 5, y + 34, 15, 5, 2, HOCKEY_BLACK);
  }
}

//=============================================================================
// DRAW SHOOTER
//=============================================================================
void hockey_drawShooter(TFT_eSPI &tft, int x, int y) {
  uint16_t jerseyColor = tft.color565(200, 0, 0);
  
  tft.fillRect(x - 8, y + 15, 6, 18, HOCKEY_BLACK);
  tft.fillRect(x + 2, y + 15, 6, 18, HOCKEY_BLACK);
  
  tft.fillRect(x - 10, y + 33, 8, 3, HOCKEY_BLACK);
  tft.fillRect(x + 2, y + 33, 8, 3, HOCKEY_BLACK);
  
  tft.fillRect(x - 10, y, 20, 18, jerseyColor);
  tft.drawRect(x - 10, y, 20, 18, HOCKEY_WHITE);
  
  tft.fillCircle(x, y - 6, 6, jerseyColor);
  
  tft.fillRect(x - 14, y + 5, 4, 15, jerseyColor);
  tft.fillRect(x + 10, y + 5, 4, 15, jerseyColor);
  
  tft.fillRect(x + 12, y + 8, 2, 30, tft.color565(139, 90, 43));
  tft.fillRoundRect(x + 8, y + 36, 12, 4, 2, HOCKEY_BLACK);
}

//=============================================================================
// DRAW PUCK
//=============================================================================
void hockey_drawPuck(TFT_eSPI &tft, int x, int y) {
  tft.fillCircle(x, y, 4, HOCKEY_PUCK_BLACK);
  tft.drawCircle(x, y, 4, TFT_DARKGREY);
  tft.drawCircle(x, y, 5, TFT_DARKGREY);
}

//=============================================================================
// DRAW HUD
//=============================================================================
void hockey_drawHUD(TFT_eSPI &tft, int goalieScore, int shooterScore, int shot, bool isGoalieRound) {
  tft.fillRect(0, 0, SCREEN_W, 25, HOCKEY_BLACK);
  
  tft.setTextFont(2);
  tft.setTextColor(HOCKEY_WHITE, HOCKEY_BLACK);
  tft.setTextDatum(TL_DATUM);
  
  char buf[32];
  snprintf(buf, sizeof(buf), "Blocked: %d", goalieScore);
  tft.drawString(buf, 5, 5);
  
  tft.setTextDatum(TC_DATUM);
  if (isGoalieRound) {
    tft.setTextColor(HOCKEY_BLUE_LINE, HOCKEY_BLACK);
    snprintf(buf, sizeof(buf), "GOALIE %d/5", shot + 1);
  } else {
    tft.setTextColor(HOCKEY_RED_LINE, HOCKEY_BLACK);
    snprintf(buf, sizeof(buf), "SHOOTER %d/5", shot + 1);
  }
  tft.drawString(buf, SCREEN_W/2, 5);
  
  tft.setTextColor(HOCKEY_WHITE, HOCKEY_BLACK);
  tft.setTextDatum(TR_DATUM);
  snprintf(buf, sizeof(buf), "Scored: %d", shooterScore);
  tft.drawString(buf, SCREEN_W - 5, 5);
}

//=============================================================================
// GOALIE ROUND
//=============================================================================
void hockey_goalieRound(TFT_eSPI &tft) {
  hockey_currentShot = 0;
  hockey_lastRotaryPos = rotaryPos;
  
  tft.fillScreen(HOCKEY_BLUE_LINE);
  tft.setTextColor(HOCKEY_WHITE, HOCKEY_BLUE_LINE);
  tft.setTextFont(4);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("YOU'RE THE", SCREEN_W/2, 80);
  tft.drawString("GOALIE!", SCREEN_W/2, 110);
  tft.setTextFont(2);
  tft.drawString("Block 5 shots!", SCREEN_W/2, 150);
  
  playSound("/sounds/beep.wav", false);
  delay(2000);
  
  while (hockey_currentShot < 5) {
    hockey_drawRink(tft);
    hockey_drawCrowd(tft, hockey_crowdAnimFrame);
    hockey_drawGoal(tft, 35, true);
    hockey_drawHUD(tft, hockey_goalieScore, hockey_shooterScore, hockey_currentShot, true);
    
    hockey_drawShooter(tft, SCREEN_W/2, 180);
    
    hockey_goalieX = 160;
    
    int prevGoalieX = hockey_goalieX;
    bool diving = false;
    int lastBtn = HIGH;
    bool shotTaken = false;
    
    unsigned long shotDelay = millis() + random(1500, 3000);
    
    hockey_drawGoalie(tft, hockey_goalieX, 60, true, false);
    
    while (!shotTaken) {
      updateAudio();
      
      if (millis() - hockey_lastCrowdUpdate > 200) {
        hockey_lastCrowdUpdate = millis();
        hockey_crowdAnimFrame++;
        hockey_drawCrowd(tft, hockey_crowdAnimFrame);
      }
      
      int rotaryDiff = rotaryPos - hockey_lastRotaryPos;
      
      if (rotaryDiff != 0) {
        hockey_goalieX += rotaryDiff * 10;
        hockey_goalieX = constrain(hockey_goalieX, 85, 235);
        hockey_lastRotaryPos = rotaryPos;
      }
      
      if (hockey_goalieX != prevGoalieX) {
        tft.fillRect(prevGoalieX - 40, 30, 80, 70, HOCKEY_ICE);
        hockey_drawGoal(tft, 35, true);
        
        tft.drawCircle(SCREEN_W/2, SCREEN_H/2, 30, HOCKEY_BLUE_LINE);
        tft.drawCircle(SCREEN_W/2, SCREEN_H/2, 31, HOCKEY_BLUE_LINE);
        
        prevGoalieX = hockey_goalieX;
      }
      
      hockey_drawGoalie(tft, hockey_goalieX, 60, true, false);
      
      int btn = digitalRead(PIN_KO);
      if (btn == LOW && lastBtn == HIGH) {
        diving = true;
      }
      lastBtn = btn;
      
      if (millis() > shotDelay && !shotTaken) {
        shotTaken = true;
        
        hockey_shootDirection = random(0, 3) - 1;
        
        int targetX;
        if (hockey_shootDirection < 0) targetX = 110;
        else if (hockey_shootDirection > 0) targetX = 210;
        else targetX = 160;
        
        playSound("/sounds/beep_go.wav", false);
        
        int prevPuckX = -1;
        int prevPuckY = -1;
        int step = 0;
        
        while (step <= 20) {
          int rotaryDiff = rotaryPos - hockey_lastRotaryPos;
          
          if (rotaryDiff != 0) {
            int oldGoalieX = hockey_goalieX;
            hockey_goalieX += rotaryDiff * 10;
            hockey_goalieX = constrain(hockey_goalieX, 85, 235);
            hockey_lastRotaryPos = rotaryPos;
            
            if (oldGoalieX != hockey_goalieX) {
             tft.fillRect(oldGoalieX - 40, 30, 80, 70, HOCKEY_ICE);
              hockey_drawGoal(tft, 35, true);
              tft.drawCircle(SCREEN_W/2, SCREEN_H/2, 30, HOCKEY_BLUE_LINE);
              tft.drawCircle(SCREEN_W/2, SCREEN_H/2, 31, HOCKEY_BLUE_LINE);
            }
          }
          
          int btn = digitalRead(PIN_KO);
          if (btn == LOW && !diving && step < 10) {
            diving = true;
          }
          
          if (diving && step >= 5) {
            hockey_drawGoalie(tft, hockey_goalieX, 65, true, true);
          } else {
            hockey_drawGoalie(tft, hockey_goalieX, 60, true, false);
          }
          
          int puckX = SCREEN_W/2 + ((targetX - SCREEN_W/2) * step) / 20;
          int puckY = 200 - (step * 8);
          
          if (prevPuckX >= 0 && prevPuckY > 100) {
            tft.fillCircle(prevPuckX, prevPuckY, 7, HOCKEY_ICE);
            
            if (prevPuckY > 100 && prevPuckY < 140) {
              tft.drawCircle(SCREEN_W/2, SCREEN_H/2, 30, HOCKEY_BLUE_LINE);
              tft.drawCircle(SCREEN_W/2, SCREEN_H/2, 31, HOCKEY_BLUE_LINE);
            }
          }
          
          hockey_drawPuck(tft, puckX, puckY);
          
          prevPuckX = puckX;
          prevPuckY = puckY;
          
          step++;
          delay(30);
        }
        
        bool blocked = false;
        int finalPuckX;
        if (hockey_shootDirection < 0) finalPuckX = 110;
        else if (hockey_shootDirection > 0) finalPuckX = 210;
        else finalPuckX = 160;
        
        if (diving) {
          if (abs(finalPuckX - hockey_goalieX) < 50) blocked = true;
        } else {
          if (abs(finalPuckX - hockey_goalieX) < 35) blocked = true;
        }
        
        tft.fillRect(0, 30, SCREEN_W, 150, HOCKEY_ICE);
        hockey_drawGoal(tft, 35, true);
        tft.drawCircle(SCREEN_W/2, SCREEN_H/2, 30, HOCKEY_BLUE_LINE);
        tft.drawCircle(SCREEN_W/2, SCREEN_H/2, 31, HOCKEY_BLUE_LINE);
        
        if (diving) {
          hockey_drawGoalie(tft, hockey_goalieX, 65, true, true);
        } else {
          hockey_drawGoalie(tft, hockey_goalieX, 60, true, false);
        }
        
        tft.setTextFont(4);
        tft.setTextDatum(MC_DATUM);
        
        if (blocked) {
          hockey_goalieScore++;
          tft.setTextColor(TFT_GREEN, HOCKEY_ICE);
          tft.drawString("SAVE!", SCREEN_W/2, SCREEN_H/2);
          playSound("/sounds/beep.wav", false);
        } else {
          tft.setTextColor(HOCKEY_RED_LINE, HOCKEY_ICE);
          tft.drawString("GOAL!", SCREEN_W/2, SCREEN_H/2);
          playSound("/sounds/beep_go.wav", false);
        }
        
        delay(1500);
        hockey_currentShot++;
      }
      
      delay(20);
    }
  }
}

//=============================================================================
// SHOOTER ROUND
//=============================================================================
void hockey_shooterRound(TFT_eSPI &tft) {
  hockey_currentShot = 0;
  hockey_lastRotaryPos = rotaryPos;
  
  tft.fillScreen(HOCKEY_RED_LINE);
  tft.setTextColor(HOCKEY_WHITE, HOCKEY_RED_LINE);
  tft.setTextFont(4);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("YOU'RE THE", SCREEN_W/2, 80);
  tft.drawString("SHOOTER!", SCREEN_W/2, 110);
  tft.setTextFont(2);
  tft.drawString("Score 5 goals!", SCREEN_W/2, 150);
  
  playSound("/sounds/beep.wav", false);
  delay(2000);
  
  while (hockey_currentShot < 5) {
    hockey_drawRink(tft);
    hockey_drawCrowd(tft, hockey_crowdAnimFrame);
    hockey_drawGoal(tft, 35, false);
    hockey_drawHUD(tft, hockey_goalieScore, hockey_shooterScore, hockey_currentShot, false);
    
    int aiGoalieX = 160;
    hockey_drawGoalie(tft, aiGoalieX, 60, false, false);
    
    hockey_drawShooter(tft, SCREEN_W/2, 180);
    
    hockey_shootDirection = 0;
    int aimX = 160;
    
    tft.setTextFont(2);
    tft.setTextColor(HOCKEY_BLACK, HOCKEY_ICE);
    tft.setTextDatum(BC_DATUM);
    tft.drawString("ROTATE to aim, BUTTON to shoot", SCREEN_W/2, SCREEN_H - 5);
    
    int lastRotary = rotaryPos;
    int lastBtn = HIGH;
    bool shotTaken = false;
    
    while (!shotTaken) {
      updateAudio();
      
      if (millis() - hockey_lastCrowdUpdate > 200) {
        hockey_lastCrowdUpdate = millis();
        hockey_crowdAnimFrame++;
        hockey_drawCrowd(tft, hockey_crowdAnimFrame);
      }
      
      int rotDiff = rotaryPos - lastRotary;
      
      if (rotDiff > 2) {
        hockey_shootDirection = min(1, hockey_shootDirection + 1);
        lastRotary = rotaryPos;
      } else if (rotDiff < -2) {
        hockey_shootDirection = max(-1, hockey_shootDirection - 1);
        lastRotary = rotaryPos;
      }
      
      int newAimX;
      if (hockey_shootDirection < 0) newAimX = 110;
      else if (hockey_shootDirection > 0) newAimX = 210;
      else newAimX = 160;
      
      if (newAimX != aimX) {
        tft.fillRect(0, 140, SCREEN_W, 30, HOCKEY_ICE);
        
        tft.drawCircle(SCREEN_W/2, SCREEN_H/2, 30, HOCKEY_BLUE_LINE);
        tft.drawCircle(SCREEN_W/2, SCREEN_H/2, 31, HOCKEY_BLUE_LINE);
        
        aimX = newAimX;
      }
      
      tft.fillTriangle(aimX, 145, aimX - 8, 155, aimX + 8, 155, HOCKEY_BLACK);
      tft.setTextFont(1);
      tft.setTextColor(HOCKEY_BLACK, HOCKEY_ICE);
      tft.setTextDatum(MC_DATUM);
      
      if (hockey_shootDirection < 0) tft.drawString("LEFT", aimX, 165);
      else if (hockey_shootDirection > 0) tft.drawString("RIGHT", aimX, 165);
      else tft.drawString("CENTER", aimX, 165);
      
      int btn = digitalRead(PIN_KO);
      if (btn == LOW && lastBtn == HIGH) {
        shotTaken = true;
        
        tft.fillRect(0, 140, SCREEN_W, 30, HOCKEY_ICE);
        tft.fillRect(0, SCREEN_H - 20, SCREEN_W, 20, HOCKEY_ICE);
        
        tft.drawCircle(SCREEN_W/2, SCREEN_H/2, 30, HOCKEY_BLUE_LINE);
        tft.drawCircle(SCREEN_W/2, SCREEN_H/2, 31, HOCKEY_BLUE_LINE);
        
        int aiTargetX;
        if (random(0, 100) < 50) {
          if (hockey_shootDirection < 0) aiTargetX = 110;
          else if (hockey_shootDirection > 0) aiTargetX = 210;
          else aiTargetX = 160;
        } else {
          aiTargetX = random(0, 2) == 0 ? 110 : 210;
        }
        
        for (int i = 0; i <= 10; i++) {
          int moveX = aiGoalieX + ((aiTargetX - aiGoalieX) * i) / 10;
          
          tft.fillRect(moveX - 40, 30, 80, 75, HOCKEY_ICE);
          
          hockey_drawGoal(tft, 35, false);
          
          hockey_drawGoalie(tft, moveX, 60, false, false);
          
          delay(25);
        }
        aiGoalieX = aiTargetX;
        
        playSound("/sounds/beep_go.wav", false);
        
        int targetX = aimX;
        int prevPuckX = -1;
        int prevPuckY = -1;
        
        for (int step = 0; step <= 20; step++) {
          int puckX = SCREEN_W/2 + ((targetX - SCREEN_W/2) * step) / 20;
          int puckY = 200 - (step * 8);
          
          if (prevPuckX >= 0 && prevPuckY > 105 && prevPuckY < 165) {
            tft.fillCircle(prevPuckX, prevPuckY, 7, HOCKEY_ICE);
            
            if (prevPuckY > 110 && prevPuckY < 135) {
              tft.drawCircle(SCREEN_W/2, SCREEN_H/2, 30, HOCKEY_BLUE_LINE);
              tft.drawCircle(SCREEN_W/2, SCREEN_H/2, 31, HOCKEY_BLUE_LINE);
            }
          }
          
          hockey_drawPuck(tft, puckX, puckY);
          
          prevPuckX = puckX;
          prevPuckY = puckY;
          
          delay(30);
        }
        
        tft.fillRect(0, 100, SCREEN_W, 80, HOCKEY_ICE);
        
        tft.drawCircle(SCREEN_W/2, SCREEN_H/2, 30, HOCKEY_BLUE_LINE);
        tft.drawCircle(SCREEN_W/2, SCREEN_H/2, 31, HOCKEY_BLUE_LINE);
        
        hockey_drawShooter(tft, SCREEN_W/2, 180);
        
        tft.fillRect(aiGoalieX - 40, 30, 80, 75, HOCKEY_ICE);
        hockey_drawGoal(tft, 35, false);
        hockey_drawGoalie(tft, aiGoalieX, 60, false, false);
        
        bool scored = (abs(aimX - aiGoalieX) > 30);
        
        tft.setTextFont(4);
        tft.setTextDatum(MC_DATUM);
        
        if (scored) {
          hockey_shooterScore++;
          tft.setTextColor(TFT_GREEN, HOCKEY_ICE);
          tft.drawString("GOAL!", SCREEN_W/2, SCREEN_H/2);
          playSound("/sounds/crowd-cheer-and-applause.wav", false);
        } else {
          tft.setTextColor(HOCKEY_RED_LINE, HOCKEY_ICE);
          tft.drawString("SAVE!", SCREEN_W/2, SCREEN_H/2);
          playSound("/sounds/beep.wav", false);
        }
        
        delay(1500);
        hockey_currentShot++;
      }
      
      lastBtn = btn;
      delay(50);
    }
  }
}

//=============================================================================
// GAME OVER SCREEN
//=============================================================================
void hockey_gameOver(TFT_eSPI &tft) {
  tft.fillScreen(HOCKEY_BLACK);
  
  hockey_drawCrowd(tft, hockey_crowdAnimFrame);
  
  tft.setTextColor(HOCKEY_YELLOW, HOCKEY_BLACK);
  tft.setTextFont(4);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("GAME OVER", SCREEN_W/2, 50);
  
  tft.fillRect(40, 75, SCREEN_W - 80, 3, HOCKEY_RED_LINE);
  
  tft.setTextFont(2);
  tft.setTextColor(HOCKEY_WHITE, HOCKEY_BLACK);
  
  char buf[32];
  snprintf(buf, sizeof(buf), "Goals Scored: %d", hockey_shooterScore);
  tft.drawString(buf, SCREEN_W/2, 95);
  
  snprintf(buf, sizeof(buf), "Shots Blocked: %d", hockey_goalieScore);
  tft.drawString(buf, SCREEN_W/2, 115);
  
  tft.fillRect(40, 135, SCREEN_W - 80, 3, HOCKEY_BLUE_LINE);
  
  tft.setTextFont(4);
  
  if (hockey_shooterScore > (5 - hockey_goalieScore)) {
    tft.setTextColor(TFT_GREEN, HOCKEY_BLACK);
    tft.drawString("YOU WIN!", SCREEN_W/2, 160);
    
    uint16_t goldColor = 0xFEA0;
    tft.fillRect(SCREEN_W/2 - 8, 195, 16, 20, goldColor);
    tft.fillRect(SCREEN_W/2 - 15, 190, 30, 8, goldColor);
    tft.fillRect(SCREEN_W/2 - 4, 215, 8, 10, goldColor);
    tft.fillRect(SCREEN_W/2 - 12, 223, 24, 4, goldColor);
    tft.drawRect(SCREEN_W/2 - 15, 190, 30, 8, HOCKEY_BLACK);
    tft.drawRect(SCREEN_W/2 - 8, 195, 16, 20, HOCKEY_BLACK);
    
    tft.drawCircle(SCREEN_W/2 - 18, 200, 5, goldColor);
    tft.drawCircle(SCREEN_W/2 + 18, 200, 5, goldColor);
    
    playSound("/sounds/crowd-cheer-and-applause.wav", true);
    
  } else if (hockey_shooterScore == (5 - hockey_goalieScore)) {
    tft.setTextColor(HOCKEY_YELLOW, HOCKEY_BLACK);
    tft.drawString("TIE GAME!", SCREEN_W/2, 160);
    
    tft.setTextFont(2);
    tft.drawString("Great balance!", SCREEN_W/2, 195);
    
    playSound("/sounds/beep.wav", false);
    
  } else {
    tft.setTextColor(HOCKEY_RED_LINE, HOCKEY_BLACK);
    tft.drawString("YOU LOSE!", SCREEN_W/2, 160);
    
    tft.setTextFont(2);
    tft.drawString("Better luck next time!", SCREEN_W/2, 195);
    
    playSound("/sounds/beep.wav", false);
  }
  
  tft.setTextFont(2);
  tft.setTextColor(HOCKEY_WHITE, HOCKEY_BLACK);
  tft.drawString("Press button to continue", SCREEN_W/2, SCREEN_H - 10);
  
  int lastBtn = HIGH;
  while (true) {
    updateAudio();
    
    if (millis() - hockey_lastCrowdUpdate > 200) {
      hockey_lastCrowdUpdate = millis();
      hockey_crowdAnimFrame++;
      hockey_drawCrowd(tft, hockey_crowdAnimFrame);
    }
    
    int btn = digitalRead(PIN_KO);
    if (btn == LOW && lastBtn == HIGH) {
      break;
    }
    lastBtn = btn;
    delay(50);
  }
  
  while (digitalRead(PIN_KO) == LOW) delay(10);
  delay(400);
}

//=============================================================================
// MAIN GAME FUNCTION
//=============================================================================
void run_Hockey(TFT_eSPI &tft) {
  tft.setRotation(3);
  pinMode(PIN_KO, INPUT_PULLUP);
  
  hockey_splashScreen1(tft);
  hockey_splashScreen2(tft);
  
  hockey_gameState = HOCKEY_GOALIE_ROUND;
  hockey_goalieScore = 0;
  hockey_shooterScore = 0;
  hockey_currentShot = 0;
  hockey_shotInProgress = false;
  hockey_lastRotaryPos = rotaryPos;
  hockey_crowdAnimFrame = 0;
  hockey_lastCrowdUpdate = millis();
  
  hockey_goalieRound(tft);
  
  tft.fillScreen(HOCKEY_BLACK);
  tft.setTextColor(HOCKEY_YELLOW, HOCKEY_BLACK);
  tft.setTextFont(4);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("SWITCHING", SCREEN_W/2, SCREEN_H/2 - 20);
  tft.drawString("SIDES!", SCREEN_W/2, SCREEN_H/2 + 10);
  
  playSound("/sounds/beep.wav", false);
  delay(2000);
  
  hockey_shooterRound(tft);
  
  stopAudio();
  hockey_gameOver(tft);
  
  stopAudio();
}

#endif // HOCKEY_H