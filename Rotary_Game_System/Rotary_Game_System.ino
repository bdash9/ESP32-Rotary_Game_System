#include <TFT_eSPI.h>
#include <SPI.h>
#include "Tempest_Gordo.h"

// Rotary Encoder Pins
#define PIN_TRA      19
#define PIN_TRB      21
#define PIN_KO       22

TFT_eSPI tft = TFT_eSPI();

volatile int rotaryPos = 0;
volatile bool lastA = 0;

// --- Forward declarations for games ---
void run_Tempest_Gordo(TFT_eSPI &tft);

// --- Game Menu Items ---
const char* game_titles[] = {
    "Tempest Gordo",
    "Game 2",
    "Game 3"
};
enum {
    TEMPEST_GORDO_INDEX = 0,
    GAME2_INDEX,
    GAME3_INDEX,
    NUM_GAMES
};
int selected_game = 0;

// --- MENU Vector Font ---
struct MENU_VecStroke { int x0, y0, x1, y1; };
struct MENU_VecLetter { const MENU_VecStroke* strokes; int n; int w; };
const MENU_VecStroke MENU_strokes_R[] = {{0,12,0,0},{0,0,8,0},{8,0,8,6},{8,6,0,6},{0,6,8,12}};
const MENU_VecStroke MENU_strokes_O[] = {{0,0,8,0},{8,0,8,12},{8,12,0,12},{0,12,0,0}};
const MENU_VecStroke MENU_strokes_T[] = {{4,0,4,12},{0,0,8,0}};
const MENU_VecStroke MENU_strokes_A[] = {{0,12,4,0},{4,0,8,12},{2,6,6,6}};
const MENU_VecStroke MENU_strokes_Y[] = {{0,0,4,6},{8,0,4,6},{4,6,4,12}};
const MENU_VecStroke MENU_strokes_C[] = {{8,0,0,0},{0,0,0,12},{0,12,8,12}};
const MENU_VecStroke MENU_strokes_D[] = {{0,12,0,0},{0,0,7,2},{7,2,8,6},{8,6,7,10},{7,10,0,12}};
const MENU_VecStroke MENU_strokes_E[] = {{8,0,0,0},{0,0,0,12},{0,6,6,6},{0,12,8,12}};
const MENU_VecLetter MENU_vecFont[] = {
    {MENU_strokes_R, sizeof(MENU_strokes_R)/sizeof(MENU_VecStroke), 9},
    {MENU_strokes_O, sizeof(MENU_strokes_O)/sizeof(MENU_VecStroke), 9},
    {MENU_strokes_T, sizeof(MENU_strokes_T)/sizeof(MENU_VecStroke), 9},
    {MENU_strokes_A, sizeof(MENU_strokes_A)/sizeof(MENU_VecStroke), 9},
    {MENU_strokes_Y, sizeof(MENU_strokes_Y)/sizeof(MENU_VecStroke), 9},
    {MENU_strokes_C, sizeof(MENU_strokes_C)/sizeof(MENU_VecStroke), 9},
    {MENU_strokes_D, sizeof(MENU_strokes_D)/sizeof(MENU_VecStroke), 9},
    {MENU_strokes_E, sizeof(MENU_strokes_E)/sizeof(MENU_VecStroke), 9},
};
int menuGetVecFontIdx(char c) {
    switch(c) {
        case 'R': return 0; case 'O': return 1; case 'T': return 2; case 'A': return 3;
        case 'Y': return 4; case 'C': return 5; case 'D': return 6; case 'E': return 7;
        default: return -1;
    }
}
const uint16_t menuRainbow[7] = {TFT_RED, 0xFD20, TFT_YELLOW, TFT_GREEN, TFT_CYAN, TFT_BLUE, TFT_MAGENTA};

void menuDrawVecLetter(TFT_eSPI &tft, char c, int x, int y, int scale, int trailLen) {
    int idx = menuGetVecFontIdx(c);
    if(idx < 0) return;
    // Draw rainbow trails first (downward), white on top
    for (int t = 0; t < trailLen; t++) {
        uint16_t color = menuRainbow[6 - (t % 7)];
        int trailOffset = t * scale * 2; // trail offset down
        for (int i = 0; i < MENU_vecFont[idx].n; i++) {
            int x0 = x + MENU_vecFont[idx].strokes[i].x0 * scale;
            int y0 = y + (MENU_vecFont[idx].strokes[i].y0 * scale)/2 + trailOffset;
            int x1 = x + MENU_vecFont[idx].strokes[i].x1 * scale;
            int y1 = y + (MENU_vecFont[idx].strokes[i].y1 * scale)/2 + trailOffset;
            tft.drawLine(x0, y0, x1, y1, color);
        }
    }
    for (int i = 0; i < MENU_vecFont[idx].n; i++) {
        int x0 = x + MENU_vecFont[idx].strokes[i].x0 * scale;
        int y0 = y + (MENU_vecFont[idx].strokes[i].y0 * scale)/2;
        int x1 = x + MENU_vecFont[idx].strokes[i].x1 * scale;
        int y1 = y + (MENU_vecFont[idx].strokes[i].y1 * scale)/2;
        tft.drawLine(x0, y0, x1, y1, TFT_WHITE);
    }
}

void splashScreenKnob(TFT_eSPI &tft) {
    tft.fillScreen(TFT_BLACK);
    int cx = 160, cy = 132;
    int r = 72;
    tft.fillCircle(cx, cy, r, TFT_BLACK);
    tft.drawCircle(cx, cy, r, TFT_WHITE);

    unsigned long duration = 2500;
    int nSteps = 36;
    for (int step = 0; step <= nSteps; step++) {
        tft.fillCircle(cx, cy, r-18, TFT_BLACK);
        tft.drawCircle(cx, cy, r-18, TFT_WHITE);

        // Twice as thick ticks
for (int i=0; i<36; i++) {
    float a = -PI + i*(2*PI/36); // covers -PI to PI
    uint16_t color = menuRainbow[i%7];
    int x0 = cx + cos(a)*(r+4);
    int y0 = cy + sin(a)*(r+4);
    int x1 = cx + cos(a)*(r+17);
    int y1 = cy + sin(a)*(r+17);
    tft.drawLine(x0, y0, x1, y1, color);
    tft.drawLine(x0+1, y0+1, x1+1, y1+1, color);
}

        float theta = -PI/2 + (PI)*step/nSteps;
        for (int w=0; w<4; w++)
            tft.drawLine(cx, cy, cx+cos(theta)*(r-18-w), cy+sin(theta)*(r-18-w), menuRainbow[step%7]);
        int px = cx+cos(theta)*(r-18);
        int py = cy+sin(theta)*(r-18);
        tft.fillCircle(px, py, 5, menuRainbow[step%7]);
        delay(duration/nSteps);
    }
}

void drawMenuTitle(TFT_eSPI &tft) {
    int scale = 3, trailLen = 6, spacing = 9*scale;
    const char* rotary = "ROTARY";
    const char* arcade = "ARCADE";
    int nR = 6, nA = 6;
    int textWidthR = nR * spacing;
    int textWidthA = nA * spacing;
    int gap = 6;
    int totalWidth = textWidthR + gap + textWidthA;
    int offset = 5; // move right by 10 pixels (adjust as needed)
    int xRotary = ((320 - totalWidth) / 2) + offset;
    int xArcade = xRotary + textWidthR + gap;
    int y = 6; // Higher up
    for (int i = 0; i < nR; i++)
        menuDrawVecLetter(tft, rotary[i], xRotary + i * spacing, y, scale, trailLen);
    for (int i = 0; i < nA; i++)
        menuDrawVecLetter(tft, arcade[i], xArcade + i * spacing, y, scale, trailLen);
}

void drawMenuItem(TFT_eSPI &tft, const char* title, int y, bool selected) {
    int bubble_w = 240, bubble_h = 54;
    int cx = 320/2, cy = y + bubble_h/2;
    if (selected) {
        tft.fillRoundRect(cx-bubble_w/2, cy-bubble_h/2, bubble_w, bubble_h, 18, TFT_WHITE);
        tft.setTextColor(TFT_RED, TFT_WHITE);
    } else {
        tft.fillRoundRect(cx-bubble_w/2, cy-bubble_h/2, bubble_w, bubble_h, 18, TFT_DARKGREY);
        tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
    }
    tft.setTextFont(4);
    tft.setTextDatum(CC_DATUM);
    tft.drawString(title, cx, cy);
}

// Only redraw bubbles when selection changes
void drawMenu(TFT_eSPI &tft, int selected, const char* const* titles, int numTitles, int visible) {
    static int last_selected = -1;
    static int last_start = -1;
    static int last_visible = -1;
    static int prev_selected = -1;
    static int prev_start = -1;

    int start = selected - visible/2;
    if (start < 0) start = 0;
    if (start > numTitles - visible) start = numTitles - visible;
    if (numTitles <= visible) start = 0;

    int y0 = 66; // move menu up

    // On first draw, draw everything
    if (last_selected == -1) {
        tft.fillScreen(TFT_BLACK);
        drawMenuTitle(tft);
        for (int i=0; i<visible; i++) {
            int idx = start + i;
            int ypos = y0 + i*60;
            drawMenuItem(tft, titles[idx], ypos, idx == selected);
        }
    } else if (selected != last_selected || start != last_start || visible != last_visible) {
        // Only redraw previously selected and newly selected bubbles
        int prev_idx = last_selected;
        int prev_start_idx = last_start;
        // Redraw the previously selected as unselected
        if (prev_idx >= 0 && prev_idx >= prev_start_idx && prev_idx < prev_start_idx+visible) {
            int ypos = y0 + (prev_idx-prev_start_idx)*60;
            drawMenuItem(tft, titles[prev_idx], ypos, false);
        }
        // Redraw the newly selected as selected
        if (selected >= start && selected < start+visible) {
            int ypos = y0 + (selected-start)*60;
            drawMenuItem(tft, titles[selected], ypos, true);
        }
        // If the visible window scrolled, redraw all
        if (start != last_start || visible != last_visible) {
            tft.fillScreen(TFT_BLACK);
            drawMenuTitle(tft);
            for (int i=0; i<visible; i++) {
                int idx = start + i;
                int ypos = y0 + i*60;
                drawMenuItem(tft, titles[idx], ypos, idx == selected);
            }
        }
    }
    last_selected = selected;
    last_start = start;
    last_visible = visible;
}

void IRAM_ATTR handleRotary() {
    bool A = digitalRead(PIN_TRA);
    bool B = digitalRead(PIN_TRB);
    if (A != lastA) {
        if (A == B) rotaryPos--;   // Turning right (CW) increases, left (CCW) decreases
        else rotaryPos++;
        lastA = A;
    }
}

void setup() {
    Serial.begin(115200);

    pinMode(PIN_TRA, INPUT_PULLUP);
    pinMode(PIN_TRB, INPUT_PULLUP);
    pinMode(PIN_KO, INPUT_PULLUP);

    //attachInterrupt(digitalPinToInterrupt(PIN_TRA), handleRotary, CHANGE);
    //attachInterrupt(digitalPinToInterrupt(PIN_TRB), handleRotary, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_TRA), handleRotary, CHANGE);

    tft.init();
    tft.setRotation(3); // Landscape

    splashScreenKnob(tft); // Show splash before menu!

    drawMenu(tft, selected_game, game_titles, NUM_GAMES, 3);
}

void loop() {
    static int lastMenuPos = 0;
    static int lastButton = HIGH;

    int menuPos = ((rotaryPos % NUM_GAMES) + NUM_GAMES) % NUM_GAMES;
    if (menuPos != lastMenuPos) {
        selected_game = menuPos;
        drawMenu(tft, selected_game, game_titles, NUM_GAMES, 3);
        lastMenuPos = menuPos;
    }

    int button = digitalRead(PIN_KO);
    if (button == LOW && lastButton == HIGH) {
        tft.fillScreen(TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.drawString("Loading...", 160, 120);

        switch (selected_game) {
            case TEMPEST_GORDO_INDEX:
                run_Tempest_Gordo(tft); break;
            // Add more games here
        }
        drawMenu(tft, selected_game, game_titles, NUM_GAMES, 3);
    }
    lastButton = button;
    delay(20);
}

// --- Game includes ---
#include "Tempest_Gordo.h"