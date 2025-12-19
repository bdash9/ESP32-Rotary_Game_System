EstarDyn 2in TFT ST7789 (240x320) with EC11 Rotary encoder and a button (on one board)


I am connecting a EstarDyn 2in TFT ST7789 (240x320) with EC11 Rotary encoder and a button to an TTGO wrover-E ESP32. The ESP32 has a built in SD card reader. The SD GPIO’s are: cs is 13, MOSI is 15, SCLK is 14 and MISO is 2. The TFT, EC11 and button are all built into separate board. The board has pins labeled GND, VCC, SCL, SDA, RES, DC, CS, BLK (TFT backlight adjustment, default is fully on) , TRA and TRB, KO (Can be used as the menu return button or confirmation button), and PUSH (Key interface of EC11 A, B: Phase A and Phase B of EC11).
How do you suggest I connect these two (GPIO pins)? Then you create a test .ino that prints to the screen using TFT_eSPI, checks a file on the SD card, button and EC11 rotary encoder? 
This has worked for the SD card reader:
const int PIN_SD_CS   = 13;
const int PIN_SD_MOSI = 15;
const int PIN_SD_MISO = 2;
const int PIN_SD_SCLK = 14;
    if (!SD.begin(PIN_SD_CS)) {
        Serial.println("[StarWars] SD.begin FAILED!");
        tft.fillScreen(TFT_RED);
        tft.setTextColor(TFT_WHITE);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("SD CARD ERROR!", tft.width()/2, tft.height()/2, 4);
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
        tft.drawString("Can't open sw1.txt on SD!", 20, 40);
        delay(2000);
        return;
    }
    Serial.println("[StarWars] SD.open /sw1.txt succeeded.");
	

GPIOS:

Rotary and button GPIO’s
TRA      19
TRB      21
PUSH     22  // KO button and not PUSH GPIO
Rotary PUSH is 25

TFT:
SCK	- 18
MOSI	- 23
CS	- 5
DC	- 27
RES	- 4
GND	- GND
VCC	- 3.3v
BLK	- 3.3v

SD:
CS   = 13;
MOSI = 15;
MISO = 2;
SCLK = 14;
