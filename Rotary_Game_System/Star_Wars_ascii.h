#ifndef STAR_WARS_ASCII_H
#define STAR_WARS_ASCII_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SD.h>
#include <SPI.h>

// Screen dimensions (same as Captain Gordo)
#define SCREEN_W 320
#define SCREEN_H 240

// SD Card pins
const int PIN_SD_CS   = 13;
const int PIN_SD_MOSI = 15;
const int PIN_SD_MISO = 2;
const int PIN_SD_SCLK = 14;

// Pin definitions (from your setup)
#define PIN_KO 22          // Fire button - exits
#define PIN_ROTARY_PUSH 25 // Rotary encoder button - pauses

// External rotary position from main file
extern volatile int rotaryPos;

// FIXED: Helper function to skip one complete frame forward - now jumps in chunks
bool skipFrameForward(File &f, char* line) {
    // Jump forward 1500 bytes, then search for next frame
    long currentPos = f.position();
    long newPos = currentPos + 1500;  // Jump ahead
    
    // Make sure we don't go past end of file
    long fileSize = f.size();
    if (newPos >= fileSize) {
        // Near end of file, just read line by line
        newPos = currentPos;
    } else {
        f.seek(newPos);
    }
    
    // Now find the next frame number after this position
    bool foundFrame = false;
    while (f.available() && !foundFrame) {
        int len = f.readBytesUntil('\n', line, 127);
        line[len] = '\0';
        String s = String(line);
        s.trim();
        
        if (s.length() > 0) {
            bool just_number = true;
            for (size_t i = 0; i < s.length(); ++i) {
                if (!isDigit(s[i])) { just_number = false; break; }
            }
            if (just_number) {
                // Found next frame number, seek back so we can read it normally
                int backseek = len + 1;
                f.seek(f.position() - backseek);
                foundFrame = true;
            }
        }
    }
    return foundFrame;
}

// Helper function to skip one complete frame backward
bool skipFrameBackward(File &f, char* line) {
    // Go back and search for previous frame number
    long currentPos = f.position();
    long newPos = currentPos - 2000;
    if (newPos < 0) newPos = 0;
    f.seek(newPos);
    
    // Now find the frame number before our current position
    long lastFramePos = -1;
    while (f.position() < currentPos - 100) {
        int len = f.readBytesUntil('\n', line, 127);
        line[len] = '\0';
        String s = String(line);
        s.trim();
        
        if (s.length() > 0) {
            bool just_number = true;
            for (size_t i = 0; i < s.length(); ++i) {
                if (!isDigit(s[i])) { just_number = false; break; }
            }
            if (just_number) {
                lastFramePos = f.position() - len - 1;
            }
        }
    }
    
    if (lastFramePos >= 0) {
        f.seek(lastFramePos);
        return true;
    } else {
        f.seek(0);
        return false;
    }
}

void run_StarWarsAscii(TFT_eSPI &tft) {
    Serial.begin(115200);
    delay(100);
    Serial.println("[StarWars] Starting...");
    
    // Initialize SD card with custom pins
    SPI.begin(PIN_SD_SCLK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);
    Serial.println("[StarWars] SPI.begin done.");

    if (!SD.begin(PIN_SD_CS)) {
        Serial.println("[StarWars] SD.begin FAILED!");
        tft.fillScreen(TFT_RED);
        tft.setTextColor(TFT_WHITE, TFT_RED);
        tft.setTextFont(2);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("SD CARD ERROR!", SCREEN_W/2, SCREEN_H/2);
        tft.setTextFont(1);
        tft.drawString("Check SD card", SCREEN_W/2, SCREEN_H/2 + 30);
        delay(3000);
        return;
    }
    Serial.println("[StarWars] SD.begin succeeded.");

    File f = SD.open("/sw1.txt", "r");
    if (!f) {
        Serial.println("[StarWars] SD.open /sw1.txt FAILED!");
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.setTextFont(2);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("Can't open sw1.txt", SCREEN_W/2, SCREEN_H/2);
        tft.setTextFont(1);
        tft.drawString("File must be in SD root", SCREEN_W/2, SCREEN_H/2 + 30);
        delay(2000);
        return;
    }
    Serial.println("[StarWars] SD.open /sw1.txt succeeded.");

    // Setup rotary button
    pinMode(PIN_ROTARY_PUSH, INPUT_PULLUP);

    char line[128];
    String frame_lines[24];
    
    const int max_cols = 80;
    const int display_cols = 53;
    const int col_offset = 7;
    const int max_rows = 24;

    tft.fillScreen(TFT_BLACK);
    tft.setTextFont(1);
    int char_w = 6;
    int char_h = 8;
    int total_w = char_w * display_cols;
    int total_h = char_h * max_rows;
    
    int x0 = (SCREEN_W - total_w) / 2;
    int y0 = (SCREEN_H - total_h) / 2 + 20;
    if (y0 < 25) y0 = 25;

    int frame_count = 0;
    bool exitRequested = false;
    bool paused = false;
    int lastRotaryPos = rotaryPos;
    int lastPauseButton = HIGH;
    
    // NEW: Exit button hold timing
    unsigned long exitButtonPressTime = 0;
    bool exitButtonHeld = false;
    
    while (!exitRequested) {
        // Check for exit button (KO button) - HOLD FOR 3 SECONDS
        int exitButton = digitalRead(PIN_KO);
        
        if (exitButton == LOW) {
            if (!exitButtonHeld) {
                // Button just pressed
                exitButtonPressTime = millis();
                exitButtonHeld = true;
                Serial.println("[StarWars] Exit button pressed - hold for 3 sec...");
            } else {
                // Button still held - check duration
                unsigned long holdDuration = millis() - exitButtonPressTime;
                if (holdDuration >= 3000) {
                    Serial.println("[StarWars] Exit button held 3 sec - exiting!");
                    exitRequested = true;
                    break;
                }
            }
        } else {
            // Button released
            if (exitButtonHeld) {
                Serial.println("[StarWars] Exit button released (hold longer to exit)");
            }
            exitButtonHeld = false;
        }

        // Check for pause button (rotary push button)
        int pauseButton = digitalRead(PIN_ROTARY_PUSH);
        if (pauseButton == LOW && lastPauseButton == HIGH) {
            paused = !paused;
            Serial.print("[StarWars] ");
            Serial.println(paused ? "PAUSED" : "RESUMED");
            
            if (paused) {
                tft.fillRect(SCREEN_W/2 - 30, 2, 60, 12, TFT_RED);
                tft.setTextColor(TFT_WHITE, TFT_RED);
                tft.setTextFont(1);
                tft.setTextDatum(MC_DATUM);
                tft.drawString("PAUSED", SCREEN_W/2, 8);
            } else {
                tft.fillRect(SCREEN_W/2 - 30, 2, 60, 12, TFT_BLACK);
            }
            
            while (digitalRead(PIN_ROTARY_PUSH) == LOW) delay(10);
        }
        lastPauseButton = pauseButton;

        // Check rotary for fast forward/rewind
        int rotaryDiff = rotaryPos - lastRotaryPos;
        if (rotaryDiff != 0) {
            bool forward = (rotaryDiff > 0);
            int clicks = abs(rotaryDiff);
            int framesToSkip = clicks * 10;
            
            Serial.print("[StarWars] ");
            Serial.print(forward ? "FF " : "RW ");
            Serial.print(framesToSkip);
            Serial.print(" frames (from position ");
            Serial.print(f.position());
            Serial.println(")");
            
            tft.fillRect(SCREEN_W/2 - 20, SCREEN_H - 12, 40, 10, forward ? TFT_GREEN : TFT_YELLOW);
            tft.setTextColor(TFT_BLACK, forward ? TFT_GREEN : TFT_YELLOW);
            tft.setTextFont(1);
            tft.setTextDatum(MC_DATUM);
            tft.drawString(forward ? "FF" : "RW", SCREEN_W/2, SCREEN_H - 7);
            
            for (int skip = 0; skip < framesToSkip; skip++) {
                if (forward) {
                    if (!skipFrameForward(f, line)) {
                        Serial.println("[StarWars] Can't skip forward - at end");
                        f.seek(0);
                        frame_count = 0;
                        break;
                    }
                } else {
                    if (!skipFrameBackward(f, line)) {
                        Serial.println("[StarWars] Can't skip backward - at start");
                        break;
                    }
                }
            }
            
            Serial.print("[StarWars] Now at position ");
            Serial.println(f.position());
            
            delay(200);
            tft.fillRect(SCREEN_W/2 - 20, SCREEN_H - 12, 40, 10, TFT_BLACK);
            
            lastRotaryPos = rotaryPos;
        }

        if (paused) {
            delay(50);
            continue;
        }

        // --- Read frame delay ---
        int frame_delay = 66;
        while (f.available()) {
            int len = f.readBytesUntil('\n', line, sizeof(line)-1);
            line[len] = '\0';
            String s = String(line);
            s.trim();
            if (s.length() == 0) continue;
            
            bool just_number = true;
            for (size_t i = 0; i < s.length(); ++i) {
                if (!isDigit(s[i])) { just_number = false; break; }
            }
            
            if (just_number && s.length() > 0) {
                int n = s.toInt();
                frame_delay = n * 62;
                if (frame_delay < 16) frame_delay = 16;
                break;
            }
        }

        // --- Read frame lines ---
        int lines_in_frame = 0;
        while (f.available() && lines_in_frame < max_rows) {
            int len = f.readBytesUntil('\n', line, sizeof(line)-1);
            line[len] = '\0';
            String s = String(line);
            s.trim();
            
            bool just_number = true;
            for (size_t i = 0; i < s.length(); ++i) {
                if (!isDigit(s[i])) { just_number = false; break; }
            }
            
            if (just_number && s.length() > 0) {
                int backseek = len + 1;
                f.seek(f.position() - backseek);
                break;
            }
            
            if (s.length() == 0) continue;
            frame_lines[lines_in_frame++] = String(line);
        }
        
        for (int i = lines_in_frame; i < max_rows; i++) {
            frame_lines[i] = "";
        }

        // Draw frame
        int y = y0;
        for (int row = 0; row < max_rows; row++) {
            String ln = frame_lines[row];
            for (int col = 0; col < display_cols; col++) {
                int source_col = col_offset + col;
                char c = (source_col < (int)ln.length()) ? ln[source_col] : ' ';
                tft.drawChar(x0 + col * char_w, y, c, TFT_WHITE, TFT_BLACK, 1);
            }
            y += char_h;
        }

        Serial.print("[StarWars] Frame #");
        Serial.println(frame_count++);

        delay(frame_delay);

        if (!f.available()) {
            Serial.println("[StarWars] End of file, looping.");
            f.seek(0);
            frame_count = 0;
        }
    }
    
    f.close();
    tft.fillScreen(TFT_BLACK);
    Serial.println("[StarWars] Exited.");
}

#endif // STAR_WARS_ASCII_H