#include <TFT_eSPI.h>
#include <SPI.h>
#include <SD.h>
#include "AudioFileSourceSD.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"

// PAM8302A Pins
//#define AMP_SD_PIN     32

// SD Card
#define SD_CS    13
#define SD_MOSI  15
#define SD_MISO  2
#define SD_SCK   14

// Controls
#define PIN_TRA 19
#define PIN_TRB 21
#define PIN_KO  22  //was 22
#define PIN_PUSH 22  // Change from 32 to 12

// === FORWARD DECLARATIONS FOR AUDIO FUNCTIONS ===
void initAudio();
void playSound(const char *path, bool stopCurrent);
void updateAudio();
void stopAudio();
// === END FORWARD DECLARATIONS ===

#include "Tempest_Gordo.h"
#include "Run_Gordo_Run.h"
#include "Captain_Gordo.h"
#include "Star_Wars_ascii.h"
#include "Rhythm_Runner.h"
#include "Winter_Olympics.h"

TFT_eSPI tft = TFT_eSPI();

AudioGeneratorWAV *wav = nullptr;
AudioFileSourceSD *file = nullptr;
AudioOutputI2S *out = nullptr;
bool audioReady = false;

volatile int rotaryPos = 0;
volatile bool lastA = 0;
volatile unsigned long lastRotaryTime = 0;

void run_Tempest_Gordo(TFT_eSPI &tft);
void run_Run_Gordo_Run(TFT_eSPI &tft);
void run_Captain_Gordo(TFT_eSPI &tft);
void run_StarWarsAscii(TFT_eSPI &tft);
void run_Rhythm_Runner(TFT_eSPI &tft);  
void run_Winter_Olympics(TFT_eSPI &tft);

// ← MODIFY THIS ARRAY
const char *game_titles[] = { 
    "Tempest Gordo", 
    "Run Gordo Run!", 
    "Captain Gordo", 
    "Star Wars: Ascii",
    "Rhythm Runner",
    "Winter Olympics"
};

// ← MODIFY THIS ENUM
enum { 
    TEMPEST_GORDO_INDEX = 0, 
    RUN_GORDO_RUN_INDEX, 
    CAPTAIN_GORDO_INDEX, 
    STAR_WARS_ASCII_INDEX, 
    RHYTHM_RUNNER_INDEX,
    WINTER_OLYMPICS_INDEX,
    NUM_GAMES 
};

int selected_game = 0;
int currentLevel = 1;

// NEW - Amp always on (Shutdown tied to 3.3V)
void ampOn() {
  // Nothing - amp is always enabled
}

void ampOff() {
  // Nothing - amp is always enabled
}

void stopAudio() {
  if (wav) {
    if (wav->isRunning()) {
      wav->stop();
    }
    delete wav;
    delete file;
    wav = nullptr;
    file = nullptr;
  }
}

void initAudio() {
  Serial.println("\n=== Audio Init ===");
    
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  
  if (!SD.begin(SD_CS)) {
    Serial.println("✗ SD failed!");
    return;
  }
  Serial.println("✓ SD ready");
  
  out = new AudioOutputI2S(0, AudioOutputI2S::EXTERNAL_I2S);
  
  // PCM5102A wiring (restore original pins):
  out->SetPinout(26, 25, 33);  // BCLK=26, LRC=25, DIN=33
  
  //Volume control:
  //out->SetGain(0.3);
  out->SetGain(0.5);  // 50% louder than (0.3)

  audioReady = true;
  Serial.println("✓ Audio ready (PCM5102A I2S DAC)\n");
}

void playSound(const char *path, bool stopCurrent) {
    if (!audioReady || !SD.exists(path)) {
    return;
  }
  
  if (stopCurrent && wav) {
    stopAudio();
    delay(20);
  }
  
  if (!stopCurrent && wav && wav->isRunning()) {
    return;
  }
  
  Serial.print("♪ ");
  Serial.println(path);
  
  file = new AudioFileSourceSD(path);
  wav = new AudioGeneratorWAV();
  
  if (!wav->begin(file, out)) {
    Serial.println("  ✗ Play failed");
    delete wav;
    delete file;
    wav = nullptr;
    file = nullptr;
  }
}

void updateAudio() {
  if (wav && wav->isRunning()) {
    if (!wav->loop()) {
      wav->stop();
      delete wav;
      delete file;
      wav = nullptr;
      file = nullptr;
      
      Serial.println("  Audio stopped");
    }
  }
}

// ORIGINAL Menu vector font
struct MENU_VecStroke { int x0, y0, x1, y1; };
struct MENU_VecLetter { const MENU_VecStroke *strokes; int n; int w; };
const MENU_VecStroke MENU_strokes_R[] = {{0,12,0,0},{0,0,8,0},{8,0,8,6},{8,6,0,6},{0,6,8,12}};
const MENU_VecStroke MENU_strokes_O[] = {{0,0,8,0},{8,0,8,12},{8,12,0,12},{0,12,0,0}};
const MENU_VecStroke MENU_strokes_T[] = {{4,0,4,12},{0,0,8,0}};
const MENU_VecStroke MENU_strokes_A[] = {{0,12,4,0},{4,0,8,12},{2,6,6,6}};
const MENU_VecStroke MENU_strokes_Y[] = {{0,0,4,6},{8,0,4,6},{4,6,4,12}};
const MENU_VecStroke MENU_strokes_C[] = {{8,0,0,0},{0,0,0,12},{0,12,8,12}};
const MENU_VecStroke MENU_strokes_D[] = {{0,12,0,0},{0,0,7,2},{7,2,8,6},{8,6,7,10},{7,10,0,12}};
const MENU_VecStroke MENU_strokes_E[] = {{8,0,0,0},{0,0,0,12},{0,6,6,6},{0,12,8,12}};
const MENU_VecStroke MENU_strokes_M[] = {{0,12,0,0},{0,0,4,6},{4,6,8,0},{8,0,8,12}};
const MENU_VecStroke MENU_strokes_V[] = {{0,0,4,12},{4,12,8,0}};
const MENU_VecLetter MENU_vecFont[] = {
  {MENU_strokes_R,5,9},{MENU_strokes_O,4,9},{MENU_strokes_T,2,9},{MENU_strokes_A,3,9},{MENU_strokes_Y,3,9},
  {MENU_strokes_C,3,9},{MENU_strokes_D,5,9},{MENU_strokes_E,4,9},{MENU_strokes_M,4,9},{MENU_strokes_V,2,9}
};
int menuGetVecFontIdx(char c) {
  switch(c) {
    case 'R':return 0; case 'O':return 1; case 'T':return 2; case 'A':return 3; case 'Y':return 4;
    case 'C':return 5; case 'D':return 6; case 'E':return 7; case 'M':return 8; case 'V':return 9;
    default: return -1;
  }
}
const uint16_t menuRainbow[7] = {TFT_RED,0xFD20,TFT_YELLOW,TFT_GREEN,TFT_CYAN,TFT_BLUE,TFT_MAGENTA};

void menuDrawVecLetter(TFT_eSPI &tft, char c, int x, int y, int scale, int trailLen) {
  int idx = menuGetVecFontIdx(c);
  if (idx < 0) return;
  for (int t = 0; t < trailLen; t++) {
    uint16_t color = menuRainbow[6 - (t % 7)];
    int trailOffset = t * scale * 2;
    for (int i = 0; i < MENU_vecFont[idx].n; i++) {
      int x0 = x + MENU_vecFont[idx].strokes[i].x0 * scale;
      int y0 = y + (MENU_vecFont[idx].strokes[i].y0 * scale) / 2 + trailOffset;
      int x1 = x + MENU_vecFont[idx].strokes[i].x1 * scale;
      int y1 = y + (MENU_vecFont[idx].strokes[i].y1 * scale) / 2 + trailOffset;
      tft.drawLine(x0, y0, x1, y1, color);
    }
  }
  for (int i = 0; i < MENU_vecFont[idx].n; i++) {
    int x0 = x + MENU_vecFont[idx].strokes[i].x0 * scale;
    int y0 = y + (MENU_vecFont[idx].strokes[i].y0 * scale) / 2;
    int x1 = x + MENU_vecFont[idx].strokes[i].x1 * scale;
    int y1 = y + (MENU_vecFont[idx].strokes[i].y1 * scale) / 2;
    tft.drawLine(x0, y0, x1, y1, TFT_WHITE);
  }
}

void menuDrawVecLetterSolid(TFT_eSPI &tft, char c, int x, int y, int scale, uint16_t color) {
  int idx = menuGetVecFontIdx(c);
  if (idx < 0) return;
  for (int i = 0; i < MENU_vecFont[idx].n; i++) {
    int x0 = x + MENU_vecFont[idx].strokes[i].x0 * scale;
    int y0 = y + (MENU_vecFont[idx].strokes[i].y0 * scale) / 2;
    int x1 = x + MENU_vecFont[idx].strokes[i].x1 * scale;
    int y1 = y + (MENU_vecFont[idx].strokes[i].y1 * scale) / 2;
    tft.drawLine(x0, y0, x1, y1, color);
  }
}

void drawSplashSideWords(TFT_eSPI &tft) {
  const char *leftWord = "ROTARY";
  const char *rightWord = "ARCADE";
  int scale = 3, spacing = 24;
  int yStart = (240 - 6 * spacing) / 2;
  
  for (int i = 0; i < 6; i++) {
    menuDrawVecLetterSolid(tft, leftWord[i], 8, yStart + i * spacing, scale, menuRainbow[i % 7]);
    menuDrawVecLetterSolid(tft, rightWord[i], 320 - 8 - 24, yStart + i * spacing, scale, menuRainbow[i % 7]);
  }
}

void splashScreenKnob(TFT_eSPI &tft) {
  tft.fillScreen(TFT_BLACK);
  drawSplashSideWords(tft);

  int cx = 160, cy = 132, r = 72;
  tft.fillCircle(cx, cy, r, TFT_BLACK);
  tft.drawCircle(cx, cy, r, TFT_WHITE);

  playSound("/sounds/splash.wav");

  unsigned long duration = 1500;
  int nSteps = 36;
  unsigned long stepDelay = duration / nSteps;
  
  for (int step = 0; step <= nSteps; step++) {
    unsigned long stepStart = millis();
    
    tft.fillCircle(cx, cy, r - 18, TFT_BLACK);
    tft.drawCircle(cx, cy, r - 18, TFT_WHITE);

    for (int i = 0; i < 36; i++) {
      float a = -PI + i * (2 * PI / 36);
      uint16_t color = menuRainbow[i % 7];
      int x0 = cx + cos(a) * (r + 4);
      int y0 = cy + sin(a) * (r + 4);
      int x1 = cx + cos(a) * (r + 17);
      int y1 = cy + sin(a) * (r + 17);
      tft.drawLine(x0, y0, x1, y1, color);
      tft.drawLine(x0 + 1, y0 + 1, x1 + 1, y1 + 1, color);
    }

    float theta = -PI / 2 + (2 * PI) * step / nSteps;
    for (int w = 0; w < 4; w++)
      tft.drawLine(cx, cy, cx + cos(theta) * (r - 18 - w), cy + sin(theta) * (r - 18 - w), menuRainbow[step % 7]);
    int px = cx + cos(theta) * (r - 18);
    int py = cy + sin(theta) * (r - 18);
    tft.fillCircle(px, py, 5, menuRainbow[step % 7]);
    
    while (millis() - stepStart < stepDelay) {
      updateAudio();
      delay(1);
    }
  }
  
  while (wav && wav->isRunning()) {
    updateAudio();
    delay(10);
  }
}

void drawMenuTitle(TFT_eSPI &tft) {
  int scale = 3, trailLen = 6, spacing = 9 * scale;
  const char *rotary = "ROTARY", *arcade = "ARCADE";
  int textWidthR = 6 * spacing, textWidthA = 6 * spacing;
  int gap = 6, totalWidth = textWidthR + gap + textWidthA;
  int xRotary = ((320 - totalWidth) / 2) + 5;
  int xArcade = xRotary + textWidthR + gap;
  int y = 6;
  
  for (int i = 0; i < 6; i++) {
    menuDrawVecLetter(tft, rotary[i], xRotary + i * spacing, y, scale, trailLen);
    menuDrawVecLetter(tft, arcade[i], xArcade + i * spacing, y, scale, trailLen);
  }
}

void drawMenuItem(TFT_eSPI &tft, const char *title, int y, bool selected) {
  int bubble_w = 240, bubble_h = 54;
  int cx = 320 / 2, cy = y + bubble_h / 2;
  if (selected) {
    tft.fillRoundRect(cx - bubble_w / 2, cy - bubble_h / 2, bubble_w, bubble_h, 18, TFT_WHITE);
    tft.setTextColor(TFT_RED, TFT_WHITE);
  } else {
    uint16_t lightBlue = 0x5D9F;
    tft.fillRoundRect(cx - bubble_w / 2, cy - bubble_h / 2, bubble_w, bubble_h, 18, lightBlue);
    tft.setTextColor(TFT_WHITE, lightBlue);
  }
  tft.setTextFont(4);
  tft.setTextDatum(CC_DATUM);
  tft.drawString(title, cx, cy);
}

void drawMenu(TFT_eSPI &tft, int selected, const char *const *titles, int numTitles, int visible) {
  static int last_selected = -1;
  static int last_start = -1;
  
  int y0 = 66;
  
  // Calculate which items to show (scrolling window)
  int start = selected - visible / 2;
  if (start < 0) start = 0;
  if (start > numTitles - visible) start = numTitles - visible;
  if (numTitles <= visible) start = 0;

  // First draw or window changed - redraw everything
  if (last_selected == -1 || start != last_start) {
    tft.fillScreen(TFT_BLACK);
    drawMenuTitle(tft);
    
    for (int i = 0; i < visible; i++) {
      int idx = start + i;
      if (idx < numTitles) {
        int ypos = y0 + i * 60;
        drawMenuItem(tft, titles[idx], ypos, idx == selected);
      }
    }
    
    last_start = start;
  } 
  // Selection changed but same window - only redraw changed items
  else if (selected != last_selected) {
    // Find old and new positions in visible window
    int oldPos = last_selected - start;
    int newPos = selected - start;
    
    if (oldPos >= 0 && oldPos < visible) {
      int yOld = y0 + oldPos * 60;
      drawMenuItem(tft, titles[last_selected], yOld, false);
    }
    
    if (newPos >= 0 && newPos < visible) {
      int yNew = y0 + newPos * 60;
      drawMenuItem(tft, titles[selected], yNew, true);
    }
  }
  
  last_selected = selected;
}

void IRAM_ATTR handleRotary() {
  unsigned long now = millis();
  if (now - lastRotaryTime < 5) return;
  
  bool A = digitalRead(PIN_TRA);
  if (A != lastA) {
    rotaryPos += (A == digitalRead(PIN_TRB)) ? -1 : 1;
    lastA = A;
    lastRotaryTime = now;
  }
}


void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== ROTARY ARCADE ===");

  pinMode(PIN_TRA, INPUT_PULLUP);
  pinMode(PIN_TRB, INPUT_PULLUP);
  pinMode(PIN_KO, INPUT_PULLUP);
  pinMode(PIN_PUSH, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(PIN_TRA), handleRotary, CHANGE);

  tft.init();
  tft.setRotation(3);
  
  initAudio();
  
  // === TEST SOUND FILES ===
  Serial.println("\n=== Checking Sound Files ===");
  if (SD.exists("/sounds/Laser_Shoot.wav")) 
      Serial.println("✓ Laser_Shoot.wav found");
  else 
      Serial.println("✗ Laser_Shoot.wav MISSING!");
  
  if (SD.exists("/sounds/Boss_died.wav")) 
      Serial.println("✓ Boss_died.wav found");
  else 
      Serial.println("✗ Boss_died.wav MISSING!");
  
  if (SD.exists("/sounds/explosion1.wav")) 
      Serial.println("✓ explosion1.wav found");
  else 
      Serial.println("✗ explosion1.wav MISSING!");
  
  if (SD.exists("/sounds/splash_Captain_Gordo.wav")) 
      Serial.println("✓ splash_Captain_Gordo.wav found");
  else 
      Serial.println("✗ splash_Captain_Gordo.wav MISSING!");
  
  Serial.println("=========================\n");
  // === END TEST ===
  
  splashScreenKnob(tft);
  
  drawMenu(tft, 0, game_titles, NUM_GAMES, 3);
  Serial.println("✓ Ready!\n");
}

void loop() {
  static int lastMenuPos = 0;
  static int lastButton = HIGH;
  static unsigned long lastMenuChange = 0;
  static bool soundPlayed = false;
  static int menuPulseCount = 0;
  
  updateAudio();  // Keep this for sound

  // NEW: Count 3 pulses before changing menu
  int currentRotary = rotaryPos;
  static int lastMenuRotary = 0;
  int rotaryDiff = currentRotary - lastMenuRotary;
  
  if (rotaryDiff >= 3) {
    menuPulseCount++;
    lastMenuRotary = currentRotary;
  } else if (rotaryDiff <= -3) {
    menuPulseCount--;
    lastMenuRotary = currentRotary;
  }
  
  int menuPos = ((menuPulseCount % NUM_GAMES) + NUM_GAMES) % NUM_GAMES;
  
  if (menuPos != lastMenuPos && millis() - lastMenuChange > 200) {
    selected_game = menuPos;
    drawMenu(tft, selected_game, game_titles, NUM_GAMES, 3);
    
    if (!soundPlayed) {
      stopAudio();
      delay(20);
      playSound("/sounds/Menu_items.wav");
      soundPlayed = true;
    }
    
    lastMenuPos = menuPos;
    lastMenuChange = millis();
  } else {
    if (millis() - lastMenuChange > 300) {
      soundPlayed = false;
    }
  }

  int button = digitalRead(PIN_KO);  // ← Use PIN_KO, not PIN_PUSH!
  if (button == LOW && lastButton == HIGH) {
    stopAudio();
    
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("Loading...", 160, 120);

    switch (selected_game) {
      case TEMPEST_GORDO_INDEX: run_Tempest_Gordo(tft); break;
      case RUN_GORDO_RUN_INDEX: run_Run_Gordo_Run(tft); break;
      case CAPTAIN_GORDO_INDEX: run_Captain_Gordo(tft); break;
      case STAR_WARS_ASCII_INDEX: run_StarWarsAscii(tft); break;
      case RHYTHM_RUNNER_INDEX: run_Rhythm_Runner(tft); break;
      case WINTER_OLYMPICS_INDEX: run_Winter_Olympics(tft); break; 
    }
    
    tft.init();
    tft.setRotation(3);
    
    // Reset menu position after returning from game
    lastMenuRotary = rotaryPos;
    menuPulseCount = selected_game;
    
    drawMenu(tft, selected_game, game_titles, NUM_GAMES, 3);
  }
  lastButton = button;
  delay(20);  // Keep this small delay
}