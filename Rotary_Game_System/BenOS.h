#ifndef BENOS_H
#define BENOS_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SD.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <time.h>
#include <TJpg_Decoder.h>
//#include <DNSServer.h>  
#include <WiFiUdp.h> 
#include <ESP32Ping.h>

// QR Code library - handle LOW macro conflict
#undef LOW
#undef HIGH
#include "QrCode.hpp"

// Save reference to QR error correction level before redefining LOW
namespace {
    const qrcodegen::QrCode::Ecc& QR_ECC_LOW = qrcodegen::QrCode::Ecc::LOW;
}

// Redefine for Arduino
#define LOW 0x0
#define HIGH 0x1

// Pin definitions from main (these are #defines, not variables)
// PIN_TRA, PIN_TRB, PIN_KO are already defined in main .ino
extern volatile int rotaryPos;

// Audio functions from main
extern void playSound(const char *path, bool stopCurrent);
extern void updateAudio();
extern void stopAudio();

// ========== CONFIG ==========
namespace BenOS_Config {
    const char* OS_VERSION = "BenOS v1.0";
    const char* WIFI_SSID = "INDIGO";  // ← CHANGE THIS
    const char* WIFI_PASS = "redsun1234";  // ← CHANGE THIS
    const char* NTP_SERVER = "pool.ntp.org";
    const long GMT_OFFSET = -5 * 3600;  // EST (adjust for your timezone)
    const int DAYLIGHT_OFFSET = 0;
}
/*
// ========== DNS SERVER GLOBALS ==========
DNSServer* benOS_dnsServer = nullptr;
bool benOS_dnsServerRunning = false;
const byte DNS_PORT = 53;
IPAddress benOS_apIP(192, 168, 4, 1);

struct DNSOverride {
    String hostname;
    IPAddress ip;
};

#define MAX_DNS_OVERRIDES 50
DNSOverride benOS_dnsOverrides[MAX_DNS_OVERRIDES];
int benOS_numDNSOverrides = 0;
*/
// ========== THEMES ==========
struct BenOS_Theme {
    uint16_t bg;
    uint16_t fg;
    const char* name;
};

BenOS_Theme benOS_themes[] = {
    {TFT_BLACK, TFT_GREEN, "matrix"},
    {TFT_BLACK, TFT_CYAN, "cyan"},
    {TFT_BLACK, TFT_WHITE, "classic"},
    {TFT_BLUE, TFT_YELLOW, "blue"},
    {TFT_WHITE, TFT_BLACK, "light"},
    {0x0014, TFT_ORANGE, "dark-orange"},
    {0x2104, TFT_MAGENTA, "purple"},
    {0x6a51, TFT_RED, "red-night"}
};

int benOS_currentTheme = 0;
const int benOS_themeCount = sizeof(benOS_themes) / sizeof(BenOS_Theme);

// ========== COMMAND MENU MEMORY ==========
int benOS_lastSelectedCommand = 0;

// ========== WEB COMMAND QUEUE ==========
#define MAX_WEB_COMMANDS 5
String benOS_webCommandQueue[MAX_WEB_COMMANDS];
int benOS_webCommandCount = 0;

// ========== WEB OUTPUT BUFFER ==========
String benOS_webCommandOutput = "";
bool benOS_webCommandDone = false;

// ========== FORWARD DECLARATIONS (EARLY) ==========
void benOS_processWebServer();
void benOS_playmusic(TFT_eSPI &tft);
void playWithVisualizer(TFT_eSPI &tft, String filepath, String filename);
void benOS_clock(TFT_eSPI &tft);

// ========== DISPLAY BUFFER ==========
#define BENOS_MAX_LINES 20
#define BENOS_LINE_HEIGHT 16
String benOS_displayBuffer[BENOS_MAX_LINES];
int benOS_scrollOffset = 0;
int benOS_totalLines = 0;

// ========== HELPER FUNCTIONS ==========

void benOS_applyTheme(TFT_eSPI &tft) {
    tft.fillScreen(benOS_themes[benOS_currentTheme].bg);
    tft.setTextColor(benOS_themes[benOS_currentTheme].fg, benOS_themes[benOS_currentTheme].bg);
}

void benOS_clearScreen(TFT_eSPI &tft) {
    benOS_applyTheme(tft);
    benOS_scrollOffset = 0;
    benOS_totalLines = 0;
    for (int i = 0; i < BENOS_MAX_LINES; i++) {
        benOS_displayBuffer[i] = "";
    }
}

void benOS_addLine(String line) {
    if (benOS_totalLines < BENOS_MAX_LINES) {
        benOS_displayBuffer[benOS_totalLines++] = line;
    } else {
        // Scroll up
        for (int i = 0; i < BENOS_MAX_LINES - 1; i++) {
            benOS_displayBuffer[i] = benOS_displayBuffer[i + 1];
        }
        benOS_displayBuffer[BENOS_MAX_LINES - 1] = line;
    }
    Serial.println(line);
}

void benOS_addLineWrapped(String line) {
    const int maxChars = 40;  // Max chars per line for font size 2
    
    if (line.length() <= maxChars) {
        benOS_addLine(line);
        return;
    }
    
    // Break into multiple lines
    int start = 0;
    while (start < line.length()) {
        int end = start + maxChars;
        if (end > line.length()) {
            end = line.length();
        }
        benOS_addLine(line.substring(start, end));
        start = end;
    }
}

void benOS_redrawScreen(TFT_eSPI &tft) {
    tft.fillScreen(benOS_themes[benOS_currentTheme].bg);
    tft.setTextColor(benOS_themes[benOS_currentTheme].fg, benOS_themes[benOS_currentTheme].bg);
    tft.setTextFont(2);
    tft.setTextDatum(TL_DATUM);
    
    int y = 2;
    int startLine = max(0, benOS_totalLines - 14);
    
    for (int i = startLine; i < benOS_totalLines; i++) {
        tft.drawString(benOS_displayBuffer[i].substring(0, 40), 2, y);
        y += BENOS_LINE_HEIGHT;
        if (y > 228) break;
    }
}

// ========== NETWORK FUNCTIONS ==========

void benOS_connectWiFi(TFT_eSPI &tft) {
    if (WiFi.status() == WL_CONNECTED) {
        benOS_addLine("Already connected.");
        benOS_redrawScreen(tft);
        return;
    }
    
    benOS_addLine("Connecting to WiFi...");
    benOS_redrawScreen(tft);
    
    WiFi.begin(BenOS_Config::WIFI_SSID, BenOS_Config::WIFI_PASS);
    int attempts = 0;
    
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        benOS_addLine("Connected!");
        benOS_addLine("IP: " + WiFi.localIP().toString());
    } else {
        benOS_addLine("Failed to connect.");
    }
    benOS_redrawScreen(tft);
}

// ========== AUTO-CONNECT WIFI HELPER ==========
bool benOS_ensureWiFi(TFT_eSPI &tft) {
    if (WiFi.status() == WL_CONNECTED) {
        return true;  // Already connected
    }
    
    benOS_clearScreen(tft);
    benOS_addLine("WiFi Required");
    benOS_addLine("Connecting...");
    benOS_redrawScreen(tft);
    
    WiFi.begin(BenOS_Config::WIFI_SSID, BenOS_Config::WIFI_PASS);
    int attempts = 0;
    
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        benOS_addLine("Connected!");
        benOS_addLine("IP: " + WiFi.localIP().toString());
        benOS_redrawScreen(tft);
        delay(1000);
        return true;
    } else {
        benOS_addLine("Connection failed!");
        benOS_redrawScreen(tft);
        delay(2000);
        return false;
    }
}


// ========== DNS FORWARDING SERVER (UDP-based) ==========

WiFiUDP benOS_dnsUdp;
WiFiUDP benOS_forwardUdp;
bool benOS_dnsServerRunning = false;
const int DNS_PORT = 53;
IPAddress benOS_apIP(192, 168, 4, 1);
IPAddress benOS_dnsForwarder(8, 8, 8, 8);  // Google DNS

struct DNSOverride {
    String hostname;
    IPAddress ip;
};

#define MAX_DNS_OVERRIDES 50
DNSOverride benOS_dnsOverrides[MAX_DNS_OVERRIDES];
int benOS_numDNSOverrides = 0;

// DNS packet structure helpers
String benOS_parseDNSQuery(byte* buffer, int length) {
    if (length < 12) return "";
    
    String hostname = "";
    int pos = 12;  // Skip DNS header
    
    while (pos < length && buffer[pos] != 0) {
        int labelLen = buffer[pos];
        if (labelLen == 0 || labelLen > 63) break;  // Invalid label
        
        pos++;
        for (int i = 0; i < labelLen && pos < length; i++) {
            char c = buffer[pos++];
            // Convert to lowercase immediately
            if (c >= 'A' && c <= 'Z') {
                c = c + 32;
            }
            hostname += c;
        }
        
        if (buffer[pos] != 0 && pos < length) {
            hostname += ".";
        }
    }
    
    // Remove any trailing dot
    if (hostname.endsWith(".")) {
        hostname = hostname.substring(0, hostname.length() - 1);
    }
    
    return hostname;
}

void benOS_createDNSResponse(byte* query, int queryLen, byte* response, int &responseLen, IPAddress ip) {
    // Copy query to response
    memcpy(response, query, queryLen);
    
    // Set flags: response, no error
    response[2] = 0x81;
    response[3] = 0x80;
    
    // Set answer count to 1
    response[6] = 0x00;
    response[7] = 0x01;
    
    // Add answer section
    int pos = queryLen;
    
    // Name pointer to question
    response[pos++] = 0xC0;
    response[pos++] = 0x0C;
    
    // Type A
    response[pos++] = 0x00;
    response[pos++] = 0x01;
    
    // Class IN
    response[pos++] = 0x00;
    response[pos++] = 0x01;
    
    // TTL (300 seconds)
    response[pos++] = 0x00;
    response[pos++] = 0x00;
    response[pos++] = 0x01;
    response[pos++] = 0x2C;
    
    // Data length (4 bytes for IPv4)
    response[pos++] = 0x00;
    response[pos++] = 0x04;
    
    // IP address
    response[pos++] = ip[0];
    response[pos++] = ip[1];
    response[pos++] = ip[2];
    response[pos++] = ip[3];
    
    responseLen = pos;
}

void benOS_loadDNSOverrides() {
    benOS_numDNSOverrides = 0;
    
    if (!SD.exists("/dns_records.txt")) {
        Serial.println("No dns_records.txt found");
        return;
    }
    
    File f = SD.open("/dns_records.txt");
    if (!f) {
        Serial.println("Failed to open dns_records.txt");
        return;
    }
    
    Serial.println("=== Loading DNS overrides ===");
    
    while (f.available() && benOS_numDNSOverrides < MAX_DNS_OVERRIDES) {
        String line = f.readStringUntil('\n');
        line.trim();
        
        if (line.length() == 0 || line.startsWith("#")) {
            continue;
        }
        
        int eqPos = line.indexOf('=');
        if (eqPos > 0) {
            String hostname = line.substring(0, eqPos);
            String ipStr = line.substring(eqPos + 1);
            
            hostname.trim();
            ipStr.trim();
            hostname.toLowerCase();
            
            IPAddress ip;
            if (ip.fromString(ipStr)) {
                benOS_dnsOverrides[benOS_numDNSOverrides].hostname = hostname;
                benOS_dnsOverrides[benOS_numDNSOverrides].ip = ip;
                
                Serial.print("  [");
                Serial.print(benOS_numDNSOverrides);
                Serial.print("] '");
                Serial.print(hostname);
                Serial.print("' -> ");
                Serial.println(ip.toString());
                
                benOS_numDNSOverrides++;
            } else {
                Serial.print("  Invalid IP in line: ");
                Serial.println(line);
            }
        }
    }
    
    f.close();
    Serial.print("=== Loaded ");
    Serial.print(benOS_numDNSOverrides);
    Serial.println(" overrides ===");
}

bool benOS_getDNSOverride(String hostname, IPAddress &result) {
    hostname.toLowerCase();
    hostname.trim();
    
    // Remove trailing dot if present
    if (hostname.endsWith(".")) {
        hostname = hostname.substring(0, hostname.length() - 1);
    }
    
    Serial.print("Checking override for: '");
    Serial.print(hostname);
    Serial.println("'");
    
    for (int i = 0; i < benOS_numDNSOverrides; i++) {
        String overrideHost = benOS_dnsOverrides[i].hostname;
        overrideHost.toLowerCase();
        overrideHost.trim();
        
        Serial.print("  Comparing with: '");
        Serial.print(overrideHost);
        Serial.print("' ... ");
        
        if (hostname == overrideHost) {
            result = benOS_dnsOverrides[i].ip;
            Serial.print("MATCH! ");
            Serial.print(hostname);
            Serial.print(" -> ");
            Serial.println(result.toString());
            return true;
        }
        Serial.println("no match");
    }
    
    Serial.println("  No override found, will forward");
    return false;
}

void benOS_processDNSRequest() {
    if (!benOS_dnsServerRunning) return;
    
    int packetSize = benOS_dnsUdp.parsePacket();
    if (packetSize) {
        byte query[512];
        int len = benOS_dnsUdp.read(query, 512);
        
        IPAddress remoteIP = benOS_dnsUdp.remoteIP();
        int remotePort = benOS_dnsUdp.remotePort();
        
        String hostname = benOS_parseDNSQuery(query, len);
        
        Serial.print("DNS Query from ");
        Serial.print(remoteIP.toString());
        Serial.print(": ");
        Serial.println(hostname);
        
        IPAddress resultIP;
        
        // Check overrides first
        if (benOS_getDNSOverride(hostname, resultIP)) {
            // Send override response
            byte response[512];
            int responseLen = 0;
            benOS_createDNSResponse(query, len, response, responseLen, resultIP);
            
            benOS_dnsUdp.beginPacket(remoteIP, remotePort);
            benOS_dnsUdp.write(response, responseLen);
            benOS_dnsUdp.endPacket();
            
            Serial.println("  Sent override response");
        } else {
            // Forward to real DNS server
            Serial.println("  Forwarding to 8.8.8.8");
            
            benOS_forwardUdp.beginPacket(benOS_dnsForwarder, 53);
            benOS_forwardUdp.write(query, len);
            benOS_forwardUdp.endPacket();
            
            // Wait for response (with timeout)
            unsigned long startTime = millis();
            while (millis() - startTime < 2000) {
                int forwardPacketSize = benOS_forwardUdp.parsePacket();
                if (forwardPacketSize) {
                    byte response[512];
                    int responseLen = benOS_forwardUdp.read(response, 512);
                    
                    // Send response back to client
                    benOS_dnsUdp.beginPacket(remoteIP, remotePort);
                    benOS_dnsUdp.write(response, responseLen);
                    benOS_dnsUdp.endPacket();
                    
                    Serial.println("  Forwarded response");
                    break;
                }
                delay(10);
            }
        }
    }
}

void benOS_startDNSServer(TFT_eSPI &tft) {
    if (benOS_dnsServerRunning) {
        benOS_clearScreen(tft);
        benOS_addLine("DNS Server Status");
        benOS_addLine("===================");
        benOS_addLine("Already running!");
        benOS_addLine("");
        benOS_addLine("SSID: BenOS-DNS");
        benOS_addLine("IP: " + benOS_apIP.toString());
        benOS_addLine("Overrides: " + String(benOS_numDNSOverrides));
        benOS_redrawScreen(tft);
        return;
    }
    
    benOS_clearScreen(tft);
    benOS_addLine("Starting DNS Server");
    benOS_addLine("===================");
    benOS_redrawScreen(tft);
    
    // Connect to WiFi for forwarding
    benOS_addLine("Connecting to WiFi...");
    benOS_redrawScreen(tft);
    
    if (WiFi.status() != WL_CONNECTED) {
        WiFi.mode(WIFI_STA);
        WiFi.begin(BenOS_Config::WIFI_SSID, BenOS_Config::WIFI_PASS);
        
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            attempts++;
        }
        
        if (WiFi.status() != WL_CONNECTED) {
            benOS_addLine("WiFi failed!");
            benOS_addLine("Can't forward DNS");
            benOS_redrawScreen(tft);
            delay(3000);
            return;
        }
    }
    
    benOS_addLine("WiFi connected!");
    benOS_redrawScreen(tft);
    delay(1000);
    
    // Load overrides
    benOS_clearScreen(tft);
    benOS_addLine("Loading overrides...");
    benOS_redrawScreen(tft);
    
    benOS_loadDNSOverrides();
    benOS_addLine("Loaded: " + String(benOS_numDNSOverrides));
    benOS_redrawScreen(tft);
    delay(1000);
    
    // Start AP
    benOS_clearScreen(tft);
    benOS_addLine("Starting AP...");
    benOS_redrawScreen(tft);
    
// Start UDP DNS server listening on all interfaces
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAPConfig(benOS_apIP, benOS_apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP("BenOS-DNS", "benOS123");
    
    delay(1000);
    
    // Bind UDP to listen on all interfaces (0.0.0.0)
    Serial.println("Starting DNS UDP server...");
    if (!benOS_dnsUdp.begin(DNS_PORT)) {
        Serial.println("ERROR: Failed to start UDP server on port 53!");
        benOS_addLine("ERROR: UDP bind failed!");
        benOS_addLine("Port 53 may be in use");
        benOS_redrawScreen(tft);
        delay(3000);
        return;
    }
    
    Serial.print("DNS UDP listening on port ");
    Serial.println(DNS_PORT);
    Serial.println("  Station IP: " + WiFi.localIP().toString());
    Serial.println("  AP IP: " + benOS_apIP.toString());
    
    benOS_forwardUdp.begin(5353);  // Local port for forwarding
    
    benOS_dnsServerRunning = true;
    
benOS_clearScreen(tft);
    benOS_addLine("DNS Server Running!");
    benOS_addLine("===================");
    benOS_addLine("");
    benOS_addLine("OPTION 1: Same Network");
    benOS_addLine("(Recommended)");
    benOS_addLine("");
    benOS_addLine("Stay on your WiFi");
    benOS_addLine("Set phone DNS to:");
    benOS_addLine("  >> " + WiFi.localIP().toString() + " <<");
    benOS_addLine("");
    benOS_addLine("========================");
    benOS_addLine("");
    benOS_addLine("OPTION 2: Direct Connect");
    benOS_addLine("Connect to: BenOS-DNS");
    benOS_addLine("Password: benOS123");
    benOS_addLine("Set DNS to: 192.168.4.1");
    benOS_addLine("(Limited - DNS only)");
    benOS_addLine("");
    benOS_addLine("Overrides: " + String(benOS_numDNSOverrides));
    benOS_redrawScreen(tft);
    
Serial.println("\n=== DNS Server Started ===");
    Serial.println("Station IP (use this for DNS): " + WiFi.localIP().toString());
    Serial.println("AP SSID: BenOS-DNS / Password: benOS123");
    Serial.println("AP IP: " + benOS_apIP.toString());
    Serial.println("DNS listening on UDP port 53");
    Serial.println("Overrides loaded: " + String(benOS_numDNSOverrides));
}

void benOS_stopDNSServer(TFT_eSPI &tft) {
    if (!benOS_dnsServerRunning) {
        benOS_clearScreen(tft);
        benOS_addLine("DNS server not running");
        benOS_redrawScreen(tft);
        return;
    }
    
    benOS_clearScreen(tft);
    benOS_addLine("Stopping DNS Server...");
    benOS_redrawScreen(tft);
    
    benOS_dnsUdp.stop();
    benOS_forwardUdp.stop();
    
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    
    benOS_dnsServerRunning = false;
    benOS_numDNSOverrides = 0;
    
    benOS_addLine("DNS server stopped");
    benOS_redrawScreen(tft);
    
    Serial.println("DNS Server stopped");
    delay(1000);
}

void benOS_dnsServerStatus(TFT_eSPI &tft) {
    benOS_clearScreen(tft);
    benOS_addLine("DNS Server Status");
    benOS_addLine("===================");
    benOS_addLine("");
    
    if (benOS_dnsServerRunning) {
        benOS_addLine("Status: RUNNING");
        benOS_addLine("");
        benOS_addLine("AP Info:");
        benOS_addLine("  SSID: BenOS-DNS");
        benOS_addLine("  IP: " + benOS_apIP.toString());
        benOS_addLine("  Clients: " + String(WiFi.softAPgetStationNum()));
        benOS_addLine("");
        benOS_addLine("Overrides: " + String(benOS_numDNSOverrides));
        
        if (benOS_numDNSOverrides > 0) {
            benOS_addLine("");
            benOS_addLine("First 3 overrides:");
            for (int i = 0; i < min(3, benOS_numDNSOverrides); i++) {
                benOS_addLine("  " + benOS_dnsOverrides[i].hostname +
                            " -> " + benOS_dnsOverrides[i].ip.toString());
            }
        }
    } else {
        benOS_addLine("Status: STOPPED");
        benOS_addLine("");
        benOS_addLine("Use 'dnsstart' to start");
    }
    
    benOS_redrawScreen(tft);
}

void benOS_syncTime(TFT_eSPI &tft) {
    // WiFi check removed - handled by caller
    
    benOS_addLine("Syncing time...");
    benOS_redrawScreen(tft);
    
    configTime(BenOS_Config::GMT_OFFSET, BenOS_Config::DAYLIGHT_OFFSET, BenOS_Config::NTP_SERVER);
    
    int attempts = 0;
    while (time(nullptr) < 100000 && attempts < 20) {
        delay(500);
        attempts++;
    }
    
    if (time(nullptr) > 100000) {
        benOS_addLine("Time synced!");
        time_t now = time(nullptr);
        struct tm* t = localtime(&now);
        char buf[40];
        sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
                t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                t->tm_hour, t->tm_min, t->tm_sec);
        benOS_addLine(buf);
    } else {
        benOS_addLine("Time sync failed.");
    }
    benOS_redrawScreen(tft);
}

String benOS_getTime() {
    time_t now = time(nullptr);
    if (now < 100000) return "Time not synced";
    
    struct tm* t = localtime(&now);
    char buf[40];
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);
    return String(buf);
}

// ========== DIG HOSTNAME SELECTION MENU ==========

struct HostnameExample {
    const char* name;
    const char* hostname;
};

HostnameExample benOS_hostnameExamples[] = {
    {"Google", "google.com"},
    {"Yahoo", "yahoo.com"},
    {"GitHub", "github.com"},
    {"Amazon", "amazon.com"},
    {"Wikipedia", "wikipedia.org"},
    {"Reddit", "reddit.com"},
    {"Stack Overflow", "stackoverflow.com"},
    {"Twitter/X", "twitter.com"},
    {"YouTube", "youtube.com"},
    {"Facebook", "facebook.com"},
    {"Netflix", "netflix.com"},
    {"OpenAI", "openai.com"},
    {"ESPN", "espn.com"},
    {"CNN", "cnn.com"},
    {"BBC", "bbc.com"},
    {"Custom hostname...", ""}  // Last option for manual entry
};

const int benOS_numHostnameExamples = sizeof(benOS_hostnameExamples) / sizeof(HostnameExample);

String benOS_hostnameInput(TFT_eSPI &tft) {
    int selected = 0;
    int lastSelected = -1;
    int lastRotary = rotaryPos;
    int lastBtn = HIGH;
    
    while (true) {
        if (selected != lastSelected) {
            tft.fillScreen(benOS_themes[benOS_currentTheme].bg);
            tft.setTextColor(benOS_themes[benOS_currentTheme].fg, benOS_themes[benOS_currentTheme].bg);
            
            // Title
            tft.setTextFont(2);
            tft.setTextDatum(TC_DATUM);
            tft.drawString("Select Hostname", 160, 5);
            
            // Show 9 items at a time
            int startIdx = selected - 4;
            if (startIdx < 0) startIdx = 0;
            if (startIdx > benOS_numHostnameExamples - 9) startIdx = benOS_numHostnameExamples - 9;
            if (benOS_numHostnameExamples < 9) startIdx = 0;
            
            tft.setTextFont(2);
            tft.setTextDatum(CL_DATUM);
            
            for (int i = 0; i < 9 && (startIdx + i) < benOS_numHostnameExamples; i++) {
                int idx = startIdx + i;
                int y = 30 + i * 23;
                
                if (idx == selected) {
                    tft.setTextColor(benOS_themes[benOS_currentTheme].bg, benOS_themes[benOS_currentTheme].fg);
                    tft.fillRect(2, y - 2, 316, 20, benOS_themes[benOS_currentTheme].fg);
                } else {
                    tft.setTextColor(benOS_themes[benOS_currentTheme].fg, benOS_themes[benOS_currentTheme].bg);
                }
                
                // Show name and hostname
                String display = benOS_hostnameExamples[idx].name;
                if (strlen(benOS_hostnameExamples[idx].hostname) > 0) {
                    display += " (" + String(benOS_hostnameExamples[idx].hostname) + ")";
                }
                tft.drawString(display.substring(0, 38), 6, y + 9);
            }
            
            lastSelected = selected;
        }
        
        // Handle rotary
        int rotDiff = rotaryPos - lastRotary;
        if (rotDiff > 2) {
            selected++;
            if (selected >= benOS_numHostnameExamples) selected = benOS_numHostnameExamples - 1;
            lastRotary = rotaryPos;
        } else if (rotDiff < -2) {
            selected--;
            if (selected < 0) selected = 0;
            lastRotary = rotaryPos;
        }
        
        // Handle button
        int btn = digitalRead(PIN_KO);
        if (btn == LOW && lastBtn == HIGH) {
            while (digitalRead(PIN_KO) == LOW) delay(10);
            delay(200);
            
            // If "Custom hostname..." selected, prompt for manual entry
            if (selected == benOS_numHostnameExamples - 1) {
                // Custom hostname entry screen
                tft.fillScreen(benOS_themes[benOS_currentTheme].bg);
                tft.setTextColor(benOS_themes[benOS_currentTheme].fg, benOS_themes[benOS_currentTheme].bg);
                tft.setTextFont(2);
                tft.setTextDatum(TC_DATUM);
                tft.drawString("Enter Custom Hostname", 160, 10);
                
                tft.setTextDatum(TL_DATUM);
                tft.drawString("Use Serial Monitor", 10, 50);
                tft.drawString("Example: example.com", 10, 70);
                tft.drawString("Or press button to cancel", 10, 90);
                
                String input = "";
                unsigned long startTime = millis();
                int lastBtnInner = HIGH;
                
                while (millis() - startTime < 20000) {  // 20 second timeout
                    while (Serial.available()) {
                        char c = Serial.read();
                        if (c == '\n' || c == '\r') {
                            if (input.length() > 0) return input;
                        } else {
                            input += c;
                            tft.fillRect(10, 120, 300, 30, benOS_themes[benOS_currentTheme].bg);
                            tft.drawString(input.substring(0, 38), 10, 120);
                        }
                    }
                    
                    int btnInner = digitalRead(PIN_KO);
                    if (btnInner == LOW && lastBtnInner == HIGH) {
                        return "";  // Cancel
                    }
                    lastBtnInner = btnInner;
                    delay(50);
                }
                
                return "";  // Timeout
            } else {
                // Return selected hostname
                return String(benOS_hostnameExamples[selected].hostname);
            }
        }
        lastBtn = btn;
        
        delay(50);
    }
}

// ========== DIG COMMAND (DNS LOOKUP) WITH PAGINATION ==========

void benOS_dig(TFT_eSPI &tft, String hostname) {
    // WiFi check removed - handled by caller
    
    benOS_clearScreen(tft);
    benOS_addLine("DNS Lookup");
    benOS_addLine("========================");
    benOS_addLine("Query: " + hostname);
    benOS_addLine("");
    benOS_addLine("Resolving...");
    benOS_redrawScreen(tft);
    
    IPAddress ip;
    unsigned long startTime = millis();
    bool success = WiFi.hostByName(hostname.c_str(), ip);
    unsigned long duration = millis() - startTime;
    
    benOS_clearScreen(tft);
    benOS_addLine("DNS Lookup Results");
    benOS_addLine("========================");
    benOS_addLine("Hostname: " + hostname);
    benOS_addLine("");
    
    if (success) {
        benOS_addLine("Status: SUCCESS");
        benOS_addLine("Query time: " + String(duration) + " ms");
        benOS_addLine("");
        benOS_addLine("Answer:");
        benOS_addLine("  " + hostname);
        benOS_addLine("  IN A " + ip.toString());
        benOS_addLine("");
        
        benOS_addLine("IP Address Details:");
        benOS_addLine("  Decimal: " + ip.toString());
        benOS_addLine("  Hex: 0x" + String(ip[0], HEX) + String(ip[1], HEX) + 
                      String(ip[2], HEX) + String(ip[3], HEX));
        
        String ipClass = "Unknown";
        if (ip[0] < 128) ipClass = "A (Public)";
        else if (ip[0] < 192) ipClass = "B (Public)";
        else if (ip[0] < 224) ipClass = "C (Public)";
        else if (ip[0] < 240) ipClass = "D (Multicast)";
        else ipClass = "E (Reserved)";
        
        if ((ip[0] == 10) ||
            (ip[0] == 172 && ip[1] >= 16 && ip[1] <= 31) ||
            (ip[0] == 192 && ip[1] == 168)) {
            ipClass += " [PRIVATE]";
        }
        
        benOS_addLine("  Class: " + ipClass);
        
    } else {
        benOS_addLine("Status: FAILED");
        benOS_addLine("Query time: " + String(duration) + " ms");
        benOS_addLine("");
        benOS_addLine("Error: Host not found");
        benOS_addLine("");
        benOS_addLine("Possible reasons:");
        benOS_addLine("  - Hostname doesn't exist");
        benOS_addLine("  - DNS server unreachable");
        benOS_addLine("  - Network issue");
    }
    
    benOS_addLine("");
    benOS_addLine("========================");
    benOS_redrawScreen(tft);
}

void benOS_addJSONFormatted(String json) {
    // This is now handled by benOS_displayWithPagination
    // Keep this function for backward compatibility but just wrap the content
    const int maxChars = 38;
    int start = 0;
    
    while (start < json.length()) {
        int end = start + maxChars;
        if (end > json.length()) end = json.length();
        
        // Try to break at sensible points
        if (end < json.length()) {
            int lastComma = json.lastIndexOf(',', end);
            int lastSpace = json.lastIndexOf(' ', end);
            int lastBrace = json.lastIndexOf('}', end);
            
            int breakPoint = max(lastComma, max(lastSpace, lastBrace));
            if (breakPoint > start && (end - breakPoint) < 10) {
                end = breakPoint + 1;
            }
        }
        
        String line = json.substring(start, end);
        line.trim();
        if (line.length() > 0) {
            benOS_addLine(line);
        }
        start = end;
    }
}

// ========== PAGINATED DISPLAY FOR LONG CONTENT ==========

void benOS_displayWithPagination(TFT_eSPI &tft, String content) {
    const int maxChars = 38;  // Slightly shorter for readability
    const int linesPerPage = 12;  // Show 12 lines, leave room for "Press button..."
    
    // Clean up JSON formatting
    content.replace("\r", "");
    content.replace("\\n", " ");
    content.replace("\\\"", "'");
    
    // Break content into lines
    String lines[200];  // Max 200 lines
    int totalLines = 0;
    
    int start = 0;
    while (start < content.length() && totalLines < 200) {
        int end = start + maxChars;
        if (end > content.length()) {
            end = content.length();
        }
        
        // Try to break at a space for readability
        if (end < content.length()) {
            int lastSpace = content.lastIndexOf(' ', end);
            if (lastSpace > start && (end - lastSpace) < 15) {
                end = lastSpace + 1;
            }
        }
        
        lines[totalLines++] = content.substring(start, end);
        start = end;
    }
    
    // Display lines with pagination
    int currentLine = 0;
    
    while (currentLine < totalLines) {
        benOS_clearScreen(tft);
        
        // Show lines for this page
        int linesShown = 0;
        while (linesShown < linesPerPage && currentLine < totalLines) {
            benOS_addLine(lines[currentLine]);
            currentLine++;
            linesShown++;
        }
        
        // Show pagination info
        if (currentLine < totalLines) {
            benOS_addLine("");
            benOS_addLine("--- More content ---");
            benOS_addLine("(" + String(totalLines - currentLine) + " lines remaining)");
            benOS_addLine("Press button to continue...");
        } else {
            benOS_addLine("");
            benOS_addLine("--- End ---");
            benOS_addLine("Press button to continue...");
        }
        
        benOS_redrawScreen(tft);
        
        // Wait for button press
        while (digitalRead(PIN_KO) == LOW) delay(10);
        delay(200);
        
        int lastBtn = HIGH;
        while (true) {
            int btn = digitalRead(PIN_KO);
            if (btn == LOW && lastBtn == HIGH) break;
            lastBtn = btn;
            delay(50);
        }
        
        while (digitalRead(PIN_KO) == LOW) delay(10);
        delay(200);
    }
}

void benOS_curl(TFT_eSPI &tft, String url) {
    // WiFi check removed - handled by caller
    
    benOS_addLine("Fetching...");
    benOS_redrawScreen(tft);
    
    HTTPClient http;
    http.begin(url);
    http.setTimeout(10000);
    
    unsigned long startTime = millis();
    int code = http.GET();
    unsigned long duration = millis() - startTime;
    
    if (code > 0) {
        benOS_clearScreen(tft);
        benOS_addLine("HTTP " + String(code) + " (" + String(duration) + "ms)");
        
        if (code == HTTP_CODE_OK) {
            String payload = http.getString();
            int len = payload.length();
            benOS_addLine("Size: " + String(len) + " bytes");
            benOS_addLine("--- Response ---");
            benOS_redrawScreen(tft);
            
            delay(500);
            benOS_displayWithPagination(tft, payload);
            
        } else {
            benOS_addLine("Status: " + String(code));
            benOS_redrawScreen(tft);
        }
    } else {
        benOS_clearScreen(tft);
        benOS_addLine("curl: Connection failed");
        benOS_addLine("Error: " + http.errorToString(code));
        benOS_redrawScreen(tft);
    }
    
    http.end();
}

// ========== FILE FUNCTIONS ==========

void benOS_writeFile(TFT_eSPI &tft, String name, String data) {
    File f = SD.open("/" + name, FILE_WRITE);
    if (!f) {
        benOS_addLine("Error writing file.");
        benOS_redrawScreen(tft);
        return;
    }
    f.print(data);
    f.close();
    benOS_addLine("File written: " + name);
    benOS_redrawScreen(tft);
}

void benOS_readFile(TFT_eSPI &tft, String name) {
    File f = SD.open("/" + name);
    if (!f) {
        benOS_addLine("Error reading file.");
        benOS_redrawScreen(tft);
        return;
    }
    
    benOS_addLine("=== " + name + " ===");
    while (f.available()) {
        benOS_addLine(f.readStringUntil('\n'));
    }
    f.close();
    benOS_redrawScreen(tft);
}

void benOS_deleteFile(TFT_eSPI &tft, String name) {
    if (SD.remove("/" + name)) {
        benOS_addLine("File deleted: " + name);
    } else {
        benOS_addLine("Error deleting file.");
    }
    benOS_redrawScreen(tft);
}

void benOS_listFiles(TFT_eSPI &tft) {
    File root = SD.open("/");
    if (!root) {
        benOS_addLine("Error opening root dir");
        benOS_redrawScreen(tft);
        return;
    }
    
    benOS_addLine("=== Files on SD ===");
    File file = root.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            String name = String(file.name());
            int size = file.size();
            benOS_addLine(name + " (" + String(size) + " bytes)");
        }
        file = root.openNextFile();
    }
    benOS_redrawScreen(tft);
}

// ========== CALCULATOR ==========

int benOS_precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/' || op == '%') return 2;
    return 0;
}

int benOS_operate(int a, int b, char op) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return b ? a / b : 0;
        case '%': return b ? a % b : 0;
    }
    return 0;
}

void benOS_calc(TFT_eSPI &tft, String expression) {
    int values[50];
    char ops[50];
    int vTop = -1, oTop = -1;
    
    for (int i = 0; i < expression.length(); i++) {
        if (expression[i] == ' ') continue;
        
        if (isdigit(expression[i])) {
            int num = 0;
            while (i < expression.length() && isdigit(expression[i])) {
                num = num * 10 + (expression[i] - '0');
                i++;
            }
            i--;
            values[++vTop] = num;
        } else if (expression[i] == '+' || expression[i] == '-' || 
                   expression[i] == '*' || expression[i] == '/' || expression[i] == '%') {
            while (oTop >= 0 && benOS_precedence(ops[oTop]) >= benOS_precedence(expression[i])) {
                int b = values[vTop--];
                int a = values[vTop--];
                char op = ops[oTop--];
                values[++vTop] = benOS_operate(a, b, op);
            }
            ops[++oTop] = expression[i];
        }
    }
    
    while (oTop >= 0) {
        int b = values[vTop--];
        int a = values[vTop--];
        char op = ops[oTop--];
        values[++vTop] = benOS_operate(a, b, op);
    }
    
    benOS_addLine("Result: " + String(values[vTop]));
    benOS_redrawScreen(tft);
}

// ========== JPG DISPLAY WITH AUTO-SCALING ==========

bool benOS_tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap, TFT_eSPI* tft) {
    if (y >= tft->height()) return 0;
    tft->pushImage(x, y, w, h, bitmap);
    return 1;
}

void benOS_showImage(TFT_eSPI &tft, const char* filename) {
    if (!SD.exists(filename)) {
        benOS_addLine("Image not found:");
        benOS_addLine(filename);
        benOS_redrawScreen(tft);
        delay(2000);
        return;
    }
    
    tft.fillScreen(TFT_BLACK);
    
    uint8_t scale = 1;
    
    TJpgDec.setJpgScale(scale);
    TJpgDec.setSwapBytes(true);
    TJpgDec.setCallback([](int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) -> bool {
        extern TFT_eSPI tft;
        return benOS_tft_output(x, y, w, h, bitmap, &tft);
    });
    
    uint16_t imgWidth = 0, imgHeight = 0;
    TJpgDec.getSdJpgSize(&imgWidth, &imgHeight, filename);
    
    if (imgWidth > 320 || imgHeight > 240) {
        if (imgWidth > imgHeight) {
            if (imgWidth > 320 * 4) scale = 8;
            else if (imgWidth > 320 * 2) scale = 4;
            else if (imgWidth > 320) scale = 2;
        } else {
            if (imgHeight > 240 * 4) scale = 8;
            else if (imgHeight > 240 * 2) scale = 4;
            else if (imgHeight > 240) scale = 2;
        }
        TJpgDec.setJpgScale(scale);
        imgWidth /= scale;
        imgHeight /= scale;
    }
    
    int xOffset = (320 - imgWidth) / 2;
    int yOffset = (240 - imgHeight) / 2;
    if (xOffset < 0) xOffset = 0;
    if (yOffset < 0) yOffset = 0;
    
    TJpgDec.drawSdJpg(xOffset, yOffset, filename);
    
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextFont(2);
    tft.setTextDatum(BC_DATUM);
    tft.drawString("Press button to return", 160, 230);
    
    while (digitalRead(PIN_KO) == LOW) delay(10);
    delay(200);
    
    int lastBtn = HIGH;
    while (true) {
        int btn = digitalRead(PIN_KO);
        if (btn == LOW && lastBtn == HIGH) break;
        lastBtn = btn;
        delay(50);
    }
    
    while (digitalRead(PIN_KO) == LOW) delay(10);
    delay(200);
}

// ========== THEME SELECTION WITH ROTARY ==========

void benOS_selectTheme(TFT_eSPI &tft) {
    int selected = benOS_currentTheme;
    int lastSelected = -1;
    int lastRotary = rotaryPos;
    int lastBtn = HIGH;
    
    while (true) {
        if (selected != lastSelected) {
            tft.fillScreen(benOS_themes[benOS_currentTheme].bg);
            tft.setTextColor(benOS_themes[benOS_currentTheme].fg, benOS_themes[benOS_currentTheme].bg);
            
            tft.setTextFont(2);
            tft.setTextDatum(TC_DATUM);
            tft.drawString("Select Theme", 160, 5);
            
            tft.setTextFont(2);
            tft.setTextDatum(CL_DATUM);
            
            for (int i = 0; i < benOS_themeCount; i++) {
                int y = 30 + i * 25;
                
                if (i == selected) {
                    tft.setTextColor(benOS_themes[i].bg, benOS_themes[i].fg);
                    tft.fillRect(5, y - 2, 310, 22, benOS_themes[i].fg);
                } else {
                    tft.setTextColor(benOS_themes[benOS_currentTheme].fg, benOS_themes[benOS_currentTheme].bg);
                }
                
                String marker = (i == benOS_currentTheme) ? " *" : "";
                tft.drawString(String(i) + ": " + benOS_themes[i].name + marker, 10, y + 9);
            }
            
            lastSelected = selected;
        }
        
        int rotDiff = rotaryPos - lastRotary;
        if (rotDiff > 2) {
            selected++;
            if (selected >= benOS_themeCount) selected = benOS_themeCount - 1;
            lastRotary = rotaryPos;
        } else if (rotDiff < -2) {
            selected--;
            if (selected < 0) selected = 0;
            lastRotary = rotaryPos;
        }
        
        int btn = digitalRead(PIN_KO);
        if (btn == LOW && lastBtn == HIGH) {
            benOS_currentTheme = selected;
            while (digitalRead(PIN_KO) == LOW) delay(10);
            delay(200);
            return;
        }
        lastBtn = btn;
        
        delay(50);
    }
}

// ========== HELP ==========

void benOS_showHelp(TFT_eSPI &tft) {
    // Page 1
    benOS_clearScreen(tft);
    benOS_addLine("=== BenOS Commands (1/2) ===");
    benOS_addLine("NETWORK:");
    benOS_addLine("  wifi - Connect to WiFi");
    benOS_addLine("  wifiscan - Scan networks");
    benOS_addLine("  dig - DNS lookup");
    benOS_addLine("  curl - HTTP request");
    benOS_addLine("  ping - Ping host");
    benOS_addLine("  speedtest - Network speed");
    benOS_addLine("  dnsstart/stop/status");
    benOS_addLine("");
    benOS_addLine("SYSTEM:");
    benOS_addLine("  sysinfo - System info");
    benOS_addLine("  time - Show current time");
    benOS_addLine("  synctime - Sync with NTP");
    benOS_addLine("  version - OS version");
    benOS_addLine("");
    benOS_addLine("Press button for more...");
    benOS_redrawScreen(tft);
    
    // Wait for button
    while (digitalRead(PIN_KO) == LOW) delay(10);
    delay(200);
    int lastBtn = HIGH;
    while (true) {
        int btn = digitalRead(PIN_KO);
        if (btn == LOW && lastBtn == HIGH) break;
        lastBtn = btn;
        delay(50);
    }
    while (digitalRead(PIN_KO) == LOW) delay(10);
    delay(200);
    
    // Page 2
    benOS_clearScreen(tft);
    benOS_addLine("=== BenOS Commands (2/2) ===");
    benOS_addLine("TOOLS:");
    benOS_addLine("  passgen - Password gen");
    benOS_addLine("  qrcode - QR code gen");
    benOS_addLine("  calc - Calculator");
    benOS_addLine("");
    benOS_addLine("INFO:");
    benOS_addLine("  weather - Weather info");
    benOS_addLine("  stocks - Stock prices");
    benOS_addLine("  news - News headlines");
    benOS_addLine("");
    benOS_addLine("MEDIA & FILES:");
    benOS_addLine("  play - Music player");
    benOS_addLine("  fart - Fart soundboard");
    benOS_addLine("  ascii - ASCII art viewer");
    benOS_addLine("  cowsay - Cowsay");
    benOS_addLine("  ls/read/write/delete/image");
    benOS_addLine("");
    benOS_addLine("Press button to continue...");
    benOS_redrawScreen(tft);
}

// ========== COMMAND MENU ==========

const char* benOS_commands[] = {
    "help", "version", "time", "synctime", "wifi", "wifiscan", "dig", "curl",
    "ping", "sysinfo", "speedtest", "passgen", "qrcode", "ascii", "cowsay",
    "weather", "stocks", "news", "webserver", "playmusic", "fart", "timer",
    "calc", "write", "read", "delete", "ls", "image", "whoami",  // ← Added here
    "themes", "dnsstart", "dnsstop", "dnsstatus", "clock", "clear", "exit"
};
const int benOS_numCommands = 36;  // ← Update count

// ========== STATUS BAR ==========

void benOS_drawStatusBar(TFT_eSPI &tft, bool centerText = false, String centerTitle = "") {
    // Draw at top of screen
    tft.setTextFont(1);
    tft.setTextDatum(TL_DATUM);
    
    // Left side: Wi-Fi status
    if (WiFi.status() == WL_CONNECTED) {
        int rssi = WiFi.RSSI();
        String wifiIcon = "";
        uint16_t wifiColor = TFT_GREEN;
        
        // Signal strength bars
        if (rssi > -50) {
            wifiIcon = "[||||]";
            wifiColor = TFT_GREEN;
        } else if (rssi > -60) {
            wifiIcon = "[|||.]";
            wifiColor = TFT_GREEN;
        } else if (rssi > -70) {
            wifiIcon = "[||..]";
            wifiColor = TFT_YELLOW;
        } else if (rssi > -80) {
            wifiIcon = "[|...]";
            wifiColor = TFT_ORANGE;
        } else {
            wifiIcon = "[....]";
            wifiColor = TFT_RED;
        }
        
        tft.setTextColor(wifiColor, benOS_themes[benOS_currentTheme].bg);
        tft.drawString(wifiIcon, 2, 2);
    } else {
        tft.setTextColor(TFT_RED, benOS_themes[benOS_currentTheme].bg);
        tft.drawString("[X]", 2, 2);
    }
    
    // Center text if provided
    if (centerText && centerTitle.length() > 0) {
        tft.setTextFont(4);
        tft.setTextDatum(TC_DATUM);
        tft.setTextColor(benOS_themes[benOS_currentTheme].fg, benOS_themes[benOS_currentTheme].bg);
        tft.drawString(centerTitle, 160, 5);
    }
    
    // Right side: Time
    tft.setTextFont(1);
    tft.setTextDatum(TR_DATUM);
    
    time_t now = time(nullptr);
    if (now > 100000) {
        struct tm* t = localtime(&now);
        char timeStr[6];
        sprintf(timeStr, "%02d:%02d", t->tm_hour, t->tm_min);
        tft.setTextColor(TFT_CYAN, benOS_themes[benOS_currentTheme].bg);
        tft.drawString(timeStr, 318, 2);
    } else {
        tft.setTextColor(TFT_DARKGREY, benOS_themes[benOS_currentTheme].bg);
        tft.drawString("--:--", 318, 2);
    }
}

int benOS_selectCommand(TFT_eSPI &tft) {
    int selected = benOS_lastSelectedCommand;
    int lastSelected = -1;
    int lastRotary = rotaryPos;
    int lastBtn = HIGH;
    unsigned long lastStatusUpdate = 0;  // ← Add this
    unsigned long lastMenuActivity = millis();  // ← ADD: Track menu activity
    
    while (true) {
        benOS_processWebServer();
        
        // ========== SCREENSAVER CHECK IN MENU ========== 
        if (millis() - lastMenuActivity > 60000) {  // 60 seconds idle
            return -2;  // Special return code for screensaver
        }

        // ========== UPDATE STATUS BAR EVERY SECOND ==========
    if (millis() - lastStatusUpdate > 1000) {
        lastStatusUpdate = millis();
        benOS_drawStatusBar(tft, true, "BenOS Menu");
    }
        
        // Check for web commands
        if (benOS_webCommandCount > 0) {
            Serial.println("Web command received while in menu!");
            return -1;
        }
        
if (selected != lastSelected) {
    tft.fillScreen(benOS_themes[benOS_currentTheme].bg);
    tft.setTextColor(benOS_themes[benOS_currentTheme].fg, benOS_themes[benOS_currentTheme].bg);
    
    // Draw status bar with title
    benOS_drawStatusBar(tft, true, "BenOS Menu");
            
            int startIdx = selected - 4;
            if (startIdx < 0) startIdx = 0;
            if (startIdx > benOS_numCommands - 8) startIdx = benOS_numCommands - 8;
            if (benOS_numCommands < 8) startIdx = 0;
            
            tft.setTextFont(4);
            tft.setTextDatum(CL_DATUM);
            
            for (int i = 0; i < 8 && (startIdx + i) < benOS_numCommands; i++) {
                int idx = startIdx + i;
                int y = 35 + i * 26;
                
                if (idx == selected) {
                    tft.setTextColor(benOS_themes[benOS_currentTheme].bg, benOS_themes[benOS_currentTheme].fg);
                    tft.fillRect(5, y - 3, 310, 24, benOS_themes[benOS_currentTheme].fg);
                } else {
                    tft.setTextColor(benOS_themes[benOS_currentTheme].fg, benOS_themes[benOS_currentTheme].bg);
                }
                
                tft.drawString(benOS_commands[idx], 10, y + 10);
            }
            
            lastSelected = selected;
        }
        
        int rotDiff = rotaryPos - lastRotary;
        if (rotDiff > 2) {
            selected++;
            if (selected >= benOS_numCommands) selected = benOS_numCommands - 1;
            lastRotary = rotaryPos;
            lastMenuActivity = millis();  // ← Reset on activity
        } else if (rotDiff < -2) {
            selected--;
            if (selected < 0) selected = 0;
            lastRotary = rotaryPos;
            lastMenuActivity = millis();  // ← Reset on activity
        }
        
        int btn = digitalRead(PIN_KO);
        if (btn == LOW && lastBtn == HIGH) {
            while (digitalRead(PIN_KO) == LOW) delay(10);
            delay(200);
            benOS_lastSelectedCommand = selected;
            return selected;
        }
        lastBtn = btn;
        
        delay(50);
    }
}

// ========== TEXT INPUT (simplified) ==========

String benOS_textInput(TFT_eSPI &tft, String prompt) {
    tft.fillScreen(benOS_themes[benOS_currentTheme].bg);
    tft.setTextColor(benOS_themes[benOS_currentTheme].fg, benOS_themes[benOS_currentTheme].bg);
    tft.setTextFont(2);
    tft.setTextDatum(TC_DATUM);
    tft.drawString(prompt, 160, 10);
    
    tft.setTextDatum(TL_DATUM);
    tft.drawString("Use Serial Monitor to type", 10, 50);
    tft.drawString("Or press button to skip", 10, 70);
    
    String input = "";
    unsigned long startTime = millis();
    int lastBtn = HIGH;
    
    while (millis() - startTime < 10000) {
        while (Serial.available()) {
            char c = Serial.read();
            if (c == '\n' || c == '\r') {
                if (input.length() > 0) return input;
            } else {
                input += c;
                tft.fillRect(10, 100, 300, 30, benOS_themes[benOS_currentTheme].bg);
                tft.drawString(input, 10, 100);
            }
        }
        
        int btn = digitalRead(PIN_KO);
        if (btn == LOW && lastBtn == HIGH) {
            return "";
        }
        lastBtn = btn;
        
        delay(50);
    }
    
    return input;
}

// ========== URL SELECTION MENU ==========

struct URLExample {
    const char* name;
    const char* url;
};

URLExample benOS_urlExamples[] = {
    {"My Public IP", "http://api.ipify.org"},
    {"Random Dog Image", "http://dog.ceo/api/breeds/image/random"},
    {"IP Geolocation", "http://ip-api.com/json/"},
    {"Random Quote", "http://api.quotable.io/random"},
    {"Random Joke", "http://official-joke-api.appspot.com/random_joke"},
    {"ISS Location", "http://api.open-notify.org/iss-now.json"},
    {"Custom URL...", ""}
};

const int benOS_numURLExamples = sizeof(benOS_urlExamples) / sizeof(URLExample);

String benOS_urlInput(TFT_eSPI &tft) {
    int selected = 0;
    int lastSelected = -1;
    int lastRotary = rotaryPos;
    int lastBtn = HIGH;
    
    while (true) {
        if (selected != lastSelected) {
            tft.fillScreen(benOS_themes[benOS_currentTheme].bg);
            tft.setTextColor(benOS_themes[benOS_currentTheme].fg, benOS_themes[benOS_currentTheme].bg);
            
            tft.setTextFont(2);
            tft.setTextDatum(TC_DATUM);
            tft.drawString("Select URL or Custom", 160, 5);
            
            int startIdx = selected - 4;
            if (startIdx < 0) startIdx = 0;
            if (startIdx > benOS_numURLExamples - 8) startIdx = benOS_numURLExamples - 8;
            if (benOS_numURLExamples < 8) startIdx = 0;
            
            tft.setTextFont(2);
            tft.setTextDatum(CL_DATUM);
            
            for (int i = 0; i < 8 && (startIdx + i) < benOS_numURLExamples; i++) {
                int idx = startIdx + i;
                int y = 30 + i * 26;
                
if (idx == selected) {
                    tft.setTextColor(benOS_themes[benOS_currentTheme].bg, benOS_themes[benOS_currentTheme].fg);
                    tft.fillRect(2, y - 2, 316, 22, benOS_themes[benOS_currentTheme].fg);
                } else {
                    tft.setTextColor(benOS_themes[benOS_currentTheme].fg, benOS_themes[benOS_currentTheme].bg);
                }
                
                tft.drawString(benOS_urlExamples[idx].name, 6, y + 9);
            }
            
            lastSelected = selected;
        }
        
        int rotDiff = rotaryPos - lastRotary;
        if (rotDiff > 2) {
            selected++;
            if (selected >= benOS_numURLExamples) selected = benOS_numURLExamples - 1;
            lastRotary = rotaryPos;
        } else if (rotDiff < -2) {
            selected--;
            if (selected < 0) selected = 0;
            lastRotary = rotaryPos;
        }
        
        int btn = digitalRead(PIN_KO);
        if (btn == LOW && lastBtn == HIGH) {
            while (digitalRead(PIN_KO) == LOW) delay(10);
            delay(200);
            
            if (selected == benOS_numURLExamples - 1) {
                tft.fillScreen(benOS_themes[benOS_currentTheme].bg);
                tft.setTextColor(benOS_themes[benOS_currentTheme].fg, benOS_themes[benOS_currentTheme].bg);
                tft.setTextFont(2);
                tft.setTextDatum(TC_DATUM);
                tft.drawString("Enter Custom URL", 160, 10);
                
                tft.setTextDatum(TL_DATUM);
                tft.drawString("Use Serial Monitor", 10, 50);
                tft.drawString("Or press button to cancel", 10, 70);
                
                String input = "";
                unsigned long startTime = millis();
                int lastBtnInner = HIGH;
                
                while (millis() - startTime < 15000) {
                    while (Serial.available()) {
                        char c = Serial.read();
                        if (c == '\n' || c == '\r') {
                            if (input.length() > 0) return input;
                        } else {
                            input += c;
                            tft.fillRect(10, 110, 300, 30, benOS_themes[benOS_currentTheme].bg);
                            tft.drawString(input.substring(0, 38), 10, 110);
                        }
                    }
                    
                    int btnInner = digitalRead(PIN_KO);
                    if (btnInner == LOW && lastBtnInner == HIGH) {
                        return "";
                    }
                    lastBtnInner = btnInner;
                    delay(50);
                }
                
                return "";
            } else {
                return String(benOS_urlExamples[selected].url);
            }
        }
        lastBtn = btn;
        
        delay(50);
    }
}
// ========== PING COMMAND ==========

struct PingHost {
    const char* name;
    const char* host;
};

PingHost benOS_pingHosts[] = {
    {"Google DNS", "8.8.8.8"},
    {"Cloudflare DNS", "1.1.1.1"},
    {"Google", "google.com"},
    {"GitHub", "github.com"},
    {"Router", "192.168.1.1"},
    {"Custom...", ""}
};

const int benOS_numPingHosts = sizeof(benOS_pingHosts) / sizeof(PingHost);

String benOS_selectPingHost(TFT_eSPI &tft) {
    int selected = 0;
    int lastSelected = -1;
    int lastRotary = rotaryPos;
    int lastBtn = HIGH;
    
    while (true) {
        if (selected != lastSelected) {
            tft.fillScreen(benOS_themes[benOS_currentTheme].bg);
            tft.setTextColor(benOS_themes[benOS_currentTheme].fg, benOS_themes[benOS_currentTheme].bg);
            
            tft.setTextFont(2);
            tft.setTextDatum(TC_DATUM);
            tft.drawString("Select Host to Ping", 160, 5);
            
            tft.setTextFont(2);
            tft.setTextDatum(CL_DATUM);
            
            for (int i = 0; i < benOS_numPingHosts; i++) {
                int y = 30 + i * 30;
                
                if (i == selected) {
                    tft.setTextColor(benOS_themes[benOS_currentTheme].bg, benOS_themes[benOS_currentTheme].fg);
                    tft.fillRect(2, y - 2, 316, 26, benOS_themes[benOS_currentTheme].fg);
                } else {
                    tft.setTextColor(benOS_themes[benOS_currentTheme].fg, benOS_themes[benOS_currentTheme].bg);
                }
                
                String display = String(benOS_pingHosts[i].name);
                if (strlen(benOS_pingHosts[i].host) > 0) {
                    display += " - " + String(benOS_pingHosts[i].host);
                }
                tft.drawString(display.substring(0, 38), 6, y + 11);
            }
            
            lastSelected = selected;
        }
        
        int rotDiff = rotaryPos - lastRotary;
        if (rotDiff > 2) {
            selected++;
            if (selected >= benOS_numPingHosts) selected = benOS_numPingHosts - 1;
            lastRotary = rotaryPos;
        } else if (rotDiff < -2) {
            selected--;
            if (selected < 0) selected = 0;
            lastRotary = rotaryPos;
        }
        
        int btn = digitalRead(PIN_KO);
        if (btn == LOW && lastBtn == HIGH) {
            while (digitalRead(PIN_KO) == LOW) delay(10);
            delay(200);
            
            if (selected == benOS_numPingHosts - 1) {
                return benOS_textInput(tft, "Enter host:");
            } else {
                return String(benOS_pingHosts[selected].host);
            }
        }
        lastBtn = btn;
        
        delay(50);
    }
}

void benOS_ping(TFT_eSPI &tft, String host) {
    benOS_clearScreen(tft);
    benOS_addLine("PING " + host);
    benOS_addLine("===================");
    benOS_redrawScreen(tft);
    
    IPAddress ip;
    if (!WiFi.hostByName(host.c_str(), ip)) {
        benOS_addLine("DNS lookup failed");
        benOS_redrawScreen(tft);
        return;
    }
    
    benOS_addLine("IP: " + ip.toString());
    benOS_addLine("");
    benOS_redrawScreen(tft);
    
    int sent = 0, received = 0;
    long totalTime = 0;
    
    for (int i = 0; i < 5; i++) {
        unsigned long start = millis();
        
        if (Ping.ping(ip, 1)) {
            long rtt = millis() - start;
            totalTime += rtt;
            received++;
            benOS_addLine("Reply: time=" + String(rtt) + "ms");
        } else {
            benOS_addLine("Request timeout");
        }
        
        sent++;
        benOS_redrawScreen(tft);
        delay(1000);
    }
    
    benOS_addLine("");
    benOS_addLine("--- Statistics ---");
    benOS_addLine("Sent: " + String(sent) + ", Received: " + String(received));
    
    if (received > 0) {
        benOS_addLine("Avg time: " + String(totalTime / received) + "ms");
        int loss = ((sent - received) * 100) / sent;
        benOS_addLine("Packet loss: " + String(loss) + "%");
    } else {
        benOS_addLine("100% packet loss");
    }
    
    benOS_redrawScreen(tft);
}

// ========== WIFI SCANNER ==========

void benOS_wifiScan(TFT_eSPI &tft) {
    benOS_clearScreen(tft);
    benOS_addLine("Scanning WiFi networks...");
    benOS_redrawScreen(tft);
    
    int n = WiFi.scanNetworks();
    
    benOS_clearScreen(tft);
    benOS_addLine("Found " + String(n) + " networks");
    benOS_addLine("===================");
    
    int displayed = 0;
    int page = 0;
    const int perPage = 4;  // Show 4 networks per page
    
    while (displayed < n) {
        benOS_clearScreen(tft);
        benOS_addLine("WiFi Networks (Page " + String(page + 1) + ")");
        benOS_addLine("===================");
        benOS_addLine("");
        
        for (int i = 0; i < perPage && displayed < n; i++, displayed++) {
            String ssid = WiFi.SSID(displayed);
            int rssi = WiFi.RSSI(displayed);
            String encType = (WiFi.encryptionType(displayed) == WIFI_AUTH_OPEN) ? " " : "*";
            
            // Signal strength bars
            String bars = "";
            if (rssi > -50) bars = "[====]";
            else if (rssi > -60) bars = "[=== ]";
            else if (rssi > -70) bars = "[==  ]";
            else if (rssi > -80) bars = "[=   ]";
            else bars = "[    ]";
            
            benOS_addLine(String(displayed+1) + "." + encType + " " + ssid.substring(0, 18));
            benOS_addLine("   " + String(rssi) + "dBm " + bars + " Ch" + String(WiFi.channel(displayed)));
        }
        
        benOS_addLine("");
        if (displayed < n) {
            benOS_addLine("(" + String(n - displayed) + " more networks)");
            benOS_addLine("Press button for next page");
        } else {
            benOS_addLine("--- End of list ---");
            benOS_addLine("Press button to continue");
        }
        
        benOS_redrawScreen(tft);
        
        // Wait for button press
        while (digitalRead(PIN_KO) == LOW) delay(10);
        delay(200);
        int lastBtn = HIGH;
        while (true) {
            int btn = digitalRead(PIN_KO);
            if (btn == LOW && lastBtn == HIGH) break;
            lastBtn = btn;
            delay(50);
        }
        while (digitalRead(PIN_KO) == LOW) delay(10);
        delay(200);
        
        page++;
    }
    
    WiFi.scanDelete();
}

// ========== SYSTEM INFO ==========

void benOS_sysinfo(TFT_eSPI &tft) {
    benOS_clearScreen(tft);
    benOS_addLine("=== System Information ===");
    benOS_addLine("");
    
    // Uptime
    unsigned long uptime = millis() / 1000;
    int hours = uptime / 3600;
    int mins = (uptime % 3600) / 60;
    int secs = uptime % 60;
    benOS_addLine("Uptime: " + String(hours) + "h " + String(mins) + "m " + String(secs) + "s");
    
    // Memory
    benOS_addLine("Free Heap: " + String(ESP.getFreeHeap() / 1024) + "KB");
    benOS_addLine("Total Heap: " + String(ESP.getHeapSize() / 1024) + "KB");
    benOS_addLine("Flash Size: " + String(ESP.getFlashChipSize() / 1024 / 1024) + "MB");
    
    // CPU
    benOS_addLine("CPU Freq: " + String(ESP.getCpuFreqMHz()) + "MHz");
    benOS_addLine("Chip: " + String(ESP.getChipModel()));
    benOS_addLine("Cores: " + String(ESP.getChipCores()));
    
    // WiFi
    if (WiFi.status() == WL_CONNECTED) {
        benOS_addLine("WiFi: Connected");
        benOS_addLine("IP: " + WiFi.localIP().toString());
        benOS_addLine("Signal: " + String(WiFi.RSSI()) + "dBm");
    } else {
        benOS_addLine("WiFi: Disconnected");
    }
    
    benOS_redrawScreen(tft);
}

// ========== SPEED TEST ==========

void benOS_speedtest(TFT_eSPI &tft) {
    benOS_clearScreen(tft);
    benOS_addLine("Network Speed Test");
    benOS_addLine("===================");
    benOS_addLine("");
    benOS_addLine("Testing ping...");
    benOS_redrawScreen(tft);
    
    // Ping test
    IPAddress testIP;
    WiFi.hostByName("google.com", testIP);
    
    unsigned long pingStart = millis();
    bool pingSuccess = Ping.ping(testIP, 1);
    long pingTime = millis() - pingStart;
    
    if (pingSuccess) {
        benOS_addLine("Ping: " + String(pingTime) + "ms");
    } else {
        benOS_addLine("Ping: Failed");
    }
    
    benOS_addLine("");
    benOS_addLine("Testing download...");
    benOS_redrawScreen(tft);
    
    // Download test (small file)
    HTTPClient http;
    http.begin("http://ipv4.download.thinkbroadband.com/5MB.zip");
    
    unsigned long dlStart = millis();
    int code = http.GET();
    
    if (code == HTTP_CODE_OK) {
        WiFiClient * stream = http.getStreamPtr();
        uint8_t buff[128];
        int totalBytes = 0;
        
        while (http.connected() && totalBytes < 500000) {  // Test 500KB
            size_t size = stream->available();
            if (size) {
                int c = stream->readBytes(buff, min(size, sizeof(buff)));
                totalBytes += c;
            }
        }
        
        unsigned long dlTime = millis() - dlStart;
        float mbps = (totalBytes * 8.0) / (dlTime * 1000.0);
        
        benOS_addLine("Download: " + String(mbps, 1) + " Mbps");
        benOS_addLine("Downloaded: " + String(totalBytes / 1024) + "KB");
        benOS_addLine("Time: " + String(dlTime / 1000.0, 1) + "s");
    } else {
        benOS_addLine("Download test failed");
    }
    
    http.end();
    benOS_redrawScreen(tft);
}

// ========== PASSWORD GENERATOR ==========

void benOS_passgen(TFT_eSPI &tft) {
    const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    const char* symbols = "!@#$%^&*()-_=+[]{}|;:,.<>?";
    
    int lengths[] = {8, 12, 16, 20, 24};
    int selected = 2;  // Default 16
    int lastSelected = -1;
    int lastRotary = rotaryPos;
    int lastBtn = HIGH;
    bool includeSymbols = true;
    
    while (true) {
        if (selected != lastSelected) {
            benOS_clearScreen(tft);
            benOS_addLine("Password Generator");
            benOS_addLine("===================");
            benOS_addLine("");
            benOS_addLine("Select length:");
            
            for (int i = 0; i < 5; i++) {
                String line = "  " + String(lengths[i]) + " characters";
                if (i == selected) line = "> " + String(lengths[i]) + " characters";
                benOS_addLine(line);
            }
            
            benOS_addLine("");
            benOS_addLine("Symbols: " + String(includeSymbols ? "Yes" : "No"));
            benOS_addLine("");
            benOS_addLine("Press button to generate");
            benOS_redrawScreen(tft);
            
            lastSelected = selected;
        }
        
        int rotDiff = rotaryPos - lastRotary;
        if (rotDiff > 2) {
            selected++;
            if (selected >= 5) selected = 4;
            lastRotary = rotaryPos;
        } else if (rotDiff < -2) {
            selected--;
            if (selected < 0) selected = 0;
            lastRotary = rotaryPos;
        }
        
        int btn = digitalRead(PIN_KO);
        if (btn == LOW && lastBtn == HIGH) {
            while (digitalRead(PIN_KO) == LOW) delay(10);
            delay(200);
            
            // Generate password
            String password = "";
            int len = lengths[selected];
            
            for (int i = 0; i < len; i++) {
                if (includeSymbols && random(100) < 20) {
                    password += symbols[random(strlen(symbols))];
                } else {
                    password += chars[random(strlen(chars))];
                }
            }
            
            benOS_clearScreen(tft);
            benOS_addLine("Generated Password:");
            benOS_addLine("===================");
            benOS_addLine("");
            
            // Display password in chunks
            for (int i = 0; i < password.length(); i += 20) {
                benOS_addLine(password.substring(i, min(i + 20, (int)password.length())));
            }
            
            benOS_addLine("");
            benOS_addLine("Length: " + String(len));
            benOS_addLine("Symbols: " + String(includeSymbols ? "Yes" : "No"));
            benOS_redrawScreen(tft);
            
            Serial.println("Generated password: " + password);
            return;
        }
        lastBtn = btn;
        
        delay(50);
    }
}

// ========== QR CODE GENERATOR ==========

void benOS_drawQRPixel(TFT_eSPI &tft, int x, int y, int scale, bool black) {
    if (black) {
        tft.fillRect(x, y, scale, scale, TFT_BLACK);
    } else {
        tft.fillRect(x, y, scale, scale, TFT_WHITE);
    }
}

void benOS_qrcode(TFT_eSPI &tft) {
    int selected = 0;
    int lastSelected = -1;
    int lastRotary = rotaryPos;
    int lastBtn = HIGH;
    
    const char* qrOptions[] = {
        "WiFi Credentials",
        "Web Server URL",
        "Custom Text"
    };
    const int numOptions = 3;
    
    while (true) {
        if (selected != lastSelected) {
            benOS_clearScreen(tft);
            benOS_addLine("QR Code Generator");
            benOS_addLine("===================");
            benOS_addLine("");
            benOS_addLine("Select content:");
            
            for (int i = 0; i < numOptions; i++) {
                String line = "  " + String(qrOptions[i]);
                if (i == selected) line = "> " + String(qrOptions[i]);
                benOS_addLine(line);
            }
            
            benOS_redrawScreen(tft);
            lastSelected = selected;
        }
        
        int rotDiff = rotaryPos - lastRotary;
        if (rotDiff > 2) {
            selected++;
            if (selected >= numOptions) selected = numOptions - 1;
            lastRotary = rotaryPos;
        } else if (rotDiff < -2) {
            selected--;
            if (selected < 0) selected = 0;
            lastRotary = rotaryPos;
        }
        
        int btn = digitalRead(PIN_KO);
        if (btn == LOW && lastBtn == HIGH) {
            while (digitalRead(PIN_KO) == LOW) delay(10);
            delay(200);
            
            String qrText = "";
            
            if (selected == 0) {
                qrText = "WIFI:T:WPA;S:" + String(BenOS_Config::WIFI_SSID) + 
                         ";P:" + String(BenOS_Config::WIFI_PASS) + ";;";
            } else if (selected == 1) {
                qrText = "http://" + WiFi.localIP().toString();
            } else {
                qrText = benOS_textInput(tft, "Enter text for QR:");
                if (qrText.length() == 0) return;
            }
            
            // Clear screen
            tft.fillScreen(TFT_WHITE);
            tft.setTextColor(TFT_BLACK, TFT_WHITE);
            tft.setTextFont(2);
            tft.setTextDatum(TC_DATUM);
            tft.drawString("QR Code", 160, 5);
            
            // Generate QR code using saved reference
            using namespace qrcodegen;
const QrCode qr = QrCode::encodeText(qrText.c_str(), 3, QR_ECC_LOW);  // Version 3

            // Get QR code size
            int size = qr.size;
            int scale = 3;
            int qrWidth = size * scale;
            int offsetX = (320 - qrWidth) / 2;
            int offsetY = 35;
            
            // Draw white border
            tft.fillRect(offsetX - 8, offsetY - 8, qrWidth + 16, qrWidth + 16, TFT_WHITE);
            
            // Draw QR code
            for (int y = 0; y < size; y++) {
                for (int x = 0; x < size; x++) {
                    if (qr.getModule(x, y)) {
                        tft.fillRect(offsetX + (x * scale), offsetY + (y * scale), 
                                   scale, scale, TFT_BLACK);
                    }
                }
            }
            
            // Border
            tft.drawRect(offsetX - 2, offsetY - 2, qrWidth + 4, qrWidth + 4, TFT_BLACK);
            
            tft.setTextDatum(BC_DATUM);
            tft.setTextFont(2);
            tft.drawString("Scan this code", 160, 210);
            tft.drawString("Press button to return", 160, 230);
            
            Serial.println("QR Code: " + qrText);
            
            // Wait for button
            while (digitalRead(PIN_KO) == LOW) delay(10);
            delay(200);
            lastBtn = HIGH;
            while (true) {
                int btn = digitalRead(PIN_KO);
                if (btn == LOW && lastBtn == HIGH) break;
                lastBtn = btn;
                delay(50);
            }
            
            return;
        }
        lastBtn = btn;
        delay(50);
    }
}

// ========== ASCII ART VIEWER ==========

void benOS_ascii(TFT_eSPI &tft) {
    // List ASCII files from SD
    File root = SD.open("/ascii");
    if (!root || !root.isDirectory()) {
        benOS_clearScreen(tft);
        benOS_addLine("No /ascii folder found");
        benOS_addLine("Create /ascii/ on SD card");
        benOS_addLine("Add .txt files with ASCII art");
        benOS_redrawScreen(tft);
        return;
    }
    
    String files[20];
    int fileCount = 0;
    
    File file = root.openNextFile();
    while (file && fileCount < 20) {
        if (!file.isDirectory()) {
            String name = String(file.name());
            if (name.endsWith(".txt")) {
                files[fileCount++] = name;
            }
        }
        file = root.openNextFile();
    }
    
    if (fileCount == 0) {
        benOS_clearScreen(tft);
        benOS_addLine("No .txt files in /ascii/");
        benOS_redrawScreen(tft);
        return;
    }
    
    // Select file
    int selected = 0;
    int lastSelected = -1;
    int lastRotary = rotaryPos;
    int lastBtn = HIGH;
    
    while (true) {
        if (selected != lastSelected) {
            benOS_clearScreen(tft);
            benOS_addLine("Select ASCII Art:");
            benOS_addLine("===================");
            
            for (int i = 0; i < min(fileCount, 12); i++) {
                String line = (i == selected) ? "> " : "  ";
                line += files[i];
                benOS_addLine(line.substring(0, 38));
            }
            
            benOS_redrawScreen(tft);
            lastSelected = selected;
        }
        
        int rotDiff = rotaryPos - lastRotary;
        if (rotDiff > 2) {
            selected++;
            if (selected >= fileCount) selected = fileCount - 1;
            lastRotary = rotaryPos;
        } else if (rotDiff < -2) {
            selected--;
            if (selected < 0) selected = 0;
            lastRotary = rotaryPos;
        }
        
        int btn = digitalRead(PIN_KO);
        if (btn == LOW && lastBtn == HIGH) {
            while (digitalRead(PIN_KO) == LOW) delay(10);
            delay(200);
            
            // Display selected file
            File artFile = SD.open("/ascii/" + files[selected]);
            if (artFile) {
                tft.fillScreen(benOS_themes[benOS_currentTheme].bg);
                tft.setTextColor(benOS_themes[benOS_currentTheme].fg, benOS_themes[benOS_currentTheme].bg);
                tft.setTextFont(1);
                tft.setCursor(0, 0);
                
                while (artFile.available()) {
                    String line = artFile.readStringUntil('\n');
                    tft.println(line);
                }
                
                artFile.close();
                
                // Wait for button
                while (digitalRead(PIN_KO) == LOW) delay(10);
                delay(200);
                lastBtn = HIGH;
                while (true) {
                    int btn = digitalRead(PIN_KO);
                    if (btn == LOW && lastBtn == HIGH) break;
                    lastBtn = btn;
                    delay(50);
                }
            }
            
            return;
        }
        lastBtn = btn;
        delay(50);
    }
}

// ========== COWSAY ==========

void benOS_cowsay(TFT_eSPI &tft) {
    String message = benOS_textInput(tft, "Cow says:");
    if (message.length() == 0) message = "Hello from BenOS!";
    
    tft.fillScreen(benOS_themes[benOS_currentTheme].bg);
    tft.setTextColor(benOS_themes[benOS_currentTheme].fg, benOS_themes[benOS_currentTheme].bg);
    tft.setTextFont(1);
    tft.setCursor(10, 10);
    
    // Top border
    String border = " ";
    for (int i = 0; i < min((int)message.length() + 2, 50); i++) border += "_";
    tft.println(border);
    
    // Message
    tft.print("< ");
    tft.print(message.substring(0, 48));
    tft.println(" >");
    
    // Bottom border
    border = " ";
    for (int i = 0; i < min((int)message.length() + 2, 50); i++) border += "-";
    tft.println(border);
    
    // Cow
    tft.println("        \\   ^__^");
    tft.println("         \\  (oo)\\_______");
    tft.println("            (__)\\       )\\/\\");
    tft.println("                ||----w |");
    tft.println("                ||     ||");
    
    tft.setTextFont(2);
    tft.setTextDatum(BC_DATUM);
    tft.drawString("Press button to return", 160, 230);
    
    // Wait for button
    while (digitalRead(PIN_KO) == LOW) delay(10);
    delay(200);
    int lastBtn = HIGH;
    while (true) {
        int btn = digitalRead(PIN_KO);
        if (btn == LOW && lastBtn == HIGH) break;
        lastBtn = btn;
        delay(50);
    }
}

// ========== WEATHER ==========

void benOS_weather(TFT_eSPI &tft) {
    benOS_clearScreen(tft);
    benOS_addLine("Weather");
    benOS_addLine("Newton Highlands, MA");
    benOS_addLine("===================");
    benOS_addLine("");
    benOS_addLine("Fetching...");
    benOS_redrawScreen(tft);
    
    // Newton Highlands, MA coordinates: 42.3370, -71.2092
    HTTPClient http;
    http.begin("http://api.open-meteo.com/v1/forecast?latitude=42.337&longitude=-71.209&current_weather=true&temperature_unit=fahrenheit&windspeed_unit=mph");
    http.setTimeout(10000);
    
    int code = http.GET();
    
    benOS_clearScreen(tft);
    benOS_addLine("Weather");
    benOS_addLine("Newton Highlands, MA");
    benOS_addLine("===================");
    benOS_addLine("");
    
    if (code == HTTP_CODE_OK) {
        String json = http.getString();
        
        Serial.println("Weather JSON response:");
        Serial.println(json);  // Debug output
        
        // Find current_weather object
        int currentWeatherIdx = json.indexOf("\"current_weather\":");
        if (currentWeatherIdx > 0) {
            String weatherData = json.substring(currentWeatherIdx);
            
            // Parse temperature (comes after "temperature":)
            int tempIdx = weatherData.indexOf("\"temperature\":");
            if (tempIdx > 0) {
                int tempStart = tempIdx + 14;
                int tempEnd = weatherData.indexOf(',', tempStart);
                if (tempEnd < 0) tempEnd = weatherData.indexOf('}', tempStart);
                String temp = weatherData.substring(tempStart, tempEnd);
                temp.trim();
                benOS_addLine("Temperature: " + temp + " F");
            }
            
            // Parse windspeed
            int windIdx = weatherData.indexOf("\"windspeed\":");
            if (windIdx > 0) {
                int windStart = windIdx + 12;
                int windEnd = weatherData.indexOf(',', windStart);
                if (windEnd < 0) windEnd = weatherData.indexOf('}', windStart);
                String wind = weatherData.substring(windStart, windEnd);
                wind.trim();
                benOS_addLine("Wind: " + wind + " mph");
            }
            
            // Parse weather code
            int codeIdx = weatherData.indexOf("\"weathercode\":");
            if (codeIdx > 0) {
                int codeStart = codeIdx + 14;
                int codeEnd = weatherData.indexOf(',', codeStart);
                if (codeEnd < 0) codeEnd = weatherData.indexOf('}', codeStart);
                String codeStr = weatherData.substring(codeStart, codeEnd);
                codeStr.trim();
                
                int weatherCode = codeStr.toInt();
                String condition = "Unknown";
                
                if (weatherCode == 0) condition = "Clear sky";
                else if (weatherCode <= 3) condition = "Partly cloudy";
                else if (weatherCode <= 48) condition = "Fog";
                else if (weatherCode <= 67) condition = "Rainy";
                else if (weatherCode <= 77) condition = "Snowy";
                else if (weatherCode <= 82) condition = "Rain showers";
                else if (weatherCode <= 86) condition = "Snow showers";
                else condition = "Thunderstorm";
                
                benOS_addLine("Conditions: " + condition);
            }
            
            // Parse time
            int timeIdx = weatherData.indexOf("\"time\":\"");
            if (timeIdx > 0) {
                int timeStart = timeIdx + 8;
                int timeEnd = weatherData.indexOf("\"", timeStart);
                String timeStr = weatherData.substring(timeStart, timeEnd);
                benOS_addLine("");
                benOS_addLine("Updated: " + timeStr);
            }
        } else {
            benOS_addLine("Parse error");
        }
        
    } else {
        benOS_addLine("Failed to fetch weather");
        benOS_addLine("Error: " + String(code));
    }
    
    http.end();
    benOS_redrawScreen(tft);
}

// ========== STOCK MARKET ==========

void benOS_stocks(TFT_eSPI &tft) {
    // Auto-sync time if not synced
    if (time(nullptr) < 100000) {
        benOS_clearScreen(tft);
        benOS_addLine("Syncing time first...");
        benOS_redrawScreen(tft);
        
        configTime(BenOS_Config::GMT_OFFSET, BenOS_Config::DAYLIGHT_OFFSET, BenOS_Config::NTP_SERVER);
        int attempts = 0;
        while (time(nullptr) < 100000 && attempts < 20) {
            delay(500);
            attempts++;
        }
    }
    
    benOS_clearScreen(tft);
    benOS_addLine("Stock Market");
    benOS_addLine("===================");
    benOS_addLine("");
    benOS_addLine("Fetching indices...");
    benOS_redrawScreen(tft);
    
    const char* symbols[] = {"^GSPC", "^IXIC", "^DJI"};  // S&P 500, NASDAQ, Dow Jones
    const char* names[] = {"S&P 500", "NASDAQ", "Dow Jones"};
    
    benOS_clearScreen(tft);
    benOS_addLine("Stock Market Indices");
    benOS_addLine("===================");
    benOS_addLine("");
    
    // Use WiFiClientSecure for HTTPS
    WiFiClientSecure *client = new WiFiClientSecure;
    if (client) {
        client->setInsecure();  // Skip certificate validation
        
        for (int i = 0; i < 3; i++) {
            HTTPClient http;
            
            String url = "https://query1.finance.yahoo.com/v8/finance/chart/";
            url += symbols[i];
            url += "?interval=1d&range=1d";
            
            Serial.println("Fetching: " + url);
            
            if (http.begin(*client, url)) {
                http.setTimeout(10000);
                http.addHeader("User-Agent", "Mozilla/5.0");
                
                int code = http.GET();
                Serial.println("Response code: " + String(code));
                
                if (code == HTTP_CODE_OK) {
                    String response = http.getString();
                    
                    // Parse price from JSON
                    int priceIdx = response.indexOf("\"regularMarketPrice\":");
                    if (priceIdx > 0) {
                        int priceStart = priceIdx + 21;
                        int priceEnd = response.indexOf(',', priceStart);
                        if (priceEnd < 0) priceEnd = response.indexOf('}', priceStart);
                        
                        String price = response.substring(priceStart, priceEnd);
                        price.trim();
                        
                        benOS_addLine(String(names[i]) + ":");
                        benOS_addLine("  " + price);
                    } else {
                        benOS_addLine(String(names[i]) + ": N/A");
                    }
                } else {
                    benOS_addLine(String(names[i]) + ": Error " + String(code));
                }
                
                http.end();
            } else {
                benOS_addLine(String(names[i]) + ": Connect failed");
            }
            
            benOS_redrawScreen(tft);
            delay(1000);
        }
        
        delete client;
    } else {
        benOS_addLine("SSL client failed");
    }
    
    benOS_addLine("");
    benOS_addLine("Updated: " + benOS_getTime());
    benOS_redrawScreen(tft);
}

// ========== NEWS HEADLINES ==========

void benOS_news(TFT_eSPI &tft) {
    benOS_clearScreen(tft);
    benOS_addLine("News Headlines");
    benOS_addLine("===================");
    benOS_addLine("");
    benOS_addLine("Fetching...");
    benOS_redrawScreen(tft);
    
    HTTPClient http;
    // Using NewsAPI.org - you'll need to get a free API key
    // For demo, using a simple RSS-to-JSON service
    http.begin("http://api.rss2json.com/v1/api.json?rss_url=http://rss.cnn.com/rss/cnn_topstories.rss");
    http.setTimeout(10000);
    
    int code = http.GET();
    
    benOS_clearScreen(tft);
    benOS_addLine("Top News Headlines");
    benOS_addLine("===================");
    benOS_addLine("");
    
    if (code == HTTP_CODE_OK) {
        String json = http.getString();
        
        // Simple parsing for first 3 titles
        int titleCount = 0;
        int searchPos = 0;
        
        while (titleCount < 3 && searchPos < json.length()) {
            int titleIdx = json.indexOf("\"title\":\"", searchPos);
            if (titleIdx < 0) break;
            
            int titleStart = titleIdx + 9;
            int titleEnd = json.indexOf("\"", titleStart);
            
            if (titleEnd > titleStart) {
                String title = json.substring(titleStart, titleEnd);
                title.replace("\\u0027", "'");
                title.replace("\\\"", "\"");
                
                benOS_addLine(String(titleCount + 1) + ". " + title.substring(0, 35));
                if (title.length() > 35) {
                    benOS_addLine("   " + title.substring(35, min(70, (int)title.length())));
                }
                benOS_addLine("");
                
                titleCount++;
            }
            
            searchPos = titleEnd + 1;
        }
        
        if (titleCount == 0) {
            benOS_addLine("No headlines found");
        }
    } else {
        benOS_addLine("Failed to fetch news");
        benOS_addLine("Error: " + String(code));
    }
    
    http.end();
    benOS_redrawScreen(tft);
}

// ========== WEB SERVER ==========

WiFiServer* benOS_webServer = nullptr;
bool benOS_webServerRunning = false;

void benOS_webserver(TFT_eSPI &tft) {
    int selected = 0;
    int lastSelected = -1;
    int lastRotary = rotaryPos;
    int lastBtn = HIGH;
    
    const char* options[] = {"Start Server", "Stop Server", "Status"};
    const int numOptions = 3;
    
    while (true) {
        if (selected != lastSelected) {
            benOS_clearScreen(tft);
            benOS_addLine("Web Server");
            benOS_addLine("===================");
            benOS_addLine("");
            benOS_addLine("Status: " + String(benOS_webServerRunning ? "Running" : "Stopped"));
            benOS_addLine("");
            
            for (int i = 0; i < numOptions; i++) {
                String line = (i == selected) ? "> " : "  ";
                line += options[i];
                benOS_addLine(line);
            }
            
            benOS_redrawScreen(tft);
            lastSelected = selected;
        }
        
        int rotDiff = rotaryPos - lastRotary;
        if (rotDiff > 2) {
            selected++;
            if (selected >= numOptions) selected = numOptions - 1;
            lastRotary = rotaryPos;
        } else if (rotDiff < -2) {
            selected--;
            if (selected < 0) selected = 0;
            lastRotary = rotaryPos;
        }
        
        int btn = digitalRead(PIN_KO);
        if (btn == LOW && lastBtn == HIGH) {
            while (digitalRead(PIN_KO) == LOW) delay(10);
            delay(200);
            
            if (selected == 0 && !benOS_webServerRunning) {
                // Start server
                benOS_webServer = new WiFiServer(80);
                benOS_webServer->begin();
                benOS_webServerRunning = true;
                
                benOS_clearScreen(tft);
                benOS_addLine("Web Server Started");
                benOS_addLine("===================");
                benOS_addLine("");
                benOS_addLine("Access at:");
                benOS_addLine("http://" + WiFi.localIP().toString());
                benOS_addLine("");
                benOS_addLine("Serving files from /www/");
                benOS_addLine("");
                benOS_addLine("Running in background...");
                benOS_redrawScreen(tft);
                
                Serial.println("Web server started: http://" + WiFi.localIP().toString());
                return;
                
            } else if (selected == 1 && benOS_webServerRunning) {
                // Stop server
                if (benOS_webServer) {
                    benOS_webServer->stop();
                    delete benOS_webServer;
                    benOS_webServer = nullptr;
                }
                benOS_webServerRunning = false;
                
                benOS_clearScreen(tft);
                benOS_addLine("Web server stopped");
                benOS_redrawScreen(tft);
                delay(1000);
                return;
                
            } else if (selected == 2) {
                // Show status
                benOS_clearScreen(tft);
                benOS_addLine("Web Server Status");
                benOS_addLine("===================");
                benOS_addLine("");
                
                if (benOS_webServerRunning) {
                    benOS_addLine("Status: Running");
                    benOS_addLine("URL: http://" + WiFi.localIP().toString());
                    benOS_addLine("Port: 80");
                } else {
                    benOS_addLine("Status: Stopped");
                }
                
                benOS_redrawScreen(tft);
                delay(2000);
                return;
            }
        }
        lastBtn = btn;
        delay(50);
    }
}


// ========== FORWARD DECLARATIONS FOR WEB SERVER ==========
void benOS_processWebServer();
void serve404(WiFiClient &client);
void serveFile(WiFiClient &client, const char *path);
void serveStatus(WiFiClient &client);
void serveOutput(WiFiClient &client);
void serveCommand(WiFiClient &client, String path);
void serveDefaultPage(WiFiClient &client);


void serveOutput(WiFiClient &client) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Connection: close");
    client.println();
    
    String json = "{";
    json += "\"done\":" + String(benOS_webCommandDone ? "true" : "false") + ",";
    json += "\"output\":\"";
    
    // Escape newlines and quotes for JSON
    String escapedOutput = benOS_webCommandOutput;
    escapedOutput.replace("\\", "\\\\");
    escapedOutput.replace("\"", "\\\"");
    escapedOutput.replace("\n", "\\n");
    
    json += escapedOutput;
    json += "\"}";
    
    client.println(json);
}

// ========== ENHANCED WEB SERVER WITH API ==========

void benOS_processWebServer() {
    if (!benOS_webServerRunning || !benOS_webServer) return;
    
    WiFiClient client = benOS_webServer->available();
    if (client) {
        Serial.println("Web client connected!");
        
        String request = "";
        String firstLine = "";
        unsigned long timeout = millis();
        
        while (client.connected() && millis() - timeout < 2000) {
            if (client.available()) {
                char c = client.read();
                request += c;
                
                if (firstLine == "" && c == '\n') {
                    int endOfLine = request.indexOf('\n');
                    if (endOfLine > 0) {
                        firstLine = request.substring(0, endOfLine);
                    }
                }
                
                if (c == '\n' && request.endsWith("\r\n\r\n")) {
                    break;
                }
            }
        }
        
        // Parse request path
        String path = "/";
        int pathStart = firstLine.indexOf("GET ") + 4;
        int pathEnd = firstLine.indexOf(" HTTP");
        if (pathStart > 3 && pathEnd > pathStart) {
            path = firstLine.substring(pathStart, pathEnd);
        }
        
        Serial.println("Web request: " + path);
        
// Route requests
if (path == "/" || path == "/index.html") {
    if (SD.exists("/www/index.html")) {
        serveFile(client, "/www/index.html");
    } else {
        serveDefaultPage(client);
    }
}
else if (path == "/api/status") {
    serveStatus(client);
}
else if (path == "/api/output") {  
    serveOutput(client);
}    
else if (path.startsWith("/api/command")) {
    serveCommand(client, path);
}
else if (path == "/api/reboot") {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();
    client.println("Rebooting...");
    client.stop();
    delay(1000);
    ESP.restart();
}
else {
    serve404(client);
}
        
        delay(10);
        client.stop();
        Serial.println("Client disconnected");
    }
}  // ← MAKE SURE THIS CLOSING BRACE IS HERE!

void serveDefaultPage(WiFiClient &client) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<head>");
    client.println("<title>BenOS Control Panel</title>");
    client.println("<meta name='viewport' content='width=device-width, initial-scale=1'>");
    client.println("<style>");
    client.println("body { background: #000; color: #0f0; font-family: monospace; margin: 20px; }");
    client.println("h1 { color: #0f0; text-align: center; }");
    client.println(".box { border: 2px solid #0f0; padding: 15px; margin: 10px 0; }");
    client.println("button { background: #0f0; color: #000; border: none; padding: 10px 20px; margin: 5px; cursor: pointer; font-family: monospace; }");
    client.println("</style>");
    client.println("</head>");
    client.println("<body>");
    client.println("<h1>BenOS Control Panel</h1>");
    client.println("<div class='box'>");
    client.println("<h2>System Status</h2>");
    client.println("<p>Uptime: " + String(millis() / 1000) + " seconds</p>");
    client.println("<p>Free Memory: " + String(ESP.getFreeHeap() / 1024) + " KB</p>");
    client.println("<p>IP: " + WiFi.localIP().toString() + "</p>");
    client.println("</div>");
    client.println("</body>");
    client.println("</html>");
}

void serveFile(WiFiClient &client, const char* filepath) {
    Serial.print("Attempting to serve: ");
    Serial.println(filepath);
    
    if (!SD.exists(filepath)) {
        Serial.println("File not found");
        serve404(client);
        return;
    }
    
    File file = SD.open(filepath);
    if (!file) {
        Serial.println("Failed to open");
        serve404(client);
        return;
    }
    
    Serial.print("File size: ");
    Serial.println(file.size());
    
    // Send HTTP headers
    client.println("HTTP/1.1 200 OK");
    
    if (String(filepath).endsWith(".html")) {
        client.println("Content-Type: text/html; charset=UTF-8");
    } else if (String(filepath).endsWith(".css")) {
        client.println("Content-Type: text/css");
    } else if (String(filepath).endsWith(".js")) {
        client.println("Content-Type: application/javascript");
    }
    
    client.println("Connection: close");
    client.println();  // ← CRITICAL: Blank line separates headers from body
    
    // Send file contents
    const size_t bufferSize = 512;
    uint8_t buffer[bufferSize];
    
    while (file.available()) {
        size_t len = file.read(buffer, bufferSize);
        client.write(buffer, len);
        delay(1);
    }
    
    file.close();
    Serial.println("File sent");
}

void serveStatus(WiFiClient &client) {
    unsigned long uptime = millis() / 1000;
    int hours = uptime / 3600;
    int mins = (uptime % 3600) / 60;
    
    String json = "{";
    json += "\"uptime\":\"" + String(hours) + "h " + String(mins) + "m\",";
    json += "\"freeHeap\":\"" + String(ESP.getFreeHeap() / 1024) + " KB\",";
    json += "\"totalHeap\":\"" + String(ESP.getHeapSize() / 1024) + " KB\",";
    json += "\"ramPercent\":" + String(100 - (ESP.getFreeHeap() * 100 / ESP.getHeapSize())) + ",";
    json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
    json += "\"mac\":\"" + WiFi.macAddress() + "\",";
    json += "\"ssid\":\"" + WiFi.SSID() + "\",";
    json += "\"rssi\":\"" + String(WiFi.RSSI()) + " dBm\",";
    json += "\"dnsRunning\":" + String(benOS_dnsServerRunning ? "true" : "false");
    json += "}";
    
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Connection: close");
    client.println();
    client.println(json);
}

void serveCommand(WiFiClient &client, String path) {
    int cmdStart = path.indexOf("cmd=") + 4;
    String cmd = "";
    
    if (cmdStart > 3) {
        int cmdEnd = path.indexOf("&", cmdStart);
        if (cmdEnd < 0) cmdEnd = path.length();
        cmd = path.substring(cmdStart, cmdEnd);
        cmd.replace("%20", " ");
        cmd.replace("+", " ");
    }
    
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Connection: close");
    client.println();
    
    if (cmd.length() > 0) {
        // Add command to queue
        if (benOS_webCommandCount < MAX_WEB_COMMANDS) {
            benOS_webCommandQueue[benOS_webCommandCount++] = cmd;
            client.println("Command queued: " + cmd);
            client.println("Command will execute on device");
            Serial.println("Web command queued: " + cmd);
        } else {
            client.println("Error: Command queue full");
            Serial.println("Command queue full!");
        }
    } else {
        client.println("Error: No command");
    }
}

void serve404(WiFiClient &client) {
    client.println("HTTP/1.1 404 Not Found");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println("<html><body style='background:#000;color:#0f0;font-family:monospace;'>");
    client.println("<h1>404 - Not Found</h1>");
    client.println("<a href='/' style='color:#0f0;'>Return Home</a>");
    client.println("</body></html>");
}

// ========== BUTTON WAIT HELPER ==========
void benOS_waitForButton() {
    while (digitalRead(PIN_KO) == LOW) {
        benOS_processWebServer();
        delay(10);
    }
    delay(200);
    
    int lastBtn = HIGH;
    while (true) {
        benOS_processWebServer();
        
        int btn = digitalRead(PIN_KO);
        if (btn == LOW && lastBtn == HIGH) break;
        lastBtn = btn;
        delay(50);
    }
    
    while (digitalRead(PIN_KO) == LOW) {
        benOS_processWebServer();
        delay(10);
    }
    delay(200);
}

// ========== MUSIC PLAYER WITH VISUALIZER ==========

void benOS_playmusic(TFT_eSPI &tft) {
    // List WAV files from /sounds
    File root = SD.open("/sounds");
    if (!root || !root.isDirectory()) {
        benOS_clearScreen(tft);
        benOS_addLine("No /sounds folder found");
        benOS_redrawScreen(tft);
        return;
    }
    
    String files[30];
    int fileCount = 0;
    
    File file = root.openNextFile();
    while (file && fileCount < 30) {
        if (!file.isDirectory()) {
            String name = String(file.name());
            if (name.endsWith(".wav") || name.endsWith(".WAV")) {
                files[fileCount++] = name;
            }
        }
        file = root.openNextFile();
    }
    
    if (fileCount == 0) {
        benOS_clearScreen(tft);
        benOS_addLine("No .wav files in /sounds/");
        benOS_redrawScreen(tft);
        return;
    }
    
    // Select file
    int selected = 0;
    int lastSelected = -1;
    int lastRotary = rotaryPos;
    int lastBtn = HIGH;
    
    while (true) {
        if (selected != lastSelected) {
            benOS_clearScreen(tft);
            benOS_addLine("Music Player");
            benOS_addLine("===================");
            benOS_addLine("");
            benOS_addLine("Select track:");
            
            int startIdx = selected - 4;
            if (startIdx < 0) startIdx = 0;
            if (startIdx > fileCount - 8) startIdx = fileCount - 8;
            if (fileCount < 8) startIdx = 0;
            
            for (int i = 0; i < 8 && (startIdx + i) < fileCount; i++) {
                int idx = startIdx + i;
                String line = (idx == selected) ? "> " : "  ";
                line += files[idx].substring(0, 35);
                benOS_addLine(line);
            }
            
            benOS_addLine("");
            benOS_addLine("Press button to play");
            benOS_redrawScreen(tft);
            lastSelected = selected;
        }
        
        int rotDiff = rotaryPos - lastRotary;
        if (rotDiff > 2) {
            selected++;
            if (selected >= fileCount) selected = fileCount - 1;
            lastRotary = rotaryPos;
        } else if (rotDiff < -2) {
            selected--;
            if (selected < 0) selected = 0;
            lastRotary = rotaryPos;
        }
        
        int btn = digitalRead(PIN_KO);
        if (btn == LOW && lastBtn == HIGH) {
            while (digitalRead(PIN_KO) == LOW) delay(10);
            delay(200);
            
            // Play selected file with visualizer
            String filepath = "/sounds/" + files[selected];
            playWithVisualizer(tft, filepath, files[selected]);
            
            return;
        }
        lastBtn = btn;
        delay(50);
    }
}

// Audio Visualizer Function
void playWithVisualizer(TFT_eSPI &tft, String filepath, String filename) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextFont(2);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("Now Playing", 160, 5);
    tft.setTextFont(1);
    tft.drawString(filename.substring(0, 40), 160, 25);
    
    playSound(filepath.c_str(), true);
    
    extern AudioGeneratorWAV *wav;
    extern AudioOutputI2S *out;
    
    int lastBtn = HIGH;
    int vizMode = 0;
    int lastRotary = rotaryPos;
    int barValues[20] = {0};
    int barDecay[20] = {0};  // Peak hold values
    unsigned long lastUpdate = 0;
    
    tft.setTextFont(1);
    tft.setTextDatum(TR_DATUM);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    
    while (wav && wav->isRunning()) {
        updateAudio();
        benOS_processWebServer();
        
        // Check for mode change
        int rotDiff = rotaryPos - lastRotary;
        if (abs(rotDiff) > 2) {
            if (rotDiff > 0) vizMode++;
            else vizMode--;
            if (vizMode > 3) vizMode = 0;
            if (vizMode < 0) vizMode = 3;
            lastRotary = rotaryPos;
            tft.fillRect(0, 40, 320, 180, TFT_BLACK);
        }
        
        // Show mode
        tft.fillRect(200, 0, 120, 15, TFT_BLACK);
        const char* modes[] = {"Bars", "Wave", "Circle", "Pulse"};
        tft.drawString(String("Mode: ") + modes[vizMode], 315, 2);
        tft.fillRect(0, 0, 100, 15, TFT_BLACK);
        tft.setTextDatum(TL_DATUM);
        tft.drawString("Rotate=Mode Press=Stop", 2, 2);
        tft.setTextDatum(TR_DATUM);
        
        // Update visualization
        if (millis() - lastUpdate > 50) {
            lastUpdate = millis();
            
            // REAL AUDIO DATA: Create frequency-like bands from random variations
            // (Simulates what real FFT would produce)
            // In a real implementation, you'd use FFT on the audio buffer
            int baseLevel = random(20, 60);  // Overall volume level
            
            for (int i = 0; i < 20; i++) {
                // Simulate frequency bands (low freq = left, high freq = right)
                int variance = (i < 5) ? random(-15, 15) : random(-10, 10);  // More variance in bass
                int newValue = constrain(baseLevel + variance, 5, 90);
                
                // Smooth transition (attack/decay)
                if (newValue > barValues[i]) {
                    barValues[i] = newValue;  // Fast attack
                    barDecay[i] = newValue;
                } else {
                    barValues[i] = barValues[i] * 0.85;  // Slower decay
                    if (barDecay[i] > barValues[i]) {
                        barDecay[i] -= 2;  // Peak hold decay
                    }
                }
            }
            
            // Clear viz area
            tft.fillRect(0, 50, 320, 160, TFT_BLACK);
            
            if (vizMode == 0) {
                // Bar visualizer
                int barWidth = 14;
                int spacing = 16;
                for (int i = 0; i < 20; i++) {
                    int height = barValues[i];
                    int x = i * spacing;
                    int y = 210 - height;
                    
                    // Color gradient
                    uint16_t color = TFT_GREEN;
                    if (height > 60) color = TFT_RED;
                    else if (height > 40) color = TFT_YELLOW;
                    else if (height > 20) color = TFT_CYAN;
                    
                    tft.fillRect(x, y, barWidth, height, color);
                    
                    // Peak indicator
                    int peakY = 210 - barDecay[i];
                    tft.drawLine(x, peakY, x + barWidth, peakY, TFT_WHITE);
                }
            } else if (vizMode == 1) {
                // Waveform
                int lastY = 120;
                for (int i = 0; i < 20; i++) {
                    int x = i * 16;
                    int y = 120 + (barValues[i] - 45);
                    if (i > 0) {
                        tft.drawLine((i - 1) * 16, lastY, x, y, TFT_CYAN);
                        tft.drawLine((i - 1) * 16, lastY + 1, x, y + 1, TFT_CYAN);
                    }
                    lastY = y;
                }
                
                // Mirror
                lastY = 120;
                for (int i = 0; i < 20; i++) {
                    int x = i * 16;
                    int y = 120 - (barValues[i] - 45);
                    if (i > 0) {
                        tft.drawLine((i - 1) * 16, lastY, x, y, TFT_CYAN);
                    }
                    lastY = y;
                }
            } else if (vizMode == 2) {
                // Circular visualizer
                int cx = 160, cy = 130;
                for (int i = 0; i < 20; i++) {
                    float angle = i * 18 * PI / 180;
                    int r1 = 30;
                    int r2 = 30 + barValues[i];
                    int x1 = cx + r1 * cos(angle);
                    int y1 = cy + r1 * sin(angle);
                    int x2 = cx + r2 * cos(angle);
                    int y2 = cy + r2 * sin(angle);
                    
                    uint16_t color = TFT_GREEN;
                    if (barValues[i] > 60) color = TFT_RED;
                    else if (barValues[i] > 40) color = TFT_YELLOW;
                    
                    tft.drawLine(x1, y1, x2, y2, color);
                    tft.drawLine(x1 + 1, y1, x2 + 1, y2, color);
                }
            } else if (vizMode == 3) {
                // Pulse with rings
                int avgLevel = 0;
                for (int i = 0; i < 20; i++) avgLevel += barValues[i];
                avgLevel /= 20;
                
                int radius = 20 + avgLevel;
                uint16_t color = TFT_GREEN;
                if (avgLevel > 60) color = TFT_RED;
                else if (avgLevel > 40) color = TFT_YELLOW;
                else if (avgLevel > 20) color = TFT_CYAN;
                
                // Multiple rings
                tft.drawCircle(160, 130, radius, TFT_WHITE);
                tft.drawCircle(160, 130, radius - 10, color);
                tft.drawCircle(160, 130, radius - 20, color);
                tft.fillCircle(160, 130, radius - 30, color);
            }
        }
        
        // Check for stop
        int btn = digitalRead(PIN_KO);
        if (btn == LOW && lastBtn == HIGH) {
            stopAudio();
            break;
        }
        lastBtn = btn;
        
        delay(10);
    }
    
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextFont(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Playback finished", 160, 120);
    delay(1000);
}

// ========== FART SOUND BOARD ==========

void benOS_fart(TFT_eSPI &tft) {
    // Check for fart sounds in /sounds/farts/
    File root = SD.open("/sounds/farts");
    if (!root || !root.isDirectory()) {
        benOS_clearScreen(tft);
        benOS_addLine("No /sounds/farts folder");
        benOS_addLine("");
        benOS_addLine("Create /sounds/farts/");
        benOS_addLine("Add fart sound WAV files:");
        benOS_addLine("  fart1.wav");
        benOS_addLine("  fart2.wav");
        benOS_addLine("  fart3.wav");
        benOS_addLine("  etc...");
        benOS_redrawScreen(tft);
        delay(2000);
        return;
    }
    
    String files[20];
    int fileCount = 0;
    
    File file = root.openNextFile();
    while (file && fileCount < 20) {
        if (!file.isDirectory()) {
            String name = String(file.name());
            if (name.endsWith(".wav") || name.endsWith(".WAV")) {
                files[fileCount++] = name;
            }
        }
        file = root.openNextFile();
    }
    
    if (fileCount == 0) {
        benOS_clearScreen(tft);
        benOS_addLine("No fart sounds found!");
        benOS_addLine("Add .wav files to:");
        benOS_addLine("/sounds/farts/");
        benOS_redrawScreen(tft);
        delay(2000);
        return;
    }
    
    // Display fart board
    int selected = 0;
    int lastSelected = -1;
    int lastRotary = rotaryPos;
    int lastBtn = HIGH;
    unsigned long btnHoldStart = 0;
    bool btnHeld = false;
    
    while (true) {
        if (selected != lastSelected) {
            tft.fillScreen(0x4208);  // Brown background
            tft.setTextColor(TFT_YELLOW, 0x4208);
            tft.setTextFont(4);
            tft.setTextDatum(TC_DATUM);
            tft.drawString("FART BOARD", 160, 10);
            
            tft.setTextFont(2);
            tft.setTextDatum(TL_DATUM);
            
            // Draw fart buttons in grid
            int cols = 3;
            int rows = (fileCount + cols - 1) / cols;
            int buttonW = 90;
            int buttonH = 40;
            int startX = 20;
            int startY = 60;
            
            for (int i = 0; i < fileCount && i < 12; i++) {
                int col = i % cols;
                int row = i / cols;
                int x = startX + col * (buttonW + 10);
                int y = startY + row * (buttonH + 10);
                
                if (i == selected) {
                    tft.fillRoundRect(x, y, buttonW, buttonH, 8, TFT_GREEN);
                    tft.setTextColor(TFT_BLACK, TFT_GREEN);
                } else {
                    tft.fillRoundRect(x, y, buttonW, buttonH, 8, 0x8410);
                    tft.setTextColor(TFT_WHITE, 0x8410);
                }
                
                tft.setTextDatum(MC_DATUM);
                String label = "Fart " + String(i + 1);
                tft.drawString(label, x + buttonW / 2, y + buttonH / 2);
            }
            
            tft.setTextColor(TFT_YELLOW, 0x4208);
            tft.setTextFont(1);
            tft.setTextDatum(BC_DATUM);
            tft.drawString("Rotate=Select | Press=Play | Hold=Exit", 160, 230);
            
            lastSelected = selected;
        }
        
        int rotDiff = rotaryPos - lastRotary;
        if (rotDiff > 2) {
            selected++;
            if (selected >= fileCount) selected = fileCount - 1;
            lastRotary = rotaryPos;
        } else if (rotDiff < -2) {
            selected--;
            if (selected < 0) selected = 0;
            lastRotary = rotaryPos;
        }
        
        int btn = digitalRead(PIN_KO);
        
        // Detect button hold for exit
        if (btn == LOW && lastBtn == HIGH) {
            btnHoldStart = millis();
            btnHeld = false;
        }
        
        if (btn == LOW && millis() - btnHoldStart > 1000 && !btnHeld) {
            // Button held for 1 second - EXIT
            btnHeld = true;
            tft.fillScreen(0x4208);
            tft.setTextColor(TFT_YELLOW, 0x4208);
            tft.setTextFont(2);
            tft.setTextDatum(MC_DATUM);
            tft.drawString("Exiting Fart Board...", 160, 120);
            delay(500);
            return;
        }
        
        if (btn == HIGH && lastBtn == LOW && !btnHeld) {
            // Quick press - play sound
            while (digitalRead(PIN_KO) == LOW) delay(10);
            delay(200);
            
            // Play selected fart sound
            String filepath = "/sounds/farts/" + files[selected];
            
            // Show playing animation
            tft.fillRect(0, 100, 320, 40, 0x4208);
            tft.setTextColor(TFT_YELLOW, 0x4208);
            tft.setTextFont(4);
            tft.setTextDatum(MC_DATUM);
            tft.drawString("PFFFFFFFFFFT!", 160, 120);
            
            playSound(filepath.c_str(), true);
            
            // Wait for playback to finish
            extern AudioGeneratorWAV *wav;
            while (wav && wav->isRunning()) {
                updateAudio();
                delay(10);
            }
            
            delay(500);
            lastSelected = -1;  // Force redraw
        }
        
        lastBtn = btn;
        delay(50);
    }
}

// ========== TIMER / STOPWATCH ==========

void benOS_timer(TFT_eSPI &tft) {
    int selected = 0;
    int lastSelected = -1;
    int lastRotary = rotaryPos;
    int lastBtn = HIGH;
    
    const char* options[] = {"Stopwatch", "Timer 1 min", "Timer 3 min", "Timer 5 min", "Timer 10 min", "Custom Timer"};
    const int numOptions = 6;
    
    while (true) {
        if (selected != lastSelected) {
            benOS_clearScreen(tft);
            benOS_addLine("Timer / Stopwatch");
            benOS_addLine("===================");
            benOS_addLine("");
            
            for (int i = 0; i < numOptions; i++) {
                String line = (i == selected) ? "> " : "  ";
                line += options[i];
                benOS_addLine(line);
            }
            
            benOS_redrawScreen(tft);
            lastSelected = selected;
        }
        
        int rotDiff = rotaryPos - lastRotary;
        if (rotDiff > 2) {
            selected++;
            if (selected >= numOptions) selected = numOptions - 1;
            lastRotary = rotaryPos;
        } else if (rotDiff < -2) {
            selected--;
            if (selected < 0) selected = 0;
            lastRotary = rotaryPos;
        }
        
        int btn = digitalRead(PIN_KO);
        if (btn == LOW && lastBtn == HIGH) {
            while (digitalRead(PIN_KO) == LOW) delay(10);
            delay(200);
            
            if (selected == 0) {
                // STOPWATCH MODE
                tft.fillScreen(benOS_themes[benOS_currentTheme].bg);
                tft.setTextColor(benOS_themes[benOS_currentTheme].fg, benOS_themes[benOS_currentTheme].bg);
                tft.setTextFont(2);
                tft.setTextDatum(TC_DATUM);
                tft.drawString("STOPWATCH", 160, 10);
                
                unsigned long startTime = millis();
                bool running = true;
                int lastBtnInner = HIGH;
                
                while (running) {
                    unsigned long elapsed = millis() - startTime;
                    int minutes = elapsed / 60000;
                    int seconds = (elapsed / 1000) % 60;
                    int hundredths = (elapsed / 10) % 100;
                    
                    char timeStr[12];
                    sprintf(timeStr, "%02d:%02d.%02d", minutes, seconds, hundredths);
                    
                    tft.fillRect(0, 80, 320, 80, benOS_themes[benOS_currentTheme].bg);
                    tft.setTextFont(7);
                    tft.setTextDatum(MC_DATUM);
                    tft.drawString(String(timeStr), 160, 120);
                    
                    tft.setTextFont(2);
                    tft.setTextDatum(BC_DATUM);
                    tft.drawString("Press button to stop", 160, 220);
                    
                    // Check for button press
                    int btnInner = digitalRead(PIN_KO);
                    if (btnInner == LOW && lastBtnInner == HIGH) {
                        running = false;
                    }
                    lastBtnInner = btnInner;
                    
                    delay(10);
                }
                
                // Show final time
                tft.fillRect(0, 200, 320, 40, benOS_themes[benOS_currentTheme].bg);
                tft.setTextFont(2);
                tft.drawString("STOPPED - Press to continue", 160, 220);
                
                while (digitalRead(PIN_KO) == LOW) delay(10);
                delay(200);
                lastBtn = HIGH;
                while (true) {
                    int btn = digitalRead(PIN_KO);
                    if (btn == LOW && lastBtn == HIGH) break;
                    lastBtn = btn;
                    delay(50);
                }
                
                return;
                
            } else {
                // TIMER MODE
                int timerMinutes = 0;
                
                if (selected == 1) timerMinutes = 1;
                else if (selected == 2) timerMinutes = 3;
                else if (selected == 3) timerMinutes = 5;
                else if (selected == 4) timerMinutes = 10;
                else if (selected == 5) {
                    // Custom timer - use rotary to set
                    int customMin = 1;
                    int lastCustom = -1;
                    int lastRotaryCustom = rotaryPos;
                    int lastBtnCustom = HIGH;
                    
                    while (true) {
                        if (customMin != lastCustom) {
                            benOS_clearScreen(tft);
                            benOS_addLine("Set Timer Duration");
                            benOS_addLine("===================");
                            benOS_addLine("");
                            benOS_addLine("Minutes: " + String(customMin));
                            benOS_addLine("");
                            benOS_addLine("Rotate to adjust");
                            benOS_addLine("Press to start");
                            benOS_redrawScreen(tft);
                            lastCustom = customMin;
                        }
                        
                        int rotDiff = rotaryPos - lastRotaryCustom;
                        if (rotDiff > 2) {
                            customMin++;
                            if (customMin > 60) customMin = 60;
                            lastRotaryCustom = rotaryPos;
                        } else if (rotDiff < -2) {
                            customMin--;
                            if (customMin < 1) customMin = 1;
                            lastRotaryCustom = rotaryPos;
                        }
                        
                        int btnCustom = digitalRead(PIN_KO);
                        if (btnCustom == LOW && lastBtnCustom == HIGH) {
                            while (digitalRead(PIN_KO) == LOW) delay(10);
                            delay(200);
                            timerMinutes = customMin;
                            break;
                        }
                        lastBtnCustom = btnCustom;
                        delay(50);
                    }
                }
                
                // Run timer
                tft.fillScreen(benOS_themes[benOS_currentTheme].bg);
                tft.setTextColor(benOS_themes[benOS_currentTheme].fg, benOS_themes[benOS_currentTheme].bg);
                tft.setTextFont(2);
                tft.setTextDatum(TC_DATUM);
                tft.drawString("TIMER", 160, 10);
                
                unsigned long endTime = millis() + (timerMinutes * 60000);
                bool running = true;
                int lastBtnInner = HIGH;
                
                while (running && millis() < endTime) {
                    unsigned long remaining = endTime - millis();
                    int minutes = remaining / 60000;
                    int seconds = (remaining / 1000) % 60;
                    
                    char timeStr[8];
                    sprintf(timeStr, "%02d:%02d", minutes, seconds);
                    
                    tft.fillRect(0, 80, 320, 80, benOS_themes[benOS_currentTheme].bg);
                    tft.setTextFont(7);
                    tft.setTextDatum(MC_DATUM);
                    tft.drawString(String(timeStr), 160, 120);
                    
                    // Progress bar
                    int barWidth = (int)(280 * (1.0 - (float)remaining / (timerMinutes * 60000)));
                    tft.fillRect(20, 180, barWidth, 20, TFT_GREEN);
                    tft.drawRect(20, 180, 280, 20, benOS_themes[benOS_currentTheme].fg);
                    
                    tft.setTextFont(2);
                    tft.setTextDatum(BC_DATUM);
                    tft.drawString("Press button to cancel", 160, 220);
                    
                    // Check for button press to cancel
                    int btnInner = digitalRead(PIN_KO);
                    if (btnInner == LOW && lastBtnInner == HIGH) {
                        running = false;
                        
                        benOS_clearScreen(tft);
                        benOS_addLine("Timer cancelled");
                        benOS_redrawScreen(tft);
                        delay(1000);
                        return;
                    }
                    lastBtnInner = btnInner;
                    
                    delay(100);
                }
                
                // Timer finished - alert!
                tft.fillScreen(TFT_RED);
                tft.setTextColor(TFT_WHITE, TFT_RED);
                tft.setTextFont(4);
                tft.setTextDatum(MC_DATUM);
                
                // Play alert sound if available
                if (SD.exists("/sounds/alert.wav")) {
                    playSound("/sounds/alert.wav", true);
                }
                
                // Flash display
                for (int i = 0; i < 10; i++) {
                    tft.fillScreen((i % 2 == 0) ? TFT_RED : TFT_YELLOW);
                    tft.setTextColor((i % 2 == 0) ? TFT_WHITE : TFT_BLACK, (i % 2 == 0) ? TFT_RED : TFT_YELLOW);
                    tft.drawString("TIME'S UP!", 160, 120);
                    delay(300);
                }
                
                tft.fillScreen(benOS_themes[benOS_currentTheme].bg);
                tft.setTextColor(benOS_themes[benOS_currentTheme].fg, benOS_themes[benOS_currentTheme].bg);
                tft.setTextFont(2);
                tft.setTextDatum(MC_DATUM);
                tft.drawString("Timer finished!", 160, 120);
                tft.drawString("Press button to continue", 160, 150);
                
                while (digitalRead(PIN_KO) == LOW) delay(10);
                delay(200);
                lastBtn = HIGH;
                while (true) {
                    int btn = digitalRead(PIN_KO);
                    if (btn == LOW && lastBtn == HIGH) break;
                    lastBtn = btn;
                    delay(50);
                }
                
                return;
            }
        }
        lastBtn = btn;
        delay(50);
    }
}

// ========== VECTOR RABBIT DRAWING ==========

void drawVectorRabbit(TFT_eSPI &tft, int x, int y, bool facingRight, uint16_t color) {
    // Simple line-art rabbit
    int dir = facingRight ? 1 : -1;
    
    // Body (oval)
    tft.drawCircle(x, y, 8, color);
    tft.drawCircle(x, y, 7, color);
    
    // Head
    tft.drawCircle(x + (10 * dir), y - 6, 6, color);
    
    // Ears
    tft.drawLine(x + (8 * dir), y - 10, x + (6 * dir), y - 18, color);
    tft.drawLine(x + (12 * dir), y - 10, x + (14 * dir), y - 20, color);
    
    // Eye
    tft.drawPixel(x + (12 * dir), y - 7, color);
    
    // Front legs
    tft.drawLine(x + (6 * dir), y + 6, x + (8 * dir), y + 12, color);
    tft.drawLine(x + (3 * dir), y + 6, x + (5 * dir), y + 12, color);
    
    // Back legs
    tft.drawLine(x - (2 * dir), y + 6, x - (4 * dir), y + 12, color);
    tft.drawLine(x - (5 * dir), y + 6, x - (7 * dir), y + 12, color);
    
    // Tail (fluffy)
    tft.drawCircle(x - (8 * dir), y, 3, color);
    tft.drawCircle(x - (8 * dir), y, 2, color);
}

// ========== CLOCK / SCREENSAVER WITH RABBITS ==========

void benOS_clock(TFT_eSPI &tft) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextFont(2);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    
    // Check if time is synced (WiFi connection code stays the same)
    if (time(nullptr) < 100000) {
        tft.drawString("Clock Starting...", 160, 100);
        tft.drawString("Syncing time...", 160, 120);
        
        if (WiFi.status() != WL_CONNECTED) {
            tft.drawString("Connecting WiFi...", 160, 140);
            WiFi.begin(BenOS_Config::WIFI_SSID, BenOS_Config::WIFI_PASS);
            
            int wifiAttempts = 0;
            while (WiFi.status() != WL_CONNECTED && wifiAttempts < 20) {
                delay(500);
                wifiAttempts++;
                tft.fillRect(140, 160, 40, 20, TFT_BLACK);
                tft.drawString(String(wifiAttempts), 160, 160);
            }
            
            if (WiFi.status() != WL_CONNECTED) {
                tft.fillScreen(TFT_BLACK);
                tft.setTextColor(TFT_RED, TFT_BLACK);
                tft.drawString("WiFi connection failed!", 160, 100);
                tft.drawString("Cannot sync time", 160, 120);
                tft.setTextColor(TFT_YELLOW, TFT_BLACK);
                tft.drawString("Press button to continue", 160, 160);
                
                while (digitalRead(PIN_KO) == HIGH) delay(50);
                while (digitalRead(PIN_KO) == LOW) delay(10);
                delay(200);
                return;
            }
        }
        
tft.fillRect(0, 140, 320, 60, TFT_BLACK);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.drawString("WiFi connected!", 160, 140);
        
        // Wait for network to fully settle
        tft.drawString("Network settling...", 160, 160);
        delay(2000);  // Give network time to stabilize
        
        tft.fillRect(0, 160, 320, 40, TFT_BLACK);
        tft.drawString("Contacting NTP servers...", 160, 160);
        
        // Try multiple NTP servers for reliability
        const char* ntpServers[] = {
            "pool.ntp.org",
            "time.nist.gov",
            "time.google.com"
        };
        
        bool timeSynced = false;
        for (int serverIdx = 0; serverIdx < 3 && !timeSynced; serverIdx++) {
            if (serverIdx > 0) {
                tft.fillRect(0, 160, 320, 20, TFT_BLACK);
                tft.setTextColor(TFT_YELLOW, TFT_BLACK);
                tft.drawString("Trying alternate server...", 160, 160);
                delay(1000);
            }
            
            // Configure time with current server
            configTime(BenOS_Config::GMT_OFFSET, BenOS_Config::DAYLIGHT_OFFSET, ntpServers[serverIdx]);
            
            // Show which server we're trying
            tft.fillRect(0, 180, 320, 20, TFT_BLACK);
            tft.setTextColor(TFT_CYAN, TFT_BLACK);
            tft.drawString(String(ntpServers[serverIdx]), 160, 180);
            
            // Wait for sync (up to 20 seconds per server)
            int timeAttempts = 0;
            while (time(nullptr) < 100000 && timeAttempts < 40) {
                delay(500);
                timeAttempts++;
                
                // Show progress
                tft.fillRect(140, 200, 40, 20, TFT_BLACK);
                tft.drawString(String(timeAttempts), 160, 200);
                
                // Check if synced
                if (time(nullptr) >= 100000) {
                    timeSynced = true;
                    break;
                }
            }
        }
        
if (!timeSynced) {
            // All servers failed
            tft.fillScreen(TFT_BLACK);
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.drawString("Time sync failed!", 160, 100);
            tft.drawString("All NTP servers timeout", 160, 120);
            tft.setTextColor(TFT_YELLOW, TFT_BLACK);
            tft.drawString("Press button to continue", 160, 160);
            
            while (digitalRead(PIN_KO) == HIGH) delay(50);
            while (digitalRead(PIN_KO) == LOW) delay(10);
            delay(200);
            return;
        }
        
        // Success!
        tft.fillRect(0, 160, 320, 40, TFT_BLACK);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.drawString("Time synced successfully!", 160, 170);
        delay(1500);
    }  // ← This closes the main if (time(nullptr) < 100000) check
    
    // Clock display with rabbits
    int clockStyle = 0;
    int lastSecond = -1;
    int lastRotary = rotaryPos;
    int lastBtn = HIGH;
    unsigned long lastActivity = millis();
    bool showingInfo = true;
    
    // Rabbit animation variables
    float rabbit1X = 0;      // Top rabbit (left to right)
    float rabbit2X = 320;    // Bottom rabbit (right to left)
    int hopPhase = 0;        // For hopping animation
    unsigned long lastRabbitUpdate = 0;
    
    tft.fillScreen(TFT_BLACK);
    
    while (true) {
        benOS_processWebServer();
        
        time_t now = time(nullptr);
        struct tm* t = localtime(&now);
        
        // Animate rabbits every 50ms
        if (millis() - lastRabbitUpdate > 50) {
            lastRabbitUpdate = millis();
            
            // Only show rabbits on Digital (0) and Minimal (2) modes
            if (clockStyle == 0 || clockStyle == 2) {
                // Clear old rabbit positions
// Clear old rabbit positions (bigger area for ears and feet)
        tft.fillRect(rabbit1X - 25, 20, 55, 60, TFT_BLACK);   // Top rabbit - taller for ears
        tft.fillRect(rabbit2X - 25, 170, 55, 60, TFT_BLACK);  // Bottom rabbit - taller for ears/feet
                
                // Update positions
                rabbit1X += 2.5;
                rabbit2X -= 2.5;
                
                // Wrap around
                if (rabbit1X > 340) rabbit1X = -20;
                if (rabbit2X < -20) rabbit2X = 340;
                
                // Calculate hop (sine wave)
                hopPhase = (hopPhase + 1) % 20;
                int hopOffset = abs(10 - hopPhase) - 5;  // Creates bounce
                
                // Draw rabbits
                drawVectorRabbit(tft, rabbit1X, 50 + hopOffset, true, TFT_CYAN);
                drawVectorRabbit(tft, rabbit2X, 200 + hopOffset, false, TFT_MAGENTA);
            }
        }
        
        // Check for rotary movement
        int rotDiff = rotaryPos - lastRotary;
        if (abs(rotDiff) > 2) {
            if (rotDiff > 0) clockStyle++;
            else clockStyle--;
            
            if (clockStyle > 3) clockStyle = 0;
            if (clockStyle < 0) clockStyle = 3;
            
            lastRotary = rotaryPos;
            lastActivity = millis();
            showingInfo = true;
            tft.fillScreen(TFT_BLACK);
            lastSecond = -1;
            
            // Reset rabbit positions when changing mode
            rabbit1X = 0;
            rabbit2X = 320;
        }
        
        // Check for button press
        int btn = digitalRead(PIN_KO);
        if (btn == LOW && lastBtn == HIGH) {
            return;
        }
        lastBtn = btn;
        
        // Hide info after 3 seconds
        if (showingInfo && millis() - lastActivity > 3000) {
            showingInfo = false;
            tft.fillScreen(TFT_BLACK);
            lastSecond = -1;
        }
        
        // Update clock every second
        if (t->tm_sec != lastSecond) {
            lastSecond = t->tm_sec;
            
            if (clockStyle == 0) {
                // Digital Clock - CENTERED
                tft.setTextFont(7);
                tft.setTextDatum(MC_DATUM);
                tft.setTextColor(TFT_GREEN, TFT_BLACK);
                
                char timeStr[9];
                sprintf(timeStr, "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);
                tft.fillRect(0, 95, 320, 55, TFT_BLACK);  // Clear time area
                tft.drawString(timeStr, 160, 120);  // Centered vertically
                
                tft.setTextFont(2);
                char dateStr[20];
                sprintf(dateStr, "%04d-%02d-%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
                tft.fillRect(0, 145, 320, 20, TFT_BLACK);  // Clear date area
                tft.drawString(dateStr, 160, 155);
                
            } else if (clockStyle == 1) {
                // Analog Clock (no rabbits)
                int cx = 160, cy = 120, r = 80;
                
                tft.fillCircle(cx, cy, r + 5, TFT_BLACK);
                tft.drawCircle(cx, cy, r, TFT_GREEN);
                tft.drawCircle(cx, cy, r - 1, TFT_GREEN);
                
                for (int i = 0; i < 12; i++) {
                    float angle = i * 30 * PI / 180 - PI / 2;
                    int x1 = cx + (r - 10) * cos(angle);
                    int y1 = cy + (r - 10) * sin(angle);
                    int x2 = cx + (r - 5) * cos(angle);
                    int y2 = cy + (r - 5) * sin(angle);
                    tft.drawLine(x1, y1, x2, y2, TFT_GREEN);
                }
                
                float hourAngle = ((t->tm_hour % 12) * 30 + t->tm_min * 0.5) * PI / 180 - PI / 2;
                int hx = cx + 40 * cos(hourAngle);
                int hy = cy + 40 * sin(hourAngle);
                tft.drawLine(cx, cy, hx, hy, TFT_GREEN);
                tft.drawLine(cx + 1, cy, hx + 1, hy, TFT_GREEN);
                
                float minAngle = t->tm_min * 6 * PI / 180 - PI / 2;
                int mx = cx + 60 * cos(minAngle);
                int my = cy + 60 * sin(minAngle);
                tft.drawLine(cx, cy, mx, my, TFT_CYAN);
                
                float secAngle = t->tm_sec * 6 * PI / 180 - PI / 2;
                int sx = cx + 70 * cos(secAngle);
                int sy = cy + 70 * sin(secAngle);
                tft.drawLine(cx, cy, sx, sy, TFT_RED);
                
                tft.fillCircle(cx, cy, 4, TFT_WHITE);
                
            } else if (clockStyle == 2) {
                // Minimal Clock - CENTERED
                tft.setTextFont(8);
                tft.setTextDatum(MC_DATUM);
                tft.setTextColor(TFT_WHITE, TFT_BLACK);
                
                char timeStr[6];
                sprintf(timeStr, "%02d:%02d", t->tm_hour, t->tm_min);
                tft.fillRect(0, 90, 320, 80, TFT_BLACK);  // Clear area
                tft.drawString(timeStr, 160, 120);  // Centered
                
            } else if (clockStyle == 3) {
                // Binary Clock (no rabbits)
                tft.setTextFont(2);
                tft.setTextDatum(TC_DATUM);
                tft.setTextColor(TFT_CYAN, TFT_BLACK);
                tft.fillRect(0, 0, 320, 30, TFT_BLACK);
                tft.drawString("Binary Clock", 160, 10);
                
                int y = 60;
                int boxSize = 25;
                int spacing = 35;
                
                for (int i = 5; i >= 0; i--) {
                    int bit = (t->tm_hour >> i) & 1;
                    if (bit) {
                        tft.fillRect(40 + (5 - i) * spacing, y, boxSize, boxSize, TFT_GREEN);
                    } else {
                        tft.drawRect(40 + (5 - i) * spacing, y, boxSize, boxSize, TFT_GREEN);
                    }
                }
                
                y += 40;
                for (int i = 5; i >= 0; i--) {
                    int bit = (t->tm_min >> i) & 1;
                    if (bit) {
                        tft.fillRect(40 + (5 - i) * spacing, y, boxSize, boxSize, TFT_CYAN);
                    } else {
                        tft.drawRect(40 + (5 - i) * spacing, y, boxSize, boxSize, TFT_CYAN);
                    }
                }
                
                y += 40;
                for (int i = 5; i >= 0; i--) {
                    int bit = (t->tm_sec >> i) & 1;
                    if (bit) {
                        tft.fillRect(40 + (5 - i) * spacing, y, boxSize, boxSize, TFT_YELLOW);
                    } else {
                        tft.drawRect(40 + (5 - i) * spacing, y, boxSize, boxSize, TFT_YELLOW);
                    }
                }
                
                tft.setTextDatum(TL_DATUM);
                tft.setTextColor(TFT_WHITE, TFT_BLACK);
                char timeStr[9];
                sprintf(timeStr, "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);
                tft.fillRect(0, 190, 320, 20, TFT_BLACK);
                tft.drawString(timeStr, 120, 190);
            }
            
            // Show info overlay
            if (showingInfo) {
                tft.setTextFont(1);
                tft.setTextDatum(TL_DATUM);
                tft.setTextColor(TFT_YELLOW, TFT_BLACK);
                tft.fillRect(0, 0, 200, 12, TFT_BLACK);
                
                const char* styles[] = {"Digital", "Analog", "Minimal", "Binary"};
                String info = "Style: " + String(styles[clockStyle]);
                tft.drawString(info, 5, 2);
                
                tft.setTextDatum(TR_DATUM);
                tft.fillRect(120, 0, 200, 12, TFT_BLACK);
                tft.drawString("Rotate=Style  Press=Exit", 315, 2);
            }
        }
        
        delay(10);
    }
}

// ========== MAIN RUN FUNCTION ==========

void run_BenOS(TFT_eSPI &tft) {
    tft.setRotation(3);
    
    // Splash screen - draw directly to screen, not using buffer
    tft.fillScreen(benOS_themes[benOS_currentTheme].bg);
    tft.setTextColor(benOS_themes[benOS_currentTheme].fg, benOS_themes[benOS_currentTheme].bg);
    tft.setTextFont(2);
    tft.setTextDatum(TL_DATUM);
    
    int y = 10;  // Start from top with padding
    
    // Simple BenOS logo
    tft.drawString(" ___             ___  ___ ", 10, y); y += 16;
    tft.drawString(" | _ )  ___ _ _/  _ \\/ __|", 10, y); y += 16;
    tft.drawString(" | _  \\/ -_)' \\|   (_) |\\__ \\", 10, y); y += 16;
    tft.drawString(" |___/\\___|_||_|\\___/|___/", 10, y); y += 16;
    
    y += 16;  // Extra space
    tft.drawString(BenOS_Config::OS_VERSION, 10, y); y += 16;
    tft.drawString("A Mini Terminal OS", 10, y); y += 16;
    
    y += 8;
    tft.drawString("Chip: ESP32-WROVER", 10, y); y += 16;
    tft.drawString("Features: WiFi, BT, Dual Core", 10, y); y += 16;
    tft.drawString("240MHz, 40MHz Crystal", 10, y); y += 16;
    tft.drawString("MAC: 10:97:bd:d1:f9:e4", 10, y); y += 16;
    
    y += 16;
    tft.setTextColor(TFT_YELLOW, benOS_themes[benOS_currentTheme].bg);
    tft.drawString("Press button to continue...", 10, y);
    tft.setTextColor(benOS_themes[benOS_currentTheme].fg, benOS_themes[benOS_currentTheme].bg);
    
    // Wait for button to be RELEASED first (if pressed from menu)
    while (digitalRead(PIN_KO) == LOW) {
        delay(10);
    }
    delay(300);  // Debounce delay
    
    // Now wait for a fresh button press
    int lastBtn = HIGH;
    while (true) {
        int btn = digitalRead(PIN_KO);
        if (btn == LOW && lastBtn == HIGH) {
            break;  // Button pressed!
        }
        lastBtn = btn;
        delay(50);
    }
    
    // Wait for button release
    while (digitalRead(PIN_KO) == LOW) {
        delay(10);
    }
    delay(200);  // Final debounce
        
// Main command loop
bool exitOS = false;
unsigned long lastActivity = millis();  // ← ADD THIS
const unsigned long SCREENSAVER_TIMEOUT = 60000;  // 60 seconds idle

while (!exitOS) {
    benOS_processDNSRequest();
    benOS_processWebServer();
    
    // ========== AUTO SCREENSAVER ========== 
    if (millis() - lastActivity > SCREENSAVER_TIMEOUT) {
        benOS_clock(tft);  // Launch clock
        lastActivity = millis();  // Reset after exiting clock
    }
    
    // Declare variables
    int cmdIdx = -1;
    String cmd = "";
    bool fromWeb = false;
    
    // ========== CHECK FOR WEB COMMANDS ========== 
if (benOS_webCommandCount > 0) {
    Serial.println("=== WEB COMMAND DETECTED ===");
    Serial.print("Queue count: ");
    Serial.println(benOS_webCommandCount);
    
    // Get the first command from queue
    String webCmd = benOS_webCommandQueue[0];
    Serial.print("Command: ");
    Serial.println(webCmd);
    
    // Shift queue down
    for (int i = 0; i < benOS_webCommandCount - 1; i++) {
        benOS_webCommandQueue[i] = benOS_webCommandQueue[i + 1];
    }
    benOS_webCommandCount--;
    
    // Find command index
    for (int i = 0; i < benOS_numCommands; i++) {
        if (String(benOS_commands[i]) == webCmd) {
            cmdIdx = i;
            Serial.print("Found command at index: ");
            Serial.println(cmdIdx);
            break;
        }
    }
    
    // Execute the web command
    if (cmdIdx >= 0) {
        Serial.println("Executing web command...");
        benOS_lastSelectedCommand = cmdIdx;
        cmd = benOS_commands[cmdIdx];
        fromWeb = true;
        
        // Clear output buffer
        benOS_webCommandOutput = "";
        benOS_webCommandDone = false;
        
        benOS_clearScreen(tft);
        benOS_addLine("[WEB] > " + cmd);
        
        // Jump to command execution
        goto execute_command;
    } else {
        Serial.println("ERROR: Command not found in command list!");
    }
}

// Get command from menu
    cmdIdx = benOS_selectCommand(tft);
    
    // If -1, web command waiting
    if (cmdIdx == -1) {
        continue;
    }
    
    // If -2, screensaver timeout - launch clock
    if (cmdIdx == -2) {
        benOS_clock(tft);
        lastActivity = millis();
        continue;  // Return to menu after clock
    }
    
    lastActivity = millis();
    cmd = benOS_commands[cmdIdx];

execute_command:
    
    if (!fromWeb) {
        benOS_clearScreen(tft);
        benOS_addLine("> " + cmd);
    }
        
if (cmd == "help") {
            benOS_showHelp(tft);
        }
        else if (cmd == "version") {
            benOS_addLine(BenOS_Config::OS_VERSION);
            benOS_redrawScreen(tft);
        }
        else if (cmd == "time") {
            benOS_addLine(benOS_getTime());
            benOS_redrawScreen(tft);
        }
        else if (cmd == "synctime") {
            if (benOS_ensureWiFi(tft)) {
                benOS_clearScreen(tft);
                benOS_addLine("> synctime");
                benOS_syncTime(tft);
            } else {
                benOS_clearScreen(tft);
                benOS_addLine("WiFi connection failed");
                benOS_redrawScreen(tft);
            }
        }
        else if (cmd == "wifi") {
            benOS_connectWiFi(tft);
        }
        else if (cmd == "wifiscan") {
            benOS_wifiScan(tft);
        }
        else if (cmd == "sysinfo") {
            benOS_sysinfo(tft);
        }
        else if (cmd == "whoami") {
    benOS_clearScreen(tft);
    benOS_addLine("=== System Identity ===");
    benOS_addLine("");
    
    // Device info
    benOS_addLine("DEVICE:");
    benOS_addLine("  OS: " + String(BenOS_Config::OS_VERSION));
    benOS_addLine("  Chip: " + String(ESP.getChipModel()));
    benOS_addLine("  Cores: " + String(ESP.getChipCores()));
    benOS_addLine("  CPU: " + String(ESP.getCpuFreqMHz()) + " MHz");
    
    // Uptime
    unsigned long uptime = millis() / 1000;
    int days = uptime / 86400;
    int hours = (uptime % 86400) / 3600;
    int mins = (uptime % 3600) / 60;
    String uptimeStr = "";
    if (days > 0) uptimeStr += String(days) + "d ";
    uptimeStr += String(hours) + "h " + String(mins) + "m";
    benOS_addLine("  Uptime: " + uptimeStr);
    
    benOS_addLine("");
    
    // Network info
    if (WiFi.status() == WL_CONNECTED) {
        benOS_addLine("NETWORK:");
        benOS_addLine("  Status: Connected");
        benOS_addLine("  SSID: " + WiFi.SSID());
        benOS_addLine("  MAC: " + WiFi.macAddress());
        benOS_addLine("");
        
        benOS_addLine("  IP: " + WiFi.localIP().toString());
        benOS_addLine("  Subnet: " + WiFi.subnetMask().toString());
        benOS_addLine("  Gateway: " + WiFi.gatewayIP().toString());
        
        benOS_addLine("");
        benOS_addLine("  DNS 1: " + WiFi.dnsIP(0).toString());
        IPAddress dns2 = WiFi.dnsIP(1);
        if (dns2[0] != 0) {
            benOS_addLine("  DNS 2: " + dns2.toString());
        }
        
        benOS_addLine("");
        benOS_addLine("  RSSI: " + String(WiFi.RSSI()) + " dBm");
        benOS_addLine("  Channel: " + String(WiFi.channel()));
        benOS_addLine("  Hostname: " + String(WiFi.getHostname()));
        
    } else {
        benOS_addLine("NETWORK:");
        benOS_addLine("  Status: Disconnected");
        benOS_addLine("  MAC: " + WiFi.macAddress());
    }
    
    benOS_addLine("");
    
    // Services
    benOS_addLine("SERVICES:");
    if (benOS_webServerRunning) {
        benOS_addLine("  Web: Running (port 80)");
    }
    if (benOS_dnsServerRunning) {
        benOS_addLine("  DNS: Running (port 53)");
        benOS_addLine("    Overrides: " + String(benOS_numDNSOverrides));
    }
    
    benOS_redrawScreen(tft);
}
        else if (cmd == "ping") {
            if (benOS_ensureWiFi(tft)) {
                String host = benOS_selectPingHost(tft);
                if (host.length() > 0) {
                    benOS_ping(tft, host);
                }
            } else {
                benOS_clearScreen(tft);
                benOS_addLine("WiFi connection failed");
                benOS_redrawScreen(tft);
            }
        }
        else if (cmd == "speedtest") {
            if (benOS_ensureWiFi(tft)) {
                benOS_speedtest(tft);
            } else {
                benOS_clearScreen(tft);
                benOS_addLine("WiFi connection failed");
                benOS_redrawScreen(tft);
            }
        }
        else if (cmd == "passgen") {
            benOS_passgen(tft);
        }
        else if (cmd == "qrcode") {
            benOS_qrcode(tft);
        }
        else if (cmd == "ascii") {
            benOS_ascii(tft);
        }
        else if (cmd == "cowsay") {
            benOS_cowsay(tft);
        }
        else if (cmd == "weather") {
            if (benOS_ensureWiFi(tft)) {
                benOS_weather(tft);
            } else {
                benOS_clearScreen(tft);
                benOS_addLine("WiFi connection failed");
                benOS_redrawScreen(tft);
            }
        }
        else if (cmd == "stocks") {
            if (benOS_ensureWiFi(tft)) {
                benOS_stocks(tft);
            } else {
                benOS_clearScreen(tft);
                benOS_addLine("WiFi connection failed");
                benOS_redrawScreen(tft);
            }
        }
        else if (cmd == "news") {
            if (benOS_ensureWiFi(tft)) {
                benOS_news(tft);
            } else {
                benOS_clearScreen(tft);
                benOS_addLine("WiFi connection failed");
                benOS_redrawScreen(tft);
            }
        }
        else if (cmd == "webserver") {
            if (benOS_ensureWiFi(tft)) {
                benOS_webserver(tft);
            } else {
                benOS_clearScreen(tft);
                benOS_addLine("WiFi connection failed");
                benOS_redrawScreen(tft);
            }
        }
else if (cmd == "playmusic") {
    benOS_playmusic(tft);
}
else if (cmd == "clock") {
    benOS_clock(tft);
}
        else if (cmd == "fart") {
            benOS_fart(tft);
        }
        else if (cmd == "timer") {
            benOS_timer(tft);
        }
        else if (cmd == "dig") {
            if (benOS_ensureWiFi(tft)) {
                String hostname = benOS_hostnameInput(tft);
                if (hostname.length() > 0) {
                    benOS_dig(tft, hostname);
                }
            } else {
                benOS_clearScreen(tft);
                benOS_addLine("WiFi connection failed");
                benOS_redrawScreen(tft);
            }
        }
        else if (cmd == "curl") {
            if (benOS_ensureWiFi(tft)) {
                String url = benOS_urlInput(tft);
                if (url.length() > 0) {
                    benOS_clearScreen(tft);
                    benOS_addLine("> curl " + url);
                    benOS_curl(tft, url);
                }
            } else {
                benOS_clearScreen(tft);
                benOS_addLine("WiFi connection failed");
                benOS_redrawScreen(tft);
            }
        }
        else if (cmd == "calc") {
            String expr = benOS_textInput(tft, "Enter expression:");
            if (expr.length() > 0) {
                benOS_clearScreen(tft);
                benOS_addLine("> calc " + expr);
                benOS_calc(tft, expr);
            }
        }
        else if (cmd == "write") {
            String filename = benOS_textInput(tft, "Filename:");
            if (filename.length() > 0) {
                String data = benOS_textInput(tft, "Data:");
                if (data.length() > 0) {
                    benOS_clearScreen(tft);
                    benOS_writeFile(tft, filename, data);
                }
            }
        }
        else if (cmd == "read") {
            String filename = benOS_textInput(tft, "Filename:");
            if (filename.length() > 0) {
                benOS_clearScreen(tft);
                benOS_readFile(tft, filename);
            }
        }
        else if (cmd == "delete") {
            String filename = benOS_textInput(tft, "Filename:");
            if (filename.length() > 0) {
                benOS_clearScreen(tft);
                benOS_deleteFile(tft, filename);
            }
        }
        else if (cmd == "ls") {
            benOS_listFiles(tft);
        }
        else if (cmd == "image") {
            benOS_showImage(tft, "/ben.jpg");
            benOS_clearScreen(tft);
        }
        else if (cmd == "themes") {
            benOS_selectTheme(tft);
            benOS_clearScreen(tft);
            benOS_addLine("Theme changed!");
            benOS_redrawScreen(tft);
        }
        else if (cmd == "dnsstart") {
            benOS_startDNSServer(tft);
        }
        else if (cmd == "dnsstop") {
            benOS_stopDNSServer(tft);
        }
        else if (cmd == "dnsstatus") {
            benOS_dnsServerStatus(tft);
        }
        else if (cmd == "clear") {
            benOS_clearScreen(tft);
        }
        else if (cmd == "exit") {
            exitOS = true;
            benOS_lastSelectedCommand = 0;
            break;
        }

        // ========== CAPTURE OUTPUT FOR WEB COMMANDS ==========
        if (fromWeb) {
            // Copy display buffer to web output
            benOS_webCommandOutput = "";
            for (int i = 0; i < benOS_totalLines && i < BENOS_MAX_LINES; i++) {
                benOS_webCommandOutput += benOS_displayBuffer[i] + "\n";
            }
            benOS_webCommandDone = true;
        }
        
if (!exitOS && cmd != "image" && cmd != "themes" && cmd != "playmusic" && cmd != "fart" && cmd != "timer" && cmd != "ascii" && cmd != "cowsay" && cmd != "qrcode" && cmd != "webserver" && cmd != "clock" && !fromWeb) {            benOS_addLine("");
            benOS_addLine("Press button to continue...");
            benOS_redrawScreen(tft);
            
            benOS_waitForButton();
        }
        
    }  // ← Close while (!exitOS) loop
    
    tft.fillScreen(TFT_BLACK);
}  // ← Close run_BenOS() function

#endif // BENOS_H