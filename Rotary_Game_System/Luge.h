#ifndef LUGE_H
#define LUGE_H

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
#define COLOR_ICE 0xE73F  // Whiter ice
#define COLOR_TUNNEL 0x7BEF  // Light gray ceiling
#define COLOR_WALL 0xBDF7  // Ice white walls
#define COLOR_MOUNTAIN 0xE71C

// Olympic Ring Colors
#define OLYMPIC_BLUE 0x001F
#define OLYMPIC_YELLOW 0xFFE0
#define OLYMPIC_BLACK 0x0000
#define OLYMPIC_GREEN 0x07E0
#define OLYMPIC_RED 0xF800

// Track section types
enum TrackSectionType {
    TUNNEL_STRAIGHT,
    TUNNEL_LEFT,
    TUNNEL_RIGHT,
    OPEN_STRAIGHT,
    OPEN_LEFT,
    OPEN_RIGHT
};

// Track segment
struct TrackSegment {
    TrackSectionType type;
    float banking;  // -1 to 1 (how tilted)
    bool hasCeiling;
};

// Luge sled and rider
struct LugeSled {
    int segmentIndex;
    float segmentProgress;  // 0 to 1 within segment
    float lateralPos;
    float speed;
    float velocity;
    float leanAngle;
    bool crashed;
    unsigned long startTime;
    unsigned long finishTime;
};

LugeSled sled;

const int NUM_SEGMENTS = 150;  // Much longer track!
TrackSegment trackSegments[NUM_SEGMENTS];

// Trees with random positions
struct LugeTree {
    int baseY;
    int xOffset;
    int size;
};
LugeTree leftTrees[8];
LugeTree rightTrees[8];

// Clouds for open sections
struct LugeCloud {
    float x, y, z;
    int size;
};
LugeCloud lugeClouds[5];

void luge_initTrack() {
    // Create SMOOTHER track with tunnels and open sections
    for (int i = 0; i < NUM_SEGMENTS; i++) {
        // First 15 segments: covered tunnel with GENTLE curves
        if (i < 15) {
            trackSegments[i].hasCeiling = true;
            if (i >= 3 && i <= 6) {
                // One gentle left curve
                trackSegments[i].type = TUNNEL_LEFT;
                trackSegments[i].banking = -0.4f;
            } else if (i >= 10 && i <= 13) {
                // One gentle right curve
                trackSegments[i].type = TUNNEL_RIGHT;
                trackSegments[i].banking = 0.4f;
            } else {
                // Mostly straight
                trackSegments[i].type = TUNNEL_STRAIGHT;
                trackSegments[i].banking = 0.0f;
            }
        }
        // Segments 15-30: transition to open, SMOOTH curves
        else if (i < 30) {
            trackSegments[i].hasCeiling = (i < 18);
            if (i >= 18 && i <= 22) {
                // Gentle left
                trackSegments[i].type = OPEN_LEFT;
                trackSegments[i].banking = -0.5f;
            } else if (i >= 25 && i <= 29) {
                // Gentle right
                trackSegments[i].type = OPEN_RIGHT;
                trackSegments[i].banking = 0.5f;
            } else {
                trackSegments[i].type = OPEN_STRAIGHT;
                trackSegments[i].banking = 0.0f;
            }
        }
        // Final segments: SMOOTH S-curves
        else {
            trackSegments[i].hasCeiling = false;
            
            // Create smooth wave pattern
            int phase = (i - 30) % 20;
            if (phase >= 0 && phase <= 7) {
                // Left curve
                trackSegments[i].type = OPEN_LEFT;
                trackSegments[i].banking = -0.6f;
            } else if (phase >= 13 && phase <= 19) {
                // Right curve
                trackSegments[i].type = OPEN_RIGHT;
                trackSegments[i].banking = 0.6f;
            } else {
                // Straight transition
                trackSegments[i].type = OPEN_STRAIGHT;
                trackSegments[i].banking = 0.0f;
            }
        }
    }
    
    // Initialize random trees
    for (int i = 0; i < 8; i++) {
        leftTrees[i].baseY = random(0, 280);
        leftTrees[i].xOffset = random(0, 20);
        leftTrees[i].size = random(18, 32);
        
        rightTrees[i].baseY = random(0, 280);
        rightTrees[i].xOffset = random(0, 15);
        rightTrees[i].size = random(18, 32);
    }
    
    // Initialize clouds
    for (int i = 0; i < 5; i++) {
        lugeClouds[i].x = random(-100, 100);
        lugeClouds[i].y = random(20, 60);
        lugeClouds[i].z = random(50, 200);
        lugeClouds[i].size = random(15, 35);
    }
}

//=============================================================================
// VECTOR FONT FOR "LUGE"
//=============================================================================
struct LugeVecStroke { int x0, y0, x1, y1; };
struct LugeVecLetter { const LugeVecStroke* strokes; int n; };

const LugeVecStroke luge_strokes_L[] = {{0,0,0,12},{0,12,8,12}};
const LugeVecStroke luge_strokes_U[] = {{0,0,0,12},{0,12,8,12},{8,12,8,0}};
const LugeVecStroke luge_strokes_G[] = {{8,0,0,0},{0,0,0,12},{0,12,8,12},{8,12,8,6},{8,6,4,6}};
const LugeVecStroke luge_strokes_E[] = {{8,0,0,0},{0,0,0,12},{0,12,8,12},{0,6,6,6}};

const LugeVecLetter luge_vecFont[] = {
    {luge_strokes_L,2}, {luge_strokes_U,3}, {luge_strokes_G,5}, {luge_strokes_E,4}
};

int luge_getVecFontIdx(char c) {
    switch(c) {
        case 'L': return 0; case 'U': return 1; case 'G': return 2; case 'E': return 3;
        default: return -1;
    }
}

void luge_drawVecLetter(TFT_eSPI &tft, char c, int x, int y, int scale, uint16_t color) {
    int idx = luge_getVecFontIdx(c);
    if(idx < 0) return;
    for (int i = 0; i < luge_vecFont[idx].n; i++) {
        int x0 = x + luge_vecFont[idx].strokes[i].x0 * scale;
        int y0 = y + luge_vecFont[idx].strokes[i].y0 * scale;
        int x1 = x + luge_vecFont[idx].strokes[i].x1 * scale;
        int y1 = y + luge_vecFont[idx].strokes[i].y1 * scale;
        tft.drawLine(x0, y0, x1, y1, color);
        tft.drawLine(x0+1, y0, x1+1, y1, color);
        tft.drawLine(x0, y0+1, x1, y1+1, color);
    }
}

void luge_drawVecText(TFT_eSPI &tft, const char* text, int x, int y, int scale, uint16_t color) {
    int spacing = 10 * scale;
    int cx = x;
    for (int i = 0; text[i]; i++) {
        if (text[i] != ' ') {
            luge_drawVecLetter(tft, text[i], cx, y, scale, color);
        }
        cx += spacing;
    }
}

//=============================================================================
// OLYMPIC RINGS
//=============================================================================
void luge_drawOlympicRing(TFT_eSPI &tft, int cx, int cy, int radius, uint16_t color) {
    for (int i = 0; i < 4; i++) {
        tft.drawCircle(cx, cy, radius + i, color);
    }
}

void luge_drawOlympicRings(TFT_eSPI &tft, int centerX, int centerY, int ringRadius) {
    int spacing = ringRadius * 2 + 8;
    int vertOffset = ringRadius / 2;
    
    luge_drawOlympicRing(tft, centerX - spacing, centerY, ringRadius, OLYMPIC_BLUE);
    luge_drawOlympicRing(tft, centerX, centerY, ringRadius, OLYMPIC_BLACK);
    luge_drawOlympicRing(tft, centerX + spacing, centerY, ringRadius, OLYMPIC_RED);
    luge_drawOlympicRing(tft, centerX - spacing/2, centerY + vertOffset, ringRadius, OLYMPIC_YELLOW);
    luge_drawOlympicRing(tft, centerX + spacing/2, centerY + vertOffset, ringRadius, OLYMPIC_GREEN);
}

void luge_drawMiniOlympicRings(TFT_eSPI &tft, int x, int y) {
    int r = 8;
    int spacing = r * 2 + 3;
    int vOff = r / 2;
    
    luge_drawOlympicRing(tft, x, y, r, OLYMPIC_BLUE);
    luge_drawOlympicRing(tft, x + spacing, y, r, OLYMPIC_BLACK);
    luge_drawOlympicRing(tft, x + spacing * 2, y, r, OLYMPIC_RED);
    luge_drawOlympicRing(tft, x + spacing/2, y + vOff, r, OLYMPIC_YELLOW);
    luge_drawOlympicRing(tft, x + spacing + spacing/2, y + vOff, r, OLYMPIC_GREEN);
}

//=============================================================================
// MEDAL DRAWING
//=============================================================================
void luge_drawMedal(TFT_eSPI &tft, int x, int y, int medalType) {
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
// SPLASH SCREEN - S-shaped track down mountain
//=============================================================================
void luge_drawSplashScreen(TFT_eSPI &tft) {
    tft.fillScreen(COLOR_SKY);
    
    // Draw mountain background
    tft.fillTriangle(SCREEN_W/2, 10, 0, SCREEN_H, SCREEN_W, SCREEN_H, COLOR_MOUNTAIN);
    
    // S-curve track
    const int SPLASH_POINTS = 40;
    int startX = SCREEN_W / 2;
    int startY = 80;
    
    for (int i = 0; i < SPLASH_POINTS - 1; i++) {
        float t = (float)i / (SPLASH_POINTS - 1);
        float angle = t * PI * 3.5f;
        int x1 = startX + (int)(sin(angle) * 60);
        int y1 = startY + (int)(t * (SCREEN_H - 120));
        
        float t2 = (float)(i+1) / (SPLASH_POINTS - 1);
        float angle2 = t2 * PI * 3.5f;
        int x2 = startX + (int)(sin(angle2) * 60);
        int y2 = startY + (int)(t2 * (SCREEN_H - 120));
        
        int trackWidth = (int)(2 + t * t * 14);
        
        if (y1 < SCREEN_H && y2 < SCREEN_H) {
            tft.drawLine(x1 - trackWidth, y1, x2 - trackWidth, y2, COLOR_WALL);
            tft.drawLine(x1 + trackWidth, y1, x2 + trackWidth, y2, COLOR_WALL);
            tft.drawLine(x1, y1, x2, y2, COLOR_ICE);
            
            if (trackWidth > 6) {
                int innerWidth = trackWidth / 2;
                tft.drawLine(x1 - innerWidth, y1, x2 - innerWidth, y2, tft.color565(200, 220, 240));
                tft.drawLine(x1 + innerWidth, y1, x2 + innerWidth, y2, tft.color565(200, 220, 240));
            }
        }
    }
    
    // Title
    const char* title = "LUGE";
    int scale = 5;
    int titleW = strlen(title) * 10 * scale;
    luge_drawVecText(tft, title, (SCREEN_W - titleW) / 2, 50, scale, OLYMPIC_RED);
    
    luge_drawOlympicRings(tft, SCREEN_W / 2, 150, 18);
}

//=============================================================================
// DRAW FULL SLED WITH RIDER - Feet-first view, lying down - NOW ROTATES!
//=============================================================================
void luge_drawSled(TFT_eSPI &tft, int x, int y, float leanAngle, float lateralPos) {
    // Calculate rotation angle - CAN GO PERPENDICULAR (90 degrees)
    float rotationAngle = 0;
    if (abs(lateralPos) > 0.5f) {
        // More extreme rotation - up to 90 degrees (1.57 radians)
        float wallFactor = (abs(lateralPos) - 0.5f) / 3.0f;  // 0 to ~1.0
        if (wallFactor > 1.0f) wallFactor = 1.0f;
        rotationAngle = (lateralPos > 0 ? 1 : -1) * wallFactor * 1.57f;  // Up to 90 degrees
    }
    
    // For simplicity, we'll draw the sled with offset-based rotation
    // (true rotation would require more complex sprite manipulation)
    
    int sledWidth = 18;
    int sledHeight = 30;
    
    // Calculate rotation offsets (simulate tilting)
    int topOffset = (int)(sin(rotationAngle) * sledHeight * 0.5f);
    int bottomOffset = (int)(sin(rotationAngle) * sledHeight * -0.5f);
    
    // Metal runners (at very bottom) - tilted
    int runnerBottomX = x + bottomOffset;
    tft.fillRect(runnerBottomX - sledWidth/2 - 2, y + sledHeight - 2, 2, 5, TFT_DARKGREY);
    tft.fillRect(runnerBottomX + sledWidth/2, y + sledHeight - 2, 2, 5, TFT_DARKGREY);
    tft.drawFastVLine(runnerBottomX - sledWidth/2 - 1, y + sledHeight, 4, TFT_WHITE);
    tft.drawFastVLine(runnerBottomX + sledWidth/2 + 1, y + sledHeight, 4, TFT_WHITE);
    
    // Sled body (yellow) - tilted
    int sledMidOffset = (int)(sin(rotationAngle) * sledHeight * 0.1f);
    int sledBodyX = x + sledMidOffset;
    
    // Draw sled as tilted quadrilateral (approximated with rectangles)
    for (int dy = 0; dy < 15; dy += 2) {
        float t = (float)dy / 15.0f;
        int offsetX = (int)(bottomOffset * (1-t) + sledMidOffset * t);
        tft.fillRect(sledBodyX + offsetX - sledWidth/2, y + 15 + dy, sledWidth, 2, TFT_YELLOW);
    }
    tft.drawRoundRect(sledBodyX - sledWidth/2, y + 15, sledWidth, 15, 2, TFT_RED);
    
    // Rider lying down - ROTATES with sled
    int leanOffset = (int)(leanAngle * 3);
    
    // Legs (blue suit) - at bottom, tilted
    int legsX = runnerBottomX + leanOffset;
    tft.fillRect(legsX - 4, y + 20, 3, 8, TFT_BLUE);
    tft.fillRect(legsX + 1, y + 20, 3, 8, TFT_BLUE);
    
    // Body/torso (red suit) - middle, tilted
    int bodyOffset = (int)(sin(rotationAngle) * sledHeight * 0.0f);
    int bodyX = x + bodyOffset + leanOffset;
    tft.fillRect(bodyX - 5, y + 10, 10, 10, TFT_RED);
    
    // Arms alongside body - tilted
    int armLeftX = bodyX - sledWidth/2 + 1 - (int)(sin(rotationAngle) * 3);
    int armRightX = bodyX + sledWidth/2 - 3 + (int)(sin(rotationAngle) * 3);
    tft.fillRect(armLeftX, y + 12, 2, 6, TFT_RED);
    tft.fillRect(armRightX, y + 12, 2, 6, TFT_RED);
    
    // Helmet (blue) - at top, tilted most
    int headX = x + topOffset + leanOffset;
    tft.fillCircle(headX, y + 5, 4, OLYMPIC_BLUE);
    
    // Visor - tilted
    int visorStartX = headX - 3 + (int)(sin(rotationAngle) * 2);
    tft.fillRect(visorStartX, y + 6, 6, 2, TFT_CYAN);
}

//=============================================================================
// DRAW SIMPLE TRACK MAP - UPDATED for smoother track
//=============================================================================
void luge_drawTrackMap(TFT_eSPI &tft, int x, int y, int w, int h) {
    // Draw border
    tft.drawRect(x - 2, y - 2, w + 4, h + 4, TFT_RED);
    
    // Draw track path as red vector line - START at BOTTOM, FINISH at TOP
    int numPoints = 30;
    int lastX = x + w/2;
    int lastY = y + h;  // Start at BOTTOM
    
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setTextFont(1);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("START", x + w/2, y + h + 8);
    
    for (int i = 1; i <= numPoints; i++) {
        float t = (float)i / numPoints;
        int segIdx = (int)(t * NUM_SEGMENTS);
        
        // Calculate X position - SMOOTHER curves
        int xPos = x + w/2;
        float curve = trackSegments[segIdx].banking * 0.5f;  // Reduced curve amplitude
        xPos += (int)(curve * w * 0.4f);
        
        // Keep within bounds
        if (xPos < x + 5) xPos = x + 5;
        if (xPos > x + w - 5) xPos = x + w - 5;
        
        int yPos = y + h - (int)(t * h);  // Goes UP from bottom
        
        // Draw RED vector line
        tft.drawLine(lastX, lastY, xPos, yPos, TFT_RED);
        
        // Draw track edges (parallel lines)
        int trackW = 3 + (int)(t * 5);
        tft.drawLine(lastX - trackW, lastY, xPos - trackW, yPos, TFT_RED);
        tft.drawLine(lastX + trackW, lastY, xPos + trackW, yPos, TFT_RED);
        
        lastX = xPos;
        lastY = yPos;
    }
    
    tft.drawString("FINISH", lastX, y - 8);
}

//=============================================================================
// DRAW TRACK - OPTIMIZED to reduce flicker
//=============================================================================
void luge_drawTrackPOV(TFT_eSPI &tft, int segmentIdx, float progress, float lateral, float banking) {
    bool hasCeiling = trackSegments[segmentIdx].hasCeiling;
    bool isOutside = (segmentIdx >= 5);
    
    // Get next segment for warnings
    int nextSegIdx = segmentIdx + 1;
    if (nextSegIdx >= NUM_SEGMENTS) nextSegIdx = NUM_SEGMENTS - 1;
    TrackSectionType upcomingTurn = trackSegments[nextSegIdx].type;
    
    // Background - LESS FREQUENT REDRAW to reduce flicker
    static int frameCount = 0;
    frameCount++;
bool fullRedraw = (frameCount % 2 == 0);  // More frequent to clear tree trails
if (fullRedraw) {
    if (isOutside) {
        // Sky - NO GAP below HUD
        tft.fillRect(0, 35, SCREEN_W, 65, tft.color565(135, 200, 255));
        
        // Mountains - TOUCHING GROUND (moved up)
        tft.fillTriangle(50, 100, 100, 40, 150, 100, TFT_WHITE);
        tft.fillTriangle(180, 100, 230, 50, 280, 100, TFT_WHITE);
        
        // Snow ground - don't go below y=160
        tft.fillRect(0, 100, SCREEN_W, 60, TFT_WHITE);
        
        // Clear bottom area (avoid speed box)
        tft.fillRect(60, SCREEN_H - 40, SCREEN_W - 60, 40, tft.color565(135, 200, 255));
    } else {
        // Tunnel - fill most of screen in black, avoid HUD and speed area
        tft.fillRect(0, 35, SCREEN_W, SCREEN_H - 75, TFT_BLACK);
        
        // Clear bottom area except speed box
        tft.fillRect(60, SCREEN_H - 40, SCREEN_W - 60, 40, TFT_BLACK);
    }
}
    
// WOODEN SIDE WALLS (tunnel) - Move subtly with track
    if (!isOutside) {
        uint16_t woodColor = tft.color565(139, 90, 43);
        uint16_t darkWood = tft.color565(101, 67, 33);
        
        // Calculate track edge positions - SUBTLE movement
        int lateralShift = (int)(lateral * 15);  // Small shift
        int centerX = SCREEN_W/2 - lateralShift;
        int trackWidth = 70 + (int)(0.5f * 150);
        int leftEdge = centerX - trackWidth/2;
        int rightEdge = centerX + trackWidth/2;
                
        // Left side wooden wall
        int leftWallWidth = max(0, leftEdge - 5);
        if (leftWallWidth > 0) {
            tft.fillRect(0, 45, leftWallWidth, 60, woodColor);
            // Wood grain lines
            for (int line = 0; line < leftWallWidth / 10; line++) {
                tft.drawFastVLine(line * 10 + 5, 45, 60, darkWood);
            }
        }
        
        // Right side wooden wall
        int rightWallStart = min(SCREEN_W, rightEdge + 5);
        int rightWallWidth = SCREEN_W - rightWallStart;
        if (rightWallWidth > 0 && rightWallStart < SCREEN_W) {
            tft.fillRect(rightWallStart, 45, rightWallWidth, 60, woodColor);
            // Wood grain lines
            for (int line = 0; line < rightWallWidth / 10; line++) {
                int lineX = rightWallStart + line * 10 + 5;
                if (lineX < SCREEN_W) {
                    tft.drawFastVLine(lineX, 45, 60, darkWood);
                }
            }
        }
    }
    
    // Draw track layers - THIS COMES AFTER WOODEN WALLS
    float scrollSpeed = progress * 12.0f;
    
for (int layer = 0; layer < 5; layer++) {  // ADDED one more layer
        float depth = (float)layer / 4.0f;  // CHANGED: divide by 4 instead of 3
        float layerScroll = scrollSpeed + (float)layer * 0.5f;
        
// Track position - extends all the way to sled
        int y = 100 + (int)(depth * 110);  // INCREASED range - goes lower
        
        if (y >= 100 && y < SCREEN_H - 20) {  // CHANGED: Allow track much closer to bottom
            int trackWidth = 70 + (int)(depth * 150);
            int wallThick = 4 + (int)(depth * 6);
            
            // TALLER WALLS for proper halfpipe
            int baseWallHeight = 35 + (int)(depth * 60);
            
            // BANKING
            int leftWallHeight = baseWallHeight;
            int rightWallHeight = baseWallHeight;
            
            if (banking < -0.3f) {
                rightWallHeight = baseWallHeight + (int)(abs(banking) * 25);
            } else if (banking > 0.3f) {
                leftWallHeight = baseWallHeight + (int)(banking * 25);
            }
            
// Track moves subtly with sled - REDUCED shift to keep it on screen
            int lateralShift = (int)(lateral * (10 + depth * 15));  // MUCH smaller multipliers
            int bankShift = (int)(banking * trackWidth * 0.2f);
            int centerX = SCREEN_W/2 - lateralShift + bankShift;
            
            // Wall color
            uint16_t wallColor;
            if (isOutside) {
                wallColor = TFT_WHITE;
            } else {
                uint8_t gray = 130 + (int)(depth * 90);
                wallColor = tft.color565(gray, gray, gray + 20);
            }
            
            // Calculate transition zone
            int flatBottom = (int)(trackWidth * 0.35f);
            int transitionWidth = trackWidth/2 - flatBottom/2;
            
            // LEFT WALL - with light blue contour outline
            for (int h = 0; h < leftWallHeight; h += 3) {
                int wx;
                
                if (h < baseWallHeight * 0.3f) {
                    float t = (float)h / (baseWallHeight * 0.3f);
                    float angle = t * 1.57f;
                    float curve = sin(angle);
                    
                    int curveAmount = (int)(curve * transitionWidth);
                    wx = centerX - flatBottom/2 - curveAmount;
                } else {
                    wx = centerX - trackWidth/2;
                }
                
if (wx >= 0 && wx < SCREEN_W) {
                    tft.fillRect(wx, y - h, wallThick, 3, wallColor);
                    
                    // OUTDOOR: Add dark shadow on INSIDE edge for definition
                    if (isOutside && layer > 1 && h % 6 == 0) {
                        tft.drawFastVLine(wx + wallThick, y - h, 3, tft.color565(50, 100, 180));
                        tft.drawFastVLine(wx + wallThick + 1, y - h, 3, tft.color565(70, 120, 200));
                    }
                }
                
                // VERTICAL EDGE MARKERS (on wall face, not top)
                if (layer > 1 && h >= baseWallHeight * 0.3f && h < leftWallHeight - 5) {
                    if (h % 12 == 0) {  // Periodic vertical markers
                        tft.drawFastVLine(wx + wallThick + 2, y - h, 6, TFT_RED);
                        tft.drawFastVLine(wx + wallThick + 4, y - h, 6, TFT_YELLOW);
                        tft.drawFastVLine(wx + wallThick + 6, y - h, 6, OLYMPIC_BLUE);
                    }
                }
                
                // Flashing warning at top
                if (layer == 0 && h > leftWallHeight - 8) {
                    if ((upcomingTurn == TUNNEL_LEFT || upcomingTurn == OPEN_LEFT) && 
                        ((int)(layerScroll * 4) % 8) < 4) {
                        tft.fillRect(wx, y - h, wallThick, 3, TFT_YELLOW);
                    }
                }
            }
            
            // RIGHT WALL - with light blue contour outline
            for (int h = 0; h < rightWallHeight; h += 3) {
                int wx;
                
                if (h < baseWallHeight * 0.3f) {
                    float t = (float)h / (baseWallHeight * 0.3f);
                    float angle = t * 1.57f;
                    float curve = sin(angle);
                    
                    int curveAmount = (int)(curve * transitionWidth);
                    wx = centerX + flatBottom/2 + curveAmount;
                } else {
                    wx = centerX + trackWidth/2;
                }
                
if (wx >= 0 && wx < SCREEN_W) {
                    tft.fillRect(wx, y - h, wallThick, 3, wallColor);
                    
                    // OUTDOOR: Add dark shadow on INSIDE edge for definition
                    if (isOutside && layer > 1 && h % 6 == 0) {
                        tft.drawFastVLine(wx - 1, y - h, 3, tft.color565(50, 100, 180));
                        tft.drawFastVLine(wx - 2, y - h, 3, tft.color565(70, 120, 200));
                    }
                }
                
                // VERTICAL EDGE MARKERS (on wall face, not top)
                if (layer > 1 && h >= baseWallHeight * 0.3f && h < rightWallHeight - 5) {
                    if (h % 12 == 0) {
                        tft.drawFastVLine(wx - 8, y - h, 6, TFT_RED);
                        tft.drawFastVLine(wx - 6, y - h, 6, TFT_YELLOW);
                        tft.drawFastVLine(wx - 4, y - h, 6, OLYMPIC_BLUE);
                    }
                }
                
                // Flashing warning at top
                if (layer == 0 && h > rightWallHeight - 8) {
                    if ((upcomingTurn == TUNNEL_RIGHT || upcomingTurn == OPEN_RIGHT) && 
                        ((int)(layerScroll * 4) % 8) < 4) {
                        tft.fillRect(wx, y - h, wallThick, 3, TFT_YELLOW);
                    }
                }
            }
            
// FLOOR
            uint16_t iceColor;
            if (isOutside) {
                iceColor = tft.color565(150, 200, 240);  // Darker blue for more contrast with white snow
            } else {
                uint8_t bright = 150 + (int)(depth * 80);
                iceColor = tft.color565(bright - 30, bright + 10, bright + 60);
            }
            
            int floorX = max(0, centerX - trackWidth/2);
            int floorW = min(SCREEN_W - floorX, trackWidth);
            
            if (floorW > 0) {
                tft.fillRect(floorX, y, floorW, 18, iceColor);
                
                // OUTDOOR: Add dark boundary lines for clear track definition
                if (isOutside && depth > 0.5f) {
                    // Dark lines at track edges
                    tft.drawFastVLine(floorX, y, 18, tft.color565(80, 120, 180));
                    tft.drawFastVLine(floorX + 1, y, 18, tft.color565(100, 140, 200));
                    tft.drawFastVLine(floorX + floorW - 2, y, 18, tft.color565(100, 140, 200));
                    tft.drawFastVLine(floorX + floorW - 1, y, 18, tft.color565(80, 120, 180));
                }
            }
            
            
// SCROLLING COLORED BARS - ONLY ON FLAT BOTTOM (not in curved section)
if (depth > 0.4f && (isOutside || layer < 2)) {  // Inside: only layers 0-1 (skip 2, 3 & 4)
                int barSpacing = 20;
                int barScroll = (int)(scrollSpeed * 25) % barSpacing;
                
                int flatStart = centerX - flatBottom/2;
                int flatEnd = centerX + flatBottom/2;
                
                for (int bar = 0; bar < 8; bar++) {
                    int barY = y + (bar * barSpacing) - barScroll;
                    
// Tunnel needs stricter limit to avoid HUD overlap, outdoor can go lower
int maxY = isOutside ? (SCREEN_H - 60) : (SCREEN_H - 120);                    
                    if (barY > y - 5 && barY < y + 20 && barY < maxY) {
                        uint16_t barColor = (bar % 2 == 0) ? OLYMPIC_BLUE : OLYMPIC_GREEN;
                        
                        int barWidth = 8 + (int)(depth * 10);
                        int barThick = 3 + (int)(depth * 2);
                        
                        // Only draw on FLAT portion
                        int barStart = max(flatStart, centerX - barWidth/2);
                        int barEnd = min(flatEnd, centerX + barWidth/2);
                        
                        if (barEnd > barStart) {
                            tft.fillRect(barStart, barY, barEnd - barStart, barThick, barColor);
                        }
                    }
                }
            }
            
            // Directional arrows
            if (layer == 3) {
                int arrowY = y + 9;
                int arrowOffset = ((int)(layerScroll * 25) % 50);
                
                if (arrowOffset < 25) {
                    if (upcomingTurn == TUNNEL_LEFT || upcomingTurn == OPEN_LEFT) {
                        int arrowX = centerX - 18;
                        tft.fillTriangle(arrowX, arrowY, arrowX + 8, arrowY - 5, arrowX + 8, arrowY + 5, OLYMPIC_BLUE);
                    } else if (upcomingTurn == TUNNEL_RIGHT || upcomingTurn == OPEN_RIGHT) {
                        int arrowX = centerX + 18;
                        tft.fillTriangle(arrowX, arrowY, arrowX - 8, arrowY - 5, arrowX - 8, arrowY + 5, OLYMPIC_BLUE);
                    }
                }
// FINISH LINE indicator in final segment only
if (sled.segmentIndex >= NUM_SEGMENTS - 1) {
    int finishY = y + 9;
    for (int i = 0; i < 10; i++) {
        uint16_t color = (i % 2 == 0) ? TFT_BLACK : TFT_WHITE;
        tft.fillRect(centerX - 50 + (i * 10), finishY, 10, 4, color);
    }
}
            }
        }
}




    // SHADOW UNDER TRACK (outdoor only) - helps define track position
    if (isOutside) {
        int lateralShift = (int)(lateral * 15);
        int centerX = SCREEN_W/2 - lateralShift;
        int shadowWidth = 180;
        
        // Soft shadow gradient
        for (int i = 0; i < 3; i++) {
            uint8_t gray = 220 - (i * 20);
            uint16_t shadowColor = tft.color565(gray, gray, gray);
            tft.drawFastHLine(centerX - shadowWidth/2, SCREEN_H - 30 + i, shadowWidth, shadowColor);
        }
    }
    /*
// WOODEN SIDE WALLS (tunnel) - ONLY REDRAW PERIODICALLY to prevent flicker
     if (!isOutside) { 
        uint16_t woodColor = tft.color565(139, 90, 43);
        uint16_t darkWood = tft.color565(101, 67, 33);
        
        // Calculate track top edge positions at highest visible point
        int centerX = SCREEN_W/2 - (int)(lateral * 40);
        int trackWidth = 70 + (int)(0.5f * 150);
        int leftEdge = centerX - trackWidth/2;
        int rightEdge = centerX + trackWidth/2;
        
        // Left side wooden wall - SOLID FILL (no scrolling animation)
        int leftWallWidth = max(0, leftEdge - 5);
        if (leftWallWidth > 0) {
            tft.fillRect(0, 45, leftWallWidth, 60, woodColor);
            // Wood grain lines
            for (int line = 0; line < leftWallWidth / 10; line++) {
                tft.drawFastVLine(line * 10 + 5, 45, 60, darkWood);
            }
        }
        
        // Right side wooden wall - SOLID FILL (no scrolling animation)
        int rightWallStart = min(SCREEN_W, rightEdge + 5);
        int rightWallWidth = SCREEN_W - rightWallStart;
        if (rightWallWidth > 0 && rightWallStart < SCREEN_W) {
            tft.fillRect(rightWallStart, 45, rightWallWidth, 60, woodColor);
            // Wood grain lines
            for (int line = 0; line < rightWallWidth / 10; line++) {
                int lineX = rightWallStart + line * 10 + 5;
                if (lineX < SCREEN_W) {
                    tft.drawFastVLine(lineX, 45, 60, darkWood);
                }
            }
        }
    }
    */
// PINE TREES (outdoor only)
if (isOutside) {
    // CLEAR tree area first to remove trails - FULL HEIGHT
    tft.fillRect(0, 100, 50, SCREEN_H - 145, TFT_WHITE);  // Stop before speed box
    tft.fillRect(SCREEN_W - 50, 100, 50, SCREEN_H - 145, TFT_WHITE);
    
    int treeScroll = (int)(progress * 100) % 280;
        
    for (int i = 0; i < 8; i++) {
        int ty = leftTrees[i].baseY + treeScroll;
        if (ty > 280) ty -= 280;
        
        // Don't draw trees that would overlap speed box
        if (ty > 100 && ty < SCREEN_H - 45) {  // Stop well before speed box
            int treeSize = leftTrees[i].size;
            int treeX = 10 + leftTrees[i].xOffset;  // CHANGED: moved from 25 to 10
            
            // Only draw if tree won't extend into speed area
            if (ty + treeSize < SCREEN_H - 40) {
                tft.fillRect(treeX - 2, ty + treeSize - 5, 4, 8, tft.color565(101, 67, 33));
                
                uint16_t pineGreen = tft.color565(34, 139, 34);
                tft.fillTriangle(treeX, ty, treeX - treeSize/2, ty + treeSize - 5, treeX + treeSize/2, ty + treeSize - 5, pineGreen);
                tft.fillTriangle(treeX, ty + treeSize/4, treeX - treeSize/3, ty + treeSize - 2, treeX + treeSize/3, ty + treeSize - 2, pineGreen);
                tft.fillTriangle(treeX, ty + treeSize/2, treeX - treeSize/5, ty + treeSize, treeX + treeSize/5, ty + treeSize, pineGreen);
            }
        }
    }
    
    for (int i = 0; i < 8; i++) {
        int ty = rightTrees[i].baseY + treeScroll;
        if (ty > 280) ty -= 280;
        
        if (ty > 100 && ty < SCREEN_H - 45) {
            int treeSize = rightTrees[i].size;
            int treeX = SCREEN_W - 10 - leftTrees[i].xOffset;  // CHANGED: moved from -25 to -10
            
            if (ty + treeSize < SCREEN_H - 40) {
                tft.fillRect(treeX - 2, ty + treeSize - 5, 4, 8, tft.color565(101, 67, 33));
                
                uint16_t pineGreen = tft.color565(34, 139, 34);
                tft.fillTriangle(treeX, ty, treeX - treeSize/2, ty + treeSize - 5, treeX + treeSize/2, ty + treeSize - 5, pineGreen);
                tft.fillTriangle(treeX, ty + treeSize/4, treeX - treeSize/3, ty + treeSize - 2, treeX + treeSize/3, ty + treeSize - 2, pineGreen);
                tft.fillTriangle(treeX, ty + treeSize/2, treeX - treeSize/5, ty + treeSize, treeX + treeSize/5, ty + treeSize, pineGreen);
            }
        }
    }
}
    
// YELLOW CEILING LIGHTS (tunnel only) - FOLLOW TRACK
    if (!isOutside && hasCeiling) {
        int lateralShift = (int)(lateral * 15);  // Match track shift
        int lightCenterX = SCREEN_W/2 - lateralShift;
        
        int lightScroll = (int)(scrollSpeed * 15) % 70;
        for (int i = 0; i < 3; i++) {
            int ly = 25 + i * 70 - lightScroll;
            if (ly >= 20 && ly < 90) {  // Limited to upper portion only
                tft.fillRect(lightCenterX - 12, ly, 24, 3, TFT_YELLOW);
            }
        }
    }
}

unsigned long lastFrame = millis();
    int lastRotary = rotaryPos;

//=============================================================================
// MAIN GAME - EXCITING LUGE!
//=============================================================================
void run_Luge(TFT_eSPI &tft) {
    tft.setRotation(3);
    pinMode(PIN_KO, INPUT_PULLUP);
    
    luge_initTrack();
    luge_drawSplashScreen(tft);
    
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
    
// SHOW INSTRUCTIONS - MUST PRESS BUTTON
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextFont(4);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("HOW TO PLAY", SCREEN_W/2, 20);
    
    tft.setTextFont(2);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("TURN KNOB = STEER", 10, 50);
    tft.drawString("Ride up the walls!", 10, 70);
    tft.drawString("Too high = CRASH", 10, 90);
    tft.drawString("BUTTON = BRAKE", 10, 110);
    tft.drawString("(slow for turns)", 10, 130);
    
    // Draw track map on right side
    luge_drawTrackMap(tft, 200, 50, 100, 100);
    
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextFont(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Press button to start", SCREEN_W/2, SCREEN_H - 20);
    
    // WAIT FOR BUTTON PRESS (required) - reuse existing variables
    lastBtn = HIGH;
    buttonPressed = false;
    while (!buttonPressed) {
        updateAudio();
        int btn = digitalRead(PIN_KO);
        if (btn == LOW && lastBtn == HIGH) {
            buttonPressed = true;
        }
        lastBtn = btn;
        delay(50);
    }
    while (digitalRead(PIN_KO) == LOW) delay(10);  // Wait for release
    delay(300);
    
    // Countdown
    for (int countdown = 3; countdown >= 1; countdown--) {
        tft.fillScreen(COLOR_ICE);
        tft.setTextColor(TFT_RED, COLOR_ICE);
        tft.setTextFont(7);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(String(countdown), SCREEN_W/2, SCREEN_H/2);
        playSound("/sounds/beep.wav", false);
        
        unsigned long countStart = millis();
        while (millis() - countStart < 1000) {
            updateAudio();
            delay(10);
        }
    }
    
    tft.fillScreen(COLOR_ICE);
    tft.setTextColor(TFT_GREEN, COLOR_ICE);
    tft.setTextFont(7);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("GO!", SCREEN_W/2, SCREEN_H/2);
    playSound("/sounds/beep_go.wav", false);
    delay(500);
    
    // Initialize
    sled.segmentIndex = 0;
    sled.segmentProgress = 0.0f;
    sled.lateralPos = 0.0f;
    sled.speed = 20.0f;
    sled.velocity = 0.0f;
    sled.leanAngle = 0.0f;
    sled.crashed = false;
    sled.startTime = millis();
    
    unsigned long lastFrame = millis();
    int lastRotary = rotaryPos;
    lastBtn = HIGH;
    bool gameRunning = true;
    
    // Track vertical position (for wall riding)
    float verticalPos = 0.0f;
    
    while (gameRunning) {
        unsigned long now = millis();
        float deltaTime = (now - lastFrame) / 1000.0f;
        if (deltaTime > 0.05f) deltaTime = 0.05f;
        lastFrame = now;
        
        int btn = digitalRead(PIN_KO);
        
        unsigned long elapsed = now - sled.startTime;
        
        // BUTTON BRAKING
        if (btn == LOW) {
            sled.speed -= 20.0f * deltaTime;
            if (sled.speed < 15.0f) sled.speed = 15.0f;
            
            sled.lateralPos *= 0.95f;
            sled.velocity *= 0.85f;
        } else {
            if (elapsed < 5000) {
                sled.speed = 20.0f + (elapsed / 5000.0f) * 40.0f;
            } else {
sled.speed += 10.0f * deltaTime;
                if (sled.speed > 100.0f) sled.speed = 100.0f;  // INCREASED to 100
            }
        }
        
// Rotary steering - REDUCED sensitivity and auto-centering
        int rotDiff = rotaryPos - lastRotary;
        if (rotDiff != 0) {
            sled.leanAngle -= rotDiff * 0.06f;  // REDUCED from 0.08 to 0.06
            if (sled.leanAngle < -1.0f) sled.leanAngle = -1.0f;
            if (sled.leanAngle > 1.0f) sled.leanAngle = 1.0f;
            lastRotary = rotaryPos;
        }
        
// AUTO-CENTER: lean angle naturally returns to 0
        sled.leanAngle *= 0.95f;  // Decay toward center
        
        sled.velocity += sled.leanAngle * 0.12f;
        sled.velocity *= 0.90f;
        sled.lateralPos += sled.velocity * deltaTime * 3.0f;
        
// Soft limit - gentle push back when getting far
        if (sled.lateralPos > 4.0f) {
            sled.lateralPos = 4.0f;
            sled.velocity = -0.3f;  // Gentler push back
        }
        if (sled.lateralPos < -4.0f) {
            sled.lateralPos = -4.0f;
            sled.velocity = 0.3f;  // Gentler push back
        }
        
        
        float banking = trackSegments[sled.segmentIndex].banking;
        sled.lateralPos -= banking * deltaTime * 1.2f;
        
        bool isOutside = (sled.segmentIndex >= 4);
        float progressSpeed = isOutside ? 0.035f : 0.018f;
        sled.segmentProgress += sled.speed * deltaTime * progressSpeed;
        
        if (sled.segmentProgress >= 1.0f) {
            sled.segmentProgress -= 1.0f;
            sled.segmentIndex++;
            if (sled.segmentIndex >= NUM_SEGMENTS) {
                sled.finishTime = now - sled.startTime;
                gameRunning = false;
                break;
            }
        }
        
// Crash only if REALLY far out (very forgiving)
        if (abs(sled.lateralPos) > 4.5f) {
            sled.crashed = true;
            playSound("/sounds/beep.wav", false);
            gameRunning = false;
            break;
        }
        
// DRAW THE TRACK FIRST
        luge_drawTrackPOV(tft, sled.segmentIndex, sled.segmentProgress, sled.lateralPos, banking);
        
        // Draw HUD backgrounds AFTER track - will cover track edges but stay stable
        uint16_t hudBg = isOutside ? tft.color565(135, 200, 255) : TFT_BLACK;
        
// Top HUD bar - full width
tft.fillRect(0, 0, SCREEN_W, 35, hudBg);

// Small box for speed display only (prevents blinking)
tft.fillRect(5, SCREEN_H - 35, 50, 28, hudBg);        
        // Calculate sled position - FOLLOWS TRACK CURVE
        int sledX = SCREEN_W/2 - (int)(sled.lateralPos * 50);

        
        // Calculate Y based on track curve - sled rides ON the surface
        float absLateral = abs(sled.lateralPos);
        int sledY;
        
        // Base Y position (on flat bottom)
        int baseY = SCREEN_H - 55;
        
        if (absLateral < 0.8f) {
            // On flat bottom - no vertical change
            sledY = baseY;
        } else {
            // On curved wall section
            // How far into the curve (0 = start of curve, 1 = fully vertical)
            float curveProgress = (absLateral - 0.8f) / 2.0f;  // 0 to ~1.0
            if (curveProgress > 1.0f) curveProgress = 1.0f;
            
            // Use sine curve to match track geometry (same as track walls)
            float angle = curveProgress * 1.57f;  // 0 to PI/2 (90 degrees)
            float heightOnWall = sin(angle) * 45.0f;  // Rise up to 45 pixels
            
            sledY = baseY - (int)heightOnWall;
        }

// ERASE OLD SLED - use simple black/white to blend with environment
        static int lastSledX = SCREEN_W/2;
        static int lastSledY = SCREEN_H - 50;
        
if (abs(sledX - lastSledX) > 1 || abs(sledY - lastSledY) > 1) {
    // Simple colors that blend with environment
    uint16_t clearColor;
    if (isOutside) {
        clearColor = tft.color565(150, 200, 240);  // Match ice floor color
    } else {
        clearColor = TFT_BLACK;  // Black for tunnel (matches dark background)
    }
// Constrain erase area to not go below speed box area
int eraseY = lastSledY - 8;
int eraseHeight = 45;
if (eraseY + eraseHeight > SCREEN_H - 40) {
    eraseHeight = (SCREEN_H - 40) - eraseY;
}
if (eraseHeight > 0) {
    tft.fillRect(lastSledX - 15, eraseY, 30, eraseHeight, clearColor);
}
}
        
// Draw sled (now with rotation based on wall position)
        luge_drawSled(tft, sledX, sledY, sled.leanAngle, sled.lateralPos);
        
        lastSledX = sledX;
        lastSledY = sledY;
        
// Warning when getting close to crash zone
        if (abs(sled.lateralPos) > 4.0f) {
            tft.fillRect(SCREEN_W/2 - 60, SCREEN_H/2 - 20, 120, 30, TFT_BLACK);
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.setTextFont(4);
            tft.setTextDatum(MC_DATUM);
            tft.drawString("TOO FAR!", SCREEN_W/2, SCREEN_H/2);
        }
        
        // HUD
        luge_drawMiniOlympicRings(tft, 15, 15);
        
// Timer
        int seconds = elapsed / 1000;
        int hundredths = (elapsed % 1000) / 10;
        char timeBuf[16];
        snprintf(timeBuf, sizeof(timeBuf), "%d.%02ds", seconds, hundredths);
        
        // hudBg already declared earlier
        tft.setTextColor(TFT_YELLOW, hudBg);
        tft.setTextFont(4);
        tft.setTextDatum(TR_DATUM);
        tft.drawString(timeBuf, SCREEN_W - 10, 10);
        
        // Speed
char speedBuf[20];
        if (sled.speed > 85.0f) {  // Show danger at 85+ instead of 70+
            snprintf(speedBuf, sizeof(speedBuf), "%.0f!!", sled.speed);
            tft.setTextColor(TFT_RED, hudBg);
        } else {
            snprintf(speedBuf, sizeof(speedBuf), "%.0f", sled.speed);
            tft.setTextColor(TFT_WHITE, hudBg);
        }
        tft.setTextFont(4);
        tft.setTextDatum(TL_DATUM);
        tft.drawString(speedBuf, 10, SCREEN_H - 30);
        
        lastBtn = btn;
        updateAudio();
        delay(16);
    }
    
    // Results
    delay(500);
    tft.fillScreen(TFT_BLACK);
    
    if (sled.crashed) {
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.setTextFont(4);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("CRASHED!", SCREEN_W/2, SCREEN_H/2 - 20);
        tft.setTextFont(2);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString("Too high on the wall!", SCREEN_W/2, SCREEN_H/2 + 10);
        
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextFont(2);
        tft.drawString("Press button to continue", SCREEN_W/2, SCREEN_H - 20);
        
        while (digitalRead(PIN_KO) == HIGH) {
            updateAudio();
            delay(50);
        }
        while (digitalRead(PIN_KO) == LOW) delay(10);
        delay(400);
        
    } else {
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.setTextFont(4);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("FINISH!", SCREEN_W/2, 50);
        
        int seconds = sled.finishTime / 1000;
        int hundredths = (sled.finishTime % 1000) / 10;
        char timeBuf[32];
        snprintf(timeBuf, sizeof(timeBuf), "Time: %d.%02d sec", seconds, hundredths);
        tft.setTextFont(2);
        tft.drawString(timeBuf, SCREEN_W/2, 90);
        
int medal = 0;
        if (seconds < 50) medal = 3;        // Gold - was 30, now 50
        else if (seconds < 65) medal = 2;   // Silver - was 38, now 65
        else if (seconds < 80) medal = 1;   // Bronze - was 45, now 80
        
        if (medal > 0) {
            playSound("/sounds/crowd-cheer-and-applause.wav", true);
            luge_drawMedal(tft, SCREEN_W/2, 150, medal);
        } else {
            tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
            tft.drawString("No Medal", SCREEN_W/2, 150);
        }
        
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextFont(2);
        tft.drawString("Press button to continue", SCREEN_W/2, SCREEN_H - 20);
        
        while (digitalRead(PIN_KO) == HIGH) {
            updateAudio();
            delay(50);
        }
        while (digitalRead(PIN_KO) == LOW) delay(10);
        delay(400);
    }
}

#endif // LUGE_H