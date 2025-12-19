#ifndef RUN_GORDO_RUN_H
#define RUN_GORDO_RUN_H

#include <math.h>
#include <stdlib.h>
#include <Arduino.h>
#include <TFT_eSPI.h>

#define PIN_KO   22
#define PIN_PUSH 25
#define RGR_SCREEN_W 320
#define RGR_SCREEN_H 240
#define RGR_LEFT_MARGIN   40
#define RGR_RIGHT_MARGIN  40
#define RGR_PLAY_MIN_X    (RGR_LEFT_MARGIN + 10)
#define RGR_PLAY_MAX_X    (RGR_SCREEN_W - RGR_RIGHT_MARGIN - 10)
#define RGR_ROW_HEIGHT      24
#define RGR_TREE_CELL_W     10
#define RGR_MAX_TREE_DEPTH  3
#define RGR_NUM_ROWS        (RGR_SCREEN_H / RGR_ROW_HEIGHT + 2)
#define RGR_COLOR_GRASS    TFT_DARKGREEN
#define RGR_COLOR_TREE     TFT_GREEN
#define RGR_COLOR_TRUNK    0x9A60
#define RGR_COLOR_GORDO    TFT_BLACK
#define RGR_COLOR_GORDO_V  TFT_WHITE
#define RGR_COLOR_FOX      TFT_ORANGE
#define RGR_COLOR_FOX_FACE TFT_YELLOW
#define RGR_COLOR_FOX_TAIL TFT_WHITE
#define RGR_COLOR_ROCK     TFT_LIGHTGREY
#define RGR_COLOR_LOG      0x9A60
#define RGR_COLOR_HOLE     TFT_BROWN
#define RGR_COLOR_STREAM   TFT_CYAN
#define RGR_COLOR_ZOOMIES  TFT_ORANGE
#define RGR_MAX_ZOOMIES 3

enum Level {
    LEVEL_FOREST = 1,
    LEVEL_DESERT,
    LEVEL_SNOW,
    LEVEL_WATER,
    LEVEL_MAX = 4
};

extern int currentLevel;

struct RGR_TreeRow { int leftDepth; int rightDepth; };
RGR_TreeRow rgr_rows[RGR_NUM_ROWS];

enum RGR_ObstacleType {
    RGR_OBS_ROCK, RGR_OBS_LOG, RGR_OBS_HOLE, RGR_OBS_FALLEN_TREE_L, RGR_OBS_FALLEN_TREE_R, RGR_OBS_STREAM,
    RGR_OBS_SNAKE, RGR_OBS_SCORPION, RGR_OBS_BUZZARD,
    RGR_OBS_POLAR_BEAR, RGR_OBS_PENGUIN, RGR_OBS_ICICLE,
    RGR_OBS_SHARK, RGR_OBS_BOAT, RGR_OBS_SEAWEED
};
struct RGR_Obstacle {
    bool active;
    RGR_ObstacleType type;
    float x, y, prev_x, prev_y;
    int w, h;
    bool scored_jump;
};
struct RGR_Carrot {
    bool active;
    float x, y, prev_x, prev_y;
    int w, h;
};
#define RGR_MAX_OBSTACLES 6
#define RGR_MAX_CARROTS   3
RGR_Obstacle rgr_obs[RGR_MAX_OBSTACLES];
RGR_Carrot   rgr_carrots[RGR_MAX_CARROTS];
extern volatile int rotaryPos;

uint16_t getGrassColor() {
    switch (currentLevel) {
        case LEVEL_FOREST: return TFT_DARKGREEN;
        case LEVEL_DESERT: return 0xE4D0;
        case LEVEL_SNOW:   return TFT_WHITE;
        case LEVEL_WATER:  return TFT_CYAN;
        default: return TFT_DARKGREEN;
    }
}
uint16_t getLeftTreeColor() {
    switch (currentLevel) {
        case LEVEL_FOREST: return TFT_GREEN;
        case LEVEL_DESERT: return TFT_BROWN;
        case LEVEL_SNOW:   return TFT_CYAN;
        case LEVEL_WATER:  return TFT_DARKGREEN;
        default: return TFT_GREEN;
    }
}
uint16_t getRightTreeColor() {
    switch (currentLevel) {
        case LEVEL_FOREST: return TFT_GREEN;
        case LEVEL_DESERT: return TFT_BROWN;
        case LEVEL_SNOW:   return TFT_BLUE;
        case LEVEL_WATER:  return TFT_DARKGREEN;
        default: return TFT_GREEN;
    }
}
uint16_t getTreeTrunkColor() {
    switch (currentLevel) {
        case LEVEL_FOREST: return 0x9A60;
        case LEVEL_DESERT: return TFT_BROWN;
        case LEVEL_SNOW:   return TFT_BLUE;
        case LEVEL_WATER:  return TFT_DARKGREEN;
        default: return 0x9A60;
    }
}

void rgr_initRows() {
    for (int i = 0; i < RGR_NUM_ROWS; i++) {
        rgr_rows[i].leftDepth  = 1 + (rand() % RGR_MAX_TREE_DEPTH);
        rgr_rows[i].rightDepth = 1 + (rand() % RGR_MAX_TREE_DEPTH);
    }
}
float rgr_treeScroll = 0.0f;
void rgr_scrollTrees(float scrollSpeed) { /* disabled: trees are fixed */ }

void rgr_drawFullBackground(TFT_eSPI &tft) {
    tft.fillScreen(getGrassColor());
    for (int i = 0; i < RGR_NUM_ROWS; i++) {
        int y = i * RGR_ROW_HEIGHT + (int)rgr_treeScroll - RGR_ROW_HEIGHT;
        if (y >= RGR_SCREEN_H || y + RGR_ROW_HEIGHT <= 0) continue;
        if (rgr_rows[i].leftDepth > 0) {
            int treeW = rgr_rows[i].leftDepth * RGR_TREE_CELL_W;
            tft.fillRect(0, y, treeW, RGR_ROW_HEIGHT, getLeftTreeColor());
            tft.fillRect(treeW - 3, y, 3, RGR_ROW_HEIGHT, getTreeTrunkColor());
        }
        if (rgr_rows[i].rightDepth > 0) {
            int treeW = rgr_rows[i].rightDepth * RGR_TREE_CELL_W;
            int x0 = RGR_SCREEN_W - treeW;
            tft.fillRect(x0, y, treeW, RGR_ROW_HEIGHT, getRightTreeColor());
            tft.fillRect(x0, y, 3, RGR_ROW_HEIGHT, getTreeTrunkColor());
        }
    }
}
void rgr_redrawBackgroundArea(TFT_eSPI &tft, int x, int y, int w, int h) {
    w += 8; h += 24;
    int y0 = y - h/2;
    int y1 = y + h/2;
    tft.fillRect(x - w/2, y0, w, h, getGrassColor());
    for (int i = 0; i < RGR_NUM_ROWS; i++) {
        int rowY = i * RGR_ROW_HEIGHT + (int)rgr_treeScroll - RGR_ROW_HEIGHT;
        if (rowY > y1 || rowY + RGR_ROW_HEIGHT < y0) continue;
        if (rgr_rows[i].leftDepth > 0) {
            int treeW = rgr_rows[i].leftDepth * RGR_TREE_CELL_W;
            int tx0 = 0, tx1 = treeW;
            int ty0 = rowY, ty1 = rowY + RGR_ROW_HEIGHT;
            if (tx1 > x - w/2 && tx0 < x + w/2 && ty1 > y0 && ty0 < y1) {
                tft.fillRect(tx0, ty0, treeW, RGR_ROW_HEIGHT, getLeftTreeColor());
                tft.fillRect(treeW - 3, ty0, 3, RGR_ROW_HEIGHT, getTreeTrunkColor());
            }
        }
        if (rgr_rows[i].rightDepth > 0) {
            int treeW = rgr_rows[i].rightDepth * RGR_TREE_CELL_W;
            int tx0 = RGR_SCREEN_W - treeW, tx1 = RGR_SCREEN_W;
            int ty0 = rowY, ty1 = rowY + RGR_ROW_HEIGHT;
            if (tx1 > x - w/2 && tx0 < x + w/2 && ty1 > y0 && ty0 < y1) {
                tft.fillRect(tx0, ty0, treeW, RGR_ROW_HEIGHT, getRightTreeColor());
                tft.fillRect(tx0, ty0, 3, RGR_ROW_HEIGHT, getTreeTrunkColor());
            }
        }
    }
}

void rgr_initObstacles() {
    for (int i = 0; i < RGR_MAX_OBSTACLES; i++) rgr_obs[i].active = false;
    for (int i = 0; i < RGR_MAX_CARROTS;   i++) rgr_carrots[i].active = false;
}
void rgr_spawnObstacle() {
    int maxTreeW = RGR_MAX_TREE_DEPTH * RGR_TREE_CELL_W;
    int leftTreeBand = RGR_PLAY_MIN_X + maxTreeW + 6;
    int rightTreeBand = RGR_PLAY_MAX_X - maxTreeW - 6;
    for (int i = 0; i < RGR_MAX_OBSTACLES; i++) {
        if (!rgr_obs[i].active) {
            RGR_Obstacle &o = rgr_obs[i];
            o.active = true; o.y = -20; o.w = 24; o.h = 20; o.scored_jump = false;
            o.prev_x = o.x; o.prev_y = o.y;
            int type = 0;
            if (currentLevel == 1) type = rand() % 6;
            else if (currentLevel == 2) type = 6 + rand() % 3;
            else if (currentLevel == 3) type = 9 + rand() % 3;
            else if (currentLevel == 4) type = 12 + rand() % 3;
            o.type = (RGR_ObstacleType)type;
            switch (o.type) {
                case RGR_OBS_ROCK: o.w = 24; o.h = 24; o.x = leftTreeBand + rand() % (rightTreeBand - leftTreeBand - o.w); break;
                case RGR_OBS_LOG: o.w = 40; o.h = 10; o.x = leftTreeBand + rand() % (rightTreeBand - leftTreeBand - o.w); break;
                case RGR_OBS_HOLE: o.w = 24; o.h = 16; o.x = leftTreeBand + rand() % (rightTreeBand - leftTreeBand - o.w); break;
                case RGR_OBS_FALLEN_TREE_L: o.w = 28; o.h = 10; o.x = RGR_LEFT_MARGIN + 4; break;
                case RGR_OBS_FALLEN_TREE_R: o.w = 28; o.h = 10; o.x = RGR_SCREEN_W - RGR_RIGHT_MARGIN - o.w - 4; break;
                case RGR_OBS_STREAM: o.h = 14; o.w = 60; o.x = leftTreeBand + rand() % (rightTreeBand - leftTreeBand - o.w); break;
                case RGR_OBS_SNAKE: o.w = 44; o.h = 12; o.x = leftTreeBand + rand() % (rightTreeBand - leftTreeBand - o.w); break;
                case RGR_OBS_SCORPION: o.w = 32; o.h = 16; o.x = leftTreeBand + rand() % (rightTreeBand - leftTreeBand - o.w); break;
                case RGR_OBS_BUZZARD: o.w = 32; o.h = 16; o.x = leftTreeBand + rand() % (rightTreeBand - leftTreeBand - o.w); break;
                case RGR_OBS_POLAR_BEAR: o.w = 32; o.h = 16; o.x = leftTreeBand + rand() % (rightTreeBand - leftTreeBand - o.w); break;
                case RGR_OBS_PENGUIN: o.w = 20; o.h = 16; o.x = leftTreeBand + rand() % (rightTreeBand - leftTreeBand - o.w); break;
                case RGR_OBS_ICICLE: o.w = 12; o.h = 18; o.x = leftTreeBand + rand() % (rightTreeBand - leftTreeBand - o.w); break;
                case RGR_OBS_SHARK: o.w = 48; o.h = 18; o.x = leftTreeBand + rand() % (rightTreeBand - leftTreeBand - o.w); break;
                case RGR_OBS_BOAT: o.w = 36; o.h = 14; o.x = leftTreeBand + rand() % (rightTreeBand - leftTreeBand - o.w); break;
                case RGR_OBS_SEAWEED: o.w = 14; o.h = 24; o.x = leftTreeBand + rand() % (rightTreeBand - leftTreeBand - o.w); break;
            }
            o.prev_x = o.x; o.prev_y = o.y; return;
        }
    }
}
void rgr_spawnCarrot() {
    int maxTreeW = RGR_MAX_TREE_DEPTH * RGR_TREE_CELL_W;
    int leftTreeBand = RGR_PLAY_MIN_X + maxTreeW + 6;
    int rightTreeBand = RGR_PLAY_MAX_X - maxTreeW - 6;
    for (int i = 0; i < RGR_MAX_CARROTS; i++) {
        if (!rgr_carrots[i].active) {
            RGR_Carrot &c = rgr_carrots[i];
            c.active = true; c.y = -20; c.w = 16; c.h = 16;
            c.x = leftTreeBand + rand() % (rightTreeBand - leftTreeBand - c.w);
            c.prev_x = c.x; c.prev_y = c.y; return;
        }
    }
}
void rgr_drawObstacle(TFT_eSPI &tft, const RGR_Obstacle &o) {
    if (!o.active) return;
    switch (o.type) {
        case RGR_OBS_ROCK: tft.fillCircle((int)o.x, (int)o.y, o.w/2, RGR_COLOR_ROCK); break;
        case RGR_OBS_LOG: tft.fillRoundRect((int)o.x - o.w/2, (int)o.y - o.h/2, o.w, o.h, 3, RGR_COLOR_LOG); break;
        case RGR_OBS_HOLE: tft.fillEllipse((int)o.x, (int)o.y, o.w/2, o.h/2, RGR_COLOR_HOLE); break;
        case RGR_OBS_FALLEN_TREE_L: case RGR_OBS_FALLEN_TREE_R:
            tft.fillRect((int)o.x - o.w/2, (int)o.y - o.h/2, o.w, o.h, RGR_COLOR_LOG); break;
        case RGR_OBS_STREAM: tft.fillRect((int)o.x - o.w/2, (int)o.y - o.h/2, o.w, o.h, RGR_COLOR_STREAM); break;
        case RGR_OBS_SNAKE: {
            int snakeLen = o.w;
            int x0 = (int)o.x - snakeLen/2;
            int y0 = (int)o.y;
            tft.fillRect(x0, y0-4, snakeLen, 8, TFT_GREEN);
            for (int k = 0; k < snakeLen; k += 8)
                tft.fillRect(x0 + k, y0-4, 3, 8, TFT_RED);
            tft.fillEllipse(x0 + snakeLen, y0, 8, 6, TFT_GREEN);
            tft.drawLine(x0 + snakeLen + 6, y0, x0 + snakeLen + 14, y0+2, TFT_RED);
            tft.drawLine(x0 + snakeLen + 6, y0, x0 + snakeLen + 14, y0-2, TFT_RED);
            break;
        }
        case RGR_OBS_SCORPION: {
            int sx = (int)o.x, sy = (int)o.y;
            tft.fillCircle(sx, sy, 7, TFT_RED);
            for (int t=0; t<4; t++)
                tft.fillCircle(sx-11-t*3, sy-7+t*4, 3, TFT_RED);
            tft.fillEllipse(sx+10, sy-4, 4, 2, TFT_RED);
            tft.fillEllipse(sx+10, sy+4, 4, 2, TFT_RED);
            for (int l=0; l<3; l++) {
                tft.drawLine(sx-2, sy-6+l*3, sx-8, sy-12+l*4, TFT_BROWN);
                tft.drawLine(sx-2, sy+6-l*3, sx-8, sy+12-l*4, TFT_BROWN);
            }
            break;
        }
        case RGR_OBS_BUZZARD:
            tft.fillCircle((int)o.x, (int)o.y, 12, TFT_BROWN);
            break;
        case RGR_OBS_POLAR_BEAR: {
            tft.fillEllipse((int)o.x, (int)o.y+2, 16, 8, TFT_BLUE);
            tft.fillEllipse((int)o.x+13, (int)o.y-4, 7, 6, TFT_CYAN);
            tft.fillTriangle((int)o.x+10, (int)o.y-10, (int)o.x+12, (int)o.y-18, (int)o.x+14, (int)o.y-8, TFT_BLUE);
            tft.fillTriangle((int)o.x+16, (int)o.y-10, (int)o.x+18, (int)o.y-18, (int)o.x+20, (int)o.y-8, TFT_BLUE);
            tft.fillCircle((int)o.x+13, (int)o.y-4, 4, TFT_CYAN);
            tft.fillCircle((int)o.x+12, (int)o.y-6, 1, TFT_BLACK);
            tft.fillCircle((int)o.x+15, (int)o.y-6, 1, TFT_BLACK);
            tft.fillCircle((int)o.x+13, (int)o.y-2, 1, TFT_BLACK);
            tft.fillEllipse((int)o.x-14, (int)o.y+2, 8, 4, TFT_CYAN);
            break;
        }
        case RGR_OBS_PENGUIN:
            tft.fillEllipse((int)o.x, (int)o.y, 7, 12, TFT_BLACK);
            tft.fillEllipse((int)o.x, (int)o.y+2, 5, 8, TFT_WHITE);
            tft.fillEllipse((int)o.x, (int)o.y-8, 6, 6, TFT_BLACK);
            tft.fillEllipse((int)o.x, (int)o.y-7, 3, 4, TFT_WHITE);
            tft.fillCircle((int)o.x-1, (int)o.y-8, 1, TFT_BLACK);
            tft.fillCircle((int)o.x+1, (int)o.y-8, 1, TFT_BLACK);
            tft.fillTriangle((int)o.x, (int)o.y-4, (int)o.x-1, (int)o.y-2, (int)o.x+1, (int)o.y-2, TFT_YELLOW);
            tft.drawLine((int)o.x-7, (int)o.y+4, (int)o.x-12, (int)o.y+12, TFT_BLACK);
            tft.drawLine((int)o.x+7, (int)o.y+4, (int)o.x+12, (int)o.y+12, TFT_BLACK);
            break;
        case RGR_OBS_ICICLE:
            tft.fillTriangle((int)o.x, (int)o.y+o.h/2, (int)o.x-5, (int)o.y-o.h/2, (int)o.x+5, (int)o.y-o.h/2, TFT_CYAN);
            break;
        case RGR_OBS_SHARK: { // ALLIGATOR!
            uint16_t gatorGreen = TFT_GREEN;
            int len = o.w;
            int x0 = (int)o.x - len/2;
            int y0 = (int)o.y;
            tft.fillRect(x0, y0-7, len, 14, gatorGreen);
            tft.fillTriangle(x0-16, y0, x0, y0-10, x0, y0+10, gatorGreen);
            tft.fillRect(x0+10, y0+7, 8, 6, gatorGreen);
            tft.fillRect(x0+30, y0+7, 8, 6, gatorGreen);
            tft.fillRect(x0+10, y0-13, 8, 6, gatorGreen);
            tft.fillRect(x0+30, y0-13, 8, 6, gatorGreen);
            tft.fillEllipse(x0+len+12, y0, 16, 12, gatorGreen);
            tft.fillTriangle(x0+len+8, y0+4, x0+len+24, y0+12, x0+len+8, y0+12, TFT_WHITE);
            for (int t=0; t<5; t++)
                tft.fillTriangle(x0+len+8+2*t, y0+10, x0+len+10+2*t, y0+12, x0+len+8+2*t, y0+12, TFT_WHITE);
            tft.fillCircle(x0+len+16, y0-4, 2, TFT_BLACK);
            break;
        }
        case RGR_OBS_BOAT:
            tft.fillRect((int)o.x-14, (int)o.y, 28, 6, TFT_BROWN);
            tft.drawLine((int)o.x, (int)o.y, (int)o.x, (int)o.y-12, TFT_WHITE);
            tft.drawTriangle((int)o.x, (int)o.y-12, (int)o.x+7, (int)o.y-6, (int)o.x, (int)o.y-6, TFT_WHITE);
            break;
        case RGR_OBS_SEAWEED:
            tft.drawLine((int)o.x, (int)o.y+o.h/2, (int)o.x-4, (int)o.y-o.h/2, TFT_DARKGREEN);
            tft.drawLine((int)o.x, (int)o.y+o.h/2, (int)o.x+4, (int)o.y-o.h/2, TFT_DARKGREEN);
            break;
    }
}
void rgr_drawCarrotSprite(TFT_eSPI &tft, int x, int y) {
    int len = 10, w = 6;
    tft.fillTriangle(x, y+len, x - w/2, y, x + w/2, y, TFT_ORANGE);
    tft.drawLine(x, y, x-4, y-4, TFT_GREEN);
    tft.drawLine(x, y, x+4, y-4, TFT_GREEN);
}
void rgr_drawCarrot(TFT_eSPI &tft, const RGR_Carrot &c) {
    if (!c.active) return;
    rgr_drawCarrotSprite(tft, (int)c.x, (int)c.y);
}

void rgr_drawZoomiesMeter(TFT_eSPI &tft, int zoomies, bool flash) {
    int segW = 12, segH = 6, gap  = 3, x0 = RGR_SCREEN_W - 130, y0 = 4;
    for (int i = 0; i < RGR_MAX_ZOOMIES; i++) {
        int x = x0 + i*(segW+gap);
        uint16_t outline = RGR_COLOR_ZOOMIES, fill;
        fill = (i < zoomies) ? (flash ? TFT_WHITE : RGR_COLOR_ZOOMIES) : TFT_BLACK;
        tft.drawRect(x, y0, segW, segH, outline);
        tft.fillRect(x+1, y0+1, segW-2, segH-2, fill);
    }
}

void rgr_drawScore(TFT_eSPI &tft, int score) {
    tft.setTextColor(TFT_ORANGE, getGrassColor());
    tft.setTextFont(4);
    tft.setTextDatum(TL_DATUM);
    char buf[8];
    snprintf(buf, sizeof(buf), "%3d", score);
    tft.fillRect(86, 0, 60, 32, getGrassColor());
    tft.drawString(buf, 110, 5);
}

void rgr_drawGordo(TFT_eSPI &tft, int x, int y) {
    int bodyW = 22, bodyH = 30;
    tft.fillEllipse(x, y, bodyW/2, bodyH/2, RGR_COLOR_GORDO);
    int bandH = bodyH / 2;
    tft.fillEllipse(x, y - bandH/4, bodyW/2 - 2, bandH/2 + 2, RGR_COLOR_GORDO_V);
    tft.fillRect(x - 2, y - bodyH/2, 4, bodyH/3, RGR_COLOR_GORDO_V);
    int earH = 16, earW = 4, earY = y - bodyH/2 - earH/2 + 3;
    tft.fillRoundRect(x - 6, earY, earW, earH, 2, RGR_COLOR_GORDO);
    tft.fillRoundRect(x + 2, earY, earW, earH, 2, RGR_COLOR_GORDO);
    int legH = 6, legW = 8, legY = y + bodyH/2 - legH - 1;
    tft.fillRoundRect(x - (bodyW/2) - 1, legY, legW, legH, 2, RGR_COLOR_GORDO);
    tft.fillRoundRect(x + (bodyW/2) - legW + 1, legY, legW, legH, 2, RGR_COLOR_GORDO);
    int footR = 3;
    tft.fillCircle(x - (bodyW/2) + 1, legY + legH + 2, footR, RGR_COLOR_GORDO_V);
    tft.fillCircle(x + (bodyW/2) - 1, legY + legH + 2, footR, RGR_COLOR_GORDO_V);
}
void rgr_drawFox(TFT_eSPI &tft, int x, int y) {
    int bodyW = 12, bodyH = 20, bodyY = y + 2;
    tft.fillEllipse(x, bodyY, bodyW/2, bodyH/2, RGR_COLOR_FOX);
    int headR = 7, headY = y - 6;
    tft.fillCircle(x, headY, headR, RGR_COLOR_FOX);
    tft.fillTriangle(x, headY - 10, x - 5, headY + 1, x + 5, headY + 1, RGR_COLOR_FOX_FACE);
    tft.fillTriangle(x - 5, headY - 4, x - 2, headY - 11, x - 1, headY - 4, RGR_COLOR_FOX);
    tft.fillTriangle(x + 5, headY - 4, x + 2, headY - 11, x + 1, headY - 4, RGR_COLOR_FOX);
    tft.fillCircle(x, headY - 8, 1, TFT_BLACK);
    int legH = 5, legW = 4, legY = bodyY + bodyH/2 - legH - 1;
    tft.fillRect(x - bodyW/2 - 1, legY, legW, legH, RGR_COLOR_FOX);
    tft.fillRect(x + bodyW/2 - legW + 1, legY, legW, legH, RGR_COLOR_FOX);
    int tailY = bodyY + bodyH/2;
    tft.fillTriangle(x - 3, tailY, x + 3, tailY, x, tailY + 16, RGR_COLOR_FOX);
    tft.fillTriangle(x - 2, tailY + 8, x + 2, tailY + 8, x, tailY + 16, RGR_COLOR_FOX_TAIL);
}

void rgr_drawSplashGordoLaying(TFT_eSPI &tft, int x, int y) {
    uint16_t gordoBody = TFT_DARKGREY;
    uint16_t gordoWhite = TFT_WHITE;
    uint16_t gordoNose = TFT_PINK;
    tft.fillEllipse(x, y, 28, 16, gordoBody);
    tft.fillEllipse(x-6, y, 13, 13, gordoWhite);
    tft.fillEllipse(x-26, y-6, 10, 8, gordoBody);
    tft.fillRect(x-32, y-10, 6, 14, gordoWhite);
    tft.fillCircle(x-38, y-2, 1, gordoNose);
    tft.fillRoundRect(x-30, y-29, 5, 24, 2, gordoBody);
    tft.fillRoundRect(x-23, y-25, 3, 18, 2, gordoBody);
    tft.fillEllipse(x+18, y+10, 7, 5, gordoWhite);
    tft.fillEllipse(x-8, y+12, 10, 4, gordoWhite);
    tft.fillCircle(x-13, y+7, 4, gordoWhite);
    tft.fillEllipse(x+12, y+4, 7, 5, gordoWhite);
    tft.fillEllipse(x+28, y+3, 4, 4, gordoBody);
    tft.drawEllipse(x+28, y+3, 4, 4, gordoBody);
    tft.drawEllipse(x+29, y+4, 2, 2, gordoBody);
}

void rgr_drawSplashFoxStanding(TFT_eSPI &tft, int x, int y) {
    x += 8;
    tft.fillEllipse(x, y, 24, 13, TFT_ORANGE);
    tft.fillEllipse(x-28, y-10, 10, 8, TFT_ORANGE);
    tft.fillTriangle(x-46, y-9, x-24, y-5, x-24, y-13, TFT_YELLOW);
    tft.fillCircle(x-46, y-9, 2, TFT_BLACK);
    tft.fillTriangle(x-34, y-14, x-31, y-27, x-29, y-14, TFT_ORANGE);
    tft.fillTriangle(x-23, y-14, x-21, y-25, x-19, y-14, TFT_ORANGE);
    tft.fillCircle(x-31, y-14, 1, TFT_BLACK);
    tft.fillCircle(x-26, y-14, 1, TFT_BLACK);
    tft.fillRect(x-6, y+10, 3, 14, TFT_ORANGE);
    tft.fillRect(x+6, y+10, 3, 14, TFT_ORANGE);
    tft.fillRect(x-12, y+12, 3, 12, TFT_ORANGE);
    tft.fillRect(x+12, y+12, 3, 12, TFT_ORANGE);
    tft.fillTriangle(x+22, y+8, x+40, y, x+20, y+18, TFT_ORANGE);
    tft.fillTriangle(x+33, y+2, x+38, y+3, x+28, y+10, TFT_WHITE);
}

void rgr_splashScreen(TFT_eSPI &tft) {
    static const uint16_t rainbow[7] = {
        TFT_RED, 0xFD20, TFT_YELLOW, TFT_GREEN,
        TFT_CYAN, TFT_BLUE, TFT_MAGENTA
    };
    int font = 4;
    int cx = RGR_SCREEN_W / 2;
    int y1 = 54, y2 = 106, y3 = 158;
    int colorPhase = 0;
    int gordoX = 59;
    int foxX = RGR_SCREEN_W-56, foxY = RGR_SCREEN_H/2+38, gordoY = RGR_SCREEN_H/2+38;
    tft.fillScreen(TFT_BLACK);
    while (true) {
        tft.fillScreen(TFT_BLACK);
        rgr_drawSplashGordoLaying(tft, gordoX, gordoY);
        rgr_drawSplashFoxStanding(tft, foxX, foxY);

        tft.setTextDatum(MC_DATUM);
        tft.setTextFont(font);
        tft.setTextSize(2);
        tft.setTextColor(rainbow[(colorPhase+0)%7], TFT_BLACK);
        tft.drawString("Run",    cx, y1);
        tft.setTextColor(rainbow[(colorPhase+2)%7], TFT_BLACK);
        tft.drawString("Gordo,", cx, y2);
        tft.setTextColor(rainbow[(colorPhase+4)%7], TFT_BLACK);
        tft.drawString("RUN!",   cx, y3);
        tft.setTextSize(1);

        tft.setTextFont(2);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.setTextDatum(TC_DATUM);
        tft.drawString("Hit button to start", RGR_SCREEN_W/2, RGR_SCREEN_H - 26);

        colorPhase = (colorPhase + 1) % 7;

        while (digitalRead(PIN_KO) == LOW) delay(10);
        for (int i = 0; i < 40; i++) {
            if (digitalRead(PIN_KO) == LOW) {
                while (digitalRead(PIN_KO) == LOW) delay(10);
                return;
            }
            delay(20);
        }
    }
}

void run_Run_Gordo_Run(TFT_eSPI &tft) {
    tft.setRotation(3);
    pinMode(PIN_KO,   INPUT_PULLUP);
    pinMode(PIN_PUSH, INPUT_PULLUP);

    while (true) {
        rgr_splashScreen(tft);
        rgr_initRows(); rgr_initObstacles(); rgr_treeScroll = 0.0f;
        currentLevel = 1;
        unsigned long levelStartTime = millis();
        float gordoX = (RGR_PLAY_MIN_X + RGR_PLAY_MAX_X) / 2;
        float gordoBaseY = RGR_SCREEN_H - (RGR_SCREEN_H / 3), gordoY = gordoBaseY;
        float prevGordoX = gordoX, prevGordoY = gordoY;
        float foxX = gordoX, foxY = RGR_SCREEN_H - 28, prevFoxX = foxX, prevFoxY = foxY;
        float baseScrollSpeed = 1.6f;
        int zoomiesCharges = 0, zoomiesHits = 0, score = 0;

        tft.fillScreen(TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.setTextFont(4);
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.drawString("Forest Level!", RGR_SCREEN_W/2, RGR_SCREEN_H/2);
        delay(1800);
        rgr_drawFullBackground(tft);
        rgr_drawScore(tft, score);

        bool zoomiesFlash = false;
        bool hoppingUp = false; int hopPhaseCounter = 0;
        bool jumping = false; int jumpFrame = 0;
        const int JUMP_DURATION = 55, JUMP_HEIGHT = 28;
        extern volatile int rotaryPos;
        int lastRotary = rotaryPos;
        const int ROTARY_SENSITIVITY = 2;
        int lastJumpButton = HIGH;
        unsigned long lastFrame = millis(), pushDownTime = 0, obstacleTimer = millis(), carrotTimer = millis();
        bool pushWasDown = false; int lastPush = HIGH; bool gameOver = false;
        static unsigned long splashPushStart = 0;

        while (!gameOver) {
            unsigned long now = millis();

            // --- Level transition every 60 seconds ---
            if (now - levelStartTime > 60000) {
                currentLevel++;
                if (currentLevel > LEVEL_MAX) currentLevel = LEVEL_FOREST;
                levelStartTime = now;
                tft.fillScreen(TFT_BLACK);
                tft.setTextDatum(MC_DATUM);
                tft.setTextFont(4);
                tft.setTextColor(TFT_YELLOW, TFT_BLACK);
                if (currentLevel == 1) tft.drawString("Forest Level!", RGR_SCREEN_W/2, RGR_SCREEN_H/2);
                else if (currentLevel == 2) tft.drawString("Desert Level!", RGR_SCREEN_W/2, RGR_SCREEN_H/2);
                else if (currentLevel == 3) tft.drawString("Snow Level!", RGR_SCREEN_W/2, RGR_SCREEN_H/2);
                else if (currentLevel == 4) tft.drawString("Water Level!", RGR_SCREEN_W/2, RGR_SCREEN_H/2);
                delay(1800);
                rgr_drawFullBackground(tft);
                rgr_drawScore(tft, score);
            }

            if (now - lastFrame < 14) { delay(1); continue; }
            lastFrame = now;

            int curRot = rotaryPos, diff = curRot - lastRotary; lastRotary = curRot;
            float newGordoX = gordoX + diff * ROTARY_SENSITIVITY;
            if (newGordoX < RGR_PLAY_MIN_X) newGordoX = RGR_PLAY_MIN_X;
            if (newGordoX > RGR_PLAY_MAX_X) newGordoX = RGR_PLAY_MAX_X;

            int jumpButton = digitalRead(PIN_KO);
            if (jumpButton == LOW && lastJumpButton == HIGH && !jumping) { jumping = true; jumpFrame = 0; }
            lastJumpButton = jumpButton;
            int jumpOffset = 0;
            if (jumping) {
                float t = (float)jumpFrame / (float)JUMP_DURATION;
                float h = 4.0f * t * (1.0f - t);
                jumpOffset = (int)(h * JUMP_HEIGHT);
                jumpFrame++;
                if (jumpFrame >= JUMP_DURATION) { jumping = false; jumpOffset = 0; }
            }

            int push = digitalRead(PIN_PUSH);
            if (push == LOW && lastPush == HIGH) {
                pushDownTime = now;
                pushWasDown  = true;
            } else if (push == HIGH && lastPush == LOW && pushWasDown) {
                unsigned long held = now - pushDownTime;
                pushWasDown = false;
                if (held >= 3000) {
                    tft.fillScreen(TFT_BLACK);
                    return;
                }
                else if (zoomiesCharges > 0) {
                    zoomiesCharges--; foxY += 20;
                    if (foxY > RGR_SCREEN_H - 20) foxY = RGR_SCREEN_H - 20;
                }
            }
            lastPush = push;

            float scrollSpeed = baseScrollSpeed;
            rgr_scrollTrees(scrollSpeed);
            hopPhaseCounter++;
            if (hopPhaseCounter >= 6) { hopPhaseCounter = 0; hoppingUp = !hoppingUp; }
            int hopOffset = hoppingUp ? 4 : 0;
            float newGordoY = gordoBaseY - hopOffset - jumpOffset;
            float newFoxX = foxX;
            float newFoxY = foxY;
            if (foxX < newGordoX) newFoxX += 2; else if (foxX > newGordoX) newFoxX -= 2;

            // --- Spawn obstacles & carrots ---
            if (now - obstacleTimer > 900) { if (rand() % 100 < 60) rgr_spawnObstacle(); obstacleTimer = now; }
            if (now - carrotTimer > 4000) { if (rand() % 100 < 25) rgr_spawnCarrot(); carrotTimer = now; }

            // 1. ERASE all objects at previous positions (background restore)
            for (int i = 0; i < RGR_MAX_OBSTACLES; i++) {
                if (!rgr_obs[i].active) continue;
                int eraseW = rgr_obs[i].w;
                int eraseH = rgr_obs[i].h;
                int eraseX = (int)rgr_obs[i].prev_x;
                int eraseY = (int)rgr_obs[i].prev_y;
                if (rgr_obs[i].type == RGR_OBS_SNAKE) {
                    int snakeLen = rgr_obs[i].w;
                    int headRight = (int)rgr_obs[i].prev_x + snakeLen/2 + 16;
                    int leftEdge = (int)rgr_obs[i].prev_x - snakeLen/2 - 8;
                    eraseW = headRight - leftEdge;
                    eraseX = leftEdge + eraseW/2;
                }
                else if (rgr_obs[i].type == RGR_OBS_SCORPION) {
                    int sx = (int)rgr_obs[i].prev_x;
                    int leftEdge  = sx - 120;
                    int rightEdge = sx + 60;
                    eraseW = rightEdge - leftEdge;
                    eraseX = leftEdge + eraseW/2;
                }
                else if (rgr_obs[i].type == RGR_OBS_POLAR_BEAR) {
                    int leftEdge = (int)rgr_obs[i].prev_x - rgr_obs[i].w/2 - 32;
                    int rightEdge = (int)rgr_obs[i].prev_x + rgr_obs[i].w/2 + 44;
                    eraseW = rightEdge - leftEdge;
                    eraseX = leftEdge + eraseW/2;
                }
                else if (rgr_obs[i].type == RGR_OBS_SHARK) {
                    int leftEdge = (int)rgr_obs[i].prev_x - rgr_obs[i].w/2 - 32;
                    int rightEdge = (int)rgr_obs[i].prev_x + rgr_obs[i].w/2 + 40;
                    eraseW = rightEdge - leftEdge;
                    eraseX = leftEdge + eraseW/2;
                }
                rgr_redrawBackgroundArea(tft, eraseX, eraseY, eraseW, eraseH);
            }
            for (int i = 0; i < RGR_MAX_CARROTS; i++) {
                if (!rgr_carrots[i].active) continue;
                rgr_redrawBackgroundArea(tft, (int)rgr_carrots[i].prev_x, (int)rgr_carrots[i].prev_y, rgr_carrots[i].w, rgr_carrots[i].h);
            }
            rgr_redrawBackgroundArea(tft, (int)prevGordoX, (int)prevGordoY, 30, 38);
            rgr_redrawBackgroundArea(tft, (int)prevFoxX, (int)prevFoxY, 32, 48);

            // 2. MOVE/UPDATE all positions
            // Obstacles
            for (int i = 0; i < RGR_MAX_OBSTACLES; i++) {
                if (!rgr_obs[i].active) continue;
                rgr_obs[i].y += scrollSpeed;
                if (rgr_obs[i].y - rgr_obs[i].h/2 > RGR_SCREEN_H) rgr_obs[i].active = false;
            }
            // Carrots
            for (int i = 0; i < RGR_MAX_CARROTS; i++) {
                if (!rgr_carrots[i].active) continue;
                rgr_carrots[i].y += scrollSpeed;
                if (rgr_carrots[i].y - rgr_carrots[i].h/2 > RGR_SCREEN_H) rgr_carrots[i].active = false;
            }
            gordoX = newGordoX;
            gordoY = newGordoY;
            foxX = newFoxX;
            foxY = newFoxY;

            // 3. SAVE prev_x/prev_y for next frame
            for (int i = 0; i < RGR_MAX_OBSTACLES; i++) {
                if (!rgr_obs[i].active) continue;
                rgr_obs[i].prev_x = rgr_obs[i].x;
                rgr_obs[i].prev_y = rgr_obs[i].y;
            }
            for (int i = 0; i < RGR_MAX_CARROTS; i++) {
                if (!rgr_carrots[i].active) continue;
                rgr_carrots[i].prev_x = rgr_carrots[i].x;
                rgr_carrots[i].prev_y = rgr_carrots[i].y;
            }
            prevGordoX = gordoX; prevGordoY = gordoY;
            prevFoxX = foxX; prevFoxY = foxY;

            // 4. DRAW all objects at new positions
            for (int i = 0; i < RGR_MAX_OBSTACLES; i++)
                if (rgr_obs[i].active) rgr_drawObstacle(tft, rgr_obs[i]);
            for (int i = 0; i < RGR_MAX_CARROTS; i++)
                if (rgr_carrots[i].active) rgr_drawCarrot(tft, rgr_carrots[i]);
            rgr_drawGordo(tft, (int)gordoX, (int)gordoY);
            rgr_drawFox(tft, (int)foxX, (int)foxY);
            zoomiesFlash = (zoomiesCharges == RGR_MAX_ZOOMIES) && ((now / 200) % 2 == 0);
            rgr_drawZoomiesMeter(tft, zoomiesCharges, zoomiesFlash);
            rgr_drawScore(tft, score);

            // --- Collision logic (unchanged) ---
            for (int i = 0; i < RGR_MAX_OBSTACLES; i++) {
                if (!rgr_obs[i].active) continue;
                RGR_Obstacle &o = rgr_obs[i];
                int gx1 = (int)gordoX - 8, gx2 = (int)gordoX + 8;
                int gy1 = (int)gordoY - 8, gy2 = (int)gordoY + 8;
                int ox1 = (int)o.x - o.w/2 + 4;
                int ox2 = (int)o.x + o.w/2 - 4;
                int oy1 = (int)o.y - o.h/2 + 2;
                int oy2 = (int)o.y + o.h/2 - 2;
                bool overlapX = (gx1 <= ox2 && gx2 >= ox1);
                bool overlapY = (gy1 <= oy2 && gy2 >= oy1);

                if (overlapX && overlapY && jumping && !o.scored_jump) {
                    score += 10;
                    o.scored_jump = true;
                    rgr_drawScore(tft, score);
                }
                if (overlapX && overlapY && !jumping) {
                    rgr_redrawBackgroundArea(tft, (int)o.x, (int)o.y, o.w, o.h);
                    o.active = false;
                    zoomiesHits++;
                    foxY -= 14;
                    if (foxY < gordoY + 6 || zoomiesHits >= 3) {
                        tft.fillScreen(TFT_BLACK);
                        tft.setTextDatum(MC_DATUM);
                        tft.setTextColor(TFT_RED, TFT_BLACK);
                        tft.setTextFont(4);
                        tft.drawString("Fox got Gordo!", RGR_SCREEN_W/2, RGR_SCREEN_H/2);
                        delay(2000);
                        gameOver = true;
                        break;
                    }
                }
                if (!(overlapX && overlapY) && o.scored_jump && !jumping) {
                    o.scored_jump = false;
                }
            }

            if (gameOver) break;
            for (int i = 0; i < RGR_MAX_CARROTS; i++) {
                if (!rgr_carrots[i].active) continue;
                RGR_Carrot &c = rgr_carrots[i];
                int gx1 = (int)gordoX - 10, gx2 = (int)gordoX + 10;
                int gy1 = (int)gordoY - 12, gy2 = (int)gordoY + 12;
                int cx1 = (int)c.x - c.w/2, cx2 = (int)c.x + c.w/2;
                int cy1 = (int)c.y - c.h/2, cy2 = (int)c.y + c.h/2;
                bool overlapX = (gx1 <= cx2 && gx2 >= cx1), overlapY = (gy1 <= cy2 && gy2 >= cy1);
                if (overlapX && overlapY) {
                    c.active = false;
                    score += 20;
                    rgr_drawScore(tft, score);
                    if (zoomiesCharges < RGR_MAX_ZOOMIES) zoomiesCharges++;
                }
            }
        }
        delay(2000);
        rgr_splashScreen(tft);
    }
}

#endif // RUN_GORDO_RUN_H