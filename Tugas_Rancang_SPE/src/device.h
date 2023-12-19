#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_AHTX0.h>
#include <FastLED.h>
#include <firebase.h>
#include <icon.h>
#include <NTPClient.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
// #include <WiFiManager.h>

#define OLED_SW 14
#define LED_SW 12
#define LED_PIN 13
#define NUM_LEDS 37
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// WiFiManager wifiManager;
CRGB leds[NUM_LEDS];
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
Adafruit_AHTX0 aht;
sensors_event_t humidity, temperature;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
const char *ssid = "sogumi"; 
const char *password = "vincun123";  

int currentState = 0;
bool buttonStateLED = false;
bool lastButtonStateLED = false;
bool buttonStateOLED = false;
bool lastButtonStateOLED = false;
bool OLEDshow = false;
bool RGBshow = false;
bool allowFirebaseUpdate = true;
volatile bool rgbCycleFlag = false;
unsigned long lastAHT10data = 0;

volatile bool buttonPressed = false;
volatile unsigned long lastButtonPressTime = 0;
const unsigned long buttonDebounceInterval = 50;
