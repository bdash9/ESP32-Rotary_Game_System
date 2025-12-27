/*
 * PAM8302A Audio Amplifier Test
 * ESP32 with DAC output on GPIO26
 * Shutdown control on GPIO32
 *
 * Type numbers in teh serial prompt at menu
 */

// ===== PAM8302A PINS =====
#define AUDIO_OUT_PIN  26  // DAC2 - analog audio output
#define AMP_SD_PIN     32  // Shutdown control

// ===== EXISTING PINS (for reference) =====
// Rotary Encoder
#define TRA         19
#define TRB         21
#define PUSH_KO     22
#define ROTARY_PUSH 25

// TFT
#define TFT_SCK   18
#define TFT_MOSI  23
#define TFT_CS    5
#define TFT_DC    27
#define TFT_RES   4

// SD Card
#define SD_CS     13
#define SD_MOSI   15
#define SD_MISO   2
#define SD_SCLK   14

// ===== AUDIO SETTINGS =====
#define SAMPLE_RATE 8000  // Hz
#define MAX_VOLUME  255   // DAC range: 0-255
#define MID_VOLUME  128   // Center point

// ===== FUNCTION PROTOTYPES =====
void ampOn();
void ampOff();
void playTone(float frequency, int duration_ms, uint8_t volume);
void playSweep(int startFreq, int endFreq, int duration_ms);
void playBeep();
void playStartupSound();
void printMenu();

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=================================");
  Serial.println("PAM8302A Audio Amplifier Test");
  Serial.println("=================================\n");

  // Setup amplifier shutdown pin
  pinMode(AMP_SD_PIN, OUTPUT);
  digitalWrite(AMP_SD_PIN, LOW);  // Start with amp OFF
  Serial.println("✓ Shutdown pin configured (GPIO32)");
  
  // No setup needed for DAC pin - it's hardware controlled
  Serial.println("✓ DAC output ready (GPIO26)");
  
  delay(500);
  
  // Turn on amplifier
  Serial.println("\n--- Enabling Amplifier ---");
  ampOn();
  delay(100);
  
  // Play startup sound
  Serial.println("Playing startup sound...");
  playStartupSound();
  
  Serial.println("\n✓ Setup complete!\n");
  printMenu();
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    
    switch(cmd) {
      case '1':
        Serial.println("\n[TEST 1] Simple beep (1kHz)");
        playBeep();
        break;
        
      case '2':
        Serial.println("\n[TEST 2] Tone sweep (200Hz - 2kHz)");
        playSweep(200, 2000, 3000);
        break;
        
      case '3':
        Serial.println("\n[TEST 3] Low tone (440Hz - A4 note)");
        playTone(440, 1000, 200);
        break;
        
      case '4':
        Serial.println("\n[TEST 4] High tone (880Hz - A5 note)");
        playTone(880, 1000, 200);
        break;
        
      case '5':
        Serial.println("\n[TEST 5] Volume test (increasing)");
        testVolume();
        break;
        
      case '6':
        Serial.println("\n[TEST 6] Musical scale");
        playScale();
        break;
        
      case 'o':
      case 'O':
        Serial.println("\n[CMD] Amplifier ON");
        ampOn();
        break;
        
      case 'f':
      case 'F':
        Serial.println("\n[CMD] Amplifier OFF");
        ampOff();
        break;
        
      case 'm':
      case 'M':
        printMenu();
        break;
        
      default:
        // Ignore newlines and other chars
        break;
    }
  }
}

// ===== AMPLIFIER CONTROL =====

void ampOn() {
  digitalWrite(AMP_SD_PIN, HIGH);
  Serial.println("  ► Amplifier ENABLED");
  delay(10);  // Small delay for amp to stabilize
}

void ampOff() {
  dacWrite(AUDIO_OUT_PIN, 128);  // Center DAC before shutdown
  delay(10);
  digitalWrite(AMP_SD_PIN, LOW);
  Serial.println("  ► Amplifier DISABLED (low power mode)");
}

// ===== AUDIO GENERATION =====

void playTone(float frequency, int duration_ms, uint8_t volume) {
  Serial.printf("  Playing %.0fHz for %dms at volume %d\n", 
                frequency, duration_ms, volume);
  
  unsigned long startTime = millis();
  float period = 1000000.0 / frequency;  // Period in microseconds
  unsigned long lastToggle = micros();
  bool high = true;
  
  while (millis() - startTime < duration_ms) {
    unsigned long currentMicros = micros();
    
    if (currentMicros - lastToggle >= period / 2) {
      if (high) {
        dacWrite(AUDIO_OUT_PIN, 128 + (volume / 2));
      } else {
        dacWrite(AUDIO_OUT_PIN, 128 - (volume / 2));
      }
      high = !high;
      lastToggle = currentMicros;
    }
  }
  
  // Return to center
  dacWrite(AUDIO_OUT_PIN, 128);
  Serial.println("  ✓ Done");
}

void playSweep(int startFreq, int endFreq, int duration_ms) {
  Serial.printf("  Sweeping %dHz → %dHz over %dms\n", 
                startFreq, endFreq, duration_ms);
  
  unsigned long startTime = millis();
  int steps = 100;
  int stepDuration = duration_ms / steps;
  
  for (int i = 0; i < steps; i++) {
    float freq = startFreq + (endFreq - startFreq) * i / (float)steps;
    playTone(freq, stepDuration, 180);
  }
  
  Serial.println("  ✓ Sweep complete");
}

void playBeep() {
  playTone(1000, 200, 200);
}

void playStartupSound() {
  playTone(800, 100, 150);
  delay(50);
  playTone(1200, 150, 180);
}

void testVolume() {
  Serial.println("  Testing volume levels...");
  for (int vol = 50; vol <= 250; vol += 50) {
    Serial.printf("  Volume: %d\n", vol);
    playTone(1000, 300, vol);
    delay(200);
  }
  Serial.println("  ✓ Volume test complete");
}

void playScale() {
  // C major scale
  float notes[] = {261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25};
  String names[] = {"C4", "D4", "E4", "F4", "G4", "A4", "B4", "C5"};
  
  Serial.println("  Playing C major scale:");
  for (int i = 0; i < 8; i++) {
    Serial.printf("    %s (%.2fHz)\n", names[i].c_str(), notes[i]);
    playTone(notes[i], 400, 200);
    delay(100);
  }
  Serial.println("  ✓ Scale complete");
}

// ===== MENU =====

void printMenu() {
  Serial.println("\n╔════════════════════════════════════╗");
  Serial.println("║     PAM8302A Test Menu            ║");
  Serial.println("╠════════════════════════════════════╣");
  Serial.println("║  1 - Simple beep (1kHz)           ║");
  Serial.println("║  2 - Frequency sweep              ║");
  Serial.println("║  3 - Low tone (440Hz)             ║");
  Serial.println("║  4 - High tone (880Hz)            ║");
  Serial.println("║  5 - Volume test                  ║");
  Serial.println("║  6 - Musical scale                ║");
  Serial.println("║                                   ║");
  Serial.println("║  O - Amplifier ON                 ║");
  Serial.println("║  F - Amplifier OFF                ║");
  Serial.println("║  M - Show this menu               ║");
  Serial.println("╚════════════════════════════════════╝\n");
}