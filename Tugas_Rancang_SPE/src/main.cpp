#include <Arduino.h>
#include <FastLED.h>
#include <WiFi.h>
#include "firebase.h"
#include <Wire.h>
#include <Adafruit_AHTX0.h>

// WiFi credentials
const char* ssid = "Kokoro";
const char* password = "oranggila1220";
void whiteMode();
void warmMode();
void rgbCycleMode();
void onFirebaseStream(FirebaseStream data);
void WifiConnect();
void readAHTSensor(void *pvParameters);

// Defines
#define LED_PIN 4   // Define the pin connected to the data input of the LED strip
#define NUM_LEDS 72  // Define the number of LEDs in the strip

CRGB leds[NUM_LEDS];
Adafruit_AHTX0 aht;
volatile bool rgbCycleFlag = false;


void setup() {
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(115200);
  while (!Serial) {}
  Serial.println("ESP32 Startup");

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);

  WifiConnect();  // Call the function to connect to Wi-Fi
  Firebase_Init("select_mode");  // Replace with your Firebase stream path
}

void whiteMode() {
  Serial.println("Entering white mode");
  // Set all LEDs to white
  fill_solid(leds, NUM_LEDS, CRGB::White);
  FastLED.show();
}

void blueMode() { //aquamarine_mode
  Serial.println("Entering Blue mode");
  fill_solid(leds, NUM_LEDS, CRGB::Blue);
  FastLED.show();
}

void redMode() { //crimson_mode
  Serial.println("Entering Red mode");
  fill_solid(leds, NUM_LEDS, CRGB::Red);
  FastLED.show();
}

void purpleMode() { //amethyst_mode
  Serial.println("Entering Purple mode");
  fill_solid(leds, NUM_LEDS, CRGB::DarkViolet);
  FastLED.show();
}

void mintMode() { //green
Serial.println("Entering Mint mode");
fill_solid(leds, NUM_LEDS, CRGB::Green);
FastLED.show();
}

void sunflowerMode() {
  Serial.println("Entering Sunflower mode");
  fill_solid(leds, NUM_LEDS, CRGB::Yellow);
  FastLED.show();
}

void warmMode() {
  Serial.println("Entering warm mode");
  // Set all LEDs to a warm color (e.g., soft orange)
  fill_solid(leds, NUM_LEDS, CRGB(255, 255, 128));  // Adjust the RGB values for the desired warm color
  FastLED.show();
}

// void rgbCycleMode() {
//   Serial.println("Entering RGB cycle mode");

//   // Set the RGB cycle mode flag to true
//   rgbCycleFlag = true;

//   int i = 0;
//   while (rgbCycleFlag && i < 256) {
//     for (int j = 0; j < NUM_LEDS; ++j) {
//       leds[j] = CHSV((i + j * 2) % 256, 255, 255);
//     }
//     FastLED.show();
//     delay(10);  // Adjust the delay to control the speed of color cycling
//     i++;
//   }

//   // Clear the RGB cycle mode flag when the loop exits
//   rgbCycleFlag = false;
// }

void rgbCycleMode() {
  Serial.println("Entering RGB cycle mode");
  
  // Set the RGB cycle mode flag to true
  rgbCycleFlag = true;

  do {
    for (int i = 0; i < 256; ++i) {
      for (int j = 0; j < NUM_LEDS; ++j) {
        leds[j] = CHSV((i + j * 2) % 256, 255, 255);
      }
      FastLED.show();
      delay(10);  // Adjust the delay to control the speed of color cycling
    }
  } while (rgbCycleFlag);

  // Clear the RGB cycle mode flag when the loop exits
  rgbCycleFlag = false;
}


void onFirebaseStream(FirebaseStream data) {
  Serial.println("New data received:");
  Serial.println(data.dataPath());
  Serial.println(data.dataType());
  Serial.println(data.payload());

  // Parse the data path to determine the mode
  String modePath = data.dataPath();
  String dataPath = data.dataPath();

  // Parse the data payload to determine the mode state
bool modeState = strcmp(data.payload().c_str(), "true") == 0;

   Serial.print("Mode Path: ");
  Serial.println(modePath);
  Serial.print("Mode State: ");
  Serial.println(modeState);

  // Handling the mode changes
  if (dataPath.startsWith("/sensors")) {
    Serial.println("Changes in the /sensors path detected!");
    // Add your custom handling logic here
  }
  if (modePath == "/white_mode") {
     Serial.println("In white_mode condition");
    if (modeState) {
       Serial.println("whiteMode() called");
      whiteMode();
    } else {
      // If white_mode is turned off, turn off LEDs
       Serial.println("Turning off LEDs");
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      FastLED.show();
    }
  } else if (modePath == "/warm_mode") {
         Serial.println("In warm_mode condition");
    if (modeState) {
        Serial.println("warmMode() called");
      warmMode();
    } else {
      // If warm_mode is turned off, turn off LEDs
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      FastLED.show();
    }
  } else if (modePath == "/rgb_cycle_mode") {
     Serial.println("In rgb_cycle_mode condition");
    if (modeState) {
    Serial.println("rgbCycleMode() called");
      rgbCycleMode();
    } else {
      // If rgb_cycle_mode is turned off, turn off LEDs
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      FastLED.show();
    }
  } else if (modePath == "/amethyst_mode"){
    Serial.println("In amethyst_mode condition");
    if (modeState) {
      purpleMode();
    } else {
      fill_solid (leds, NUM_LEDS, CRGB::Black);
      FastLED.show();
    }
  } else if (modePath == "/aquamarine_mode"){
    Serial.println("in aquamarine_mode condition");
    if (modeState) {
      blueMode();
    } else {
      fill_solid (leds, NUM_LEDS, CRGB::Black);
      FastLED.show();
    }
  } else if (modePath == "/crimson_mode"){
    Serial.println("in crimson_mode condition");
    if (modeState) {
      redMode();
    } else {
      fill_solid (leds, NUM_LEDS, CRGB::Black);
      FastLED.show();
    }
  } else if (modePath == "/mint_mode"){
    Serial.println("in mint_mode condition");
    if (modeState) {
      mintMode();
    } else {
      fill_solid (leds, NUM_LEDS, CRGB::Black);
      FastLED.show();
    }
  } else if (modePath == "/sunflower_mode"){
    Serial.println("in sunflower_mode condition");
    if (modeState){
      sunflowerMode();
    } else {
      fill_solid (leds, NUM_LEDS, CRGB::Black);
      FastLED.show();
    }
  }
}

void loop() {
  // Check for conditions to switch to other modes and update rgbCycleFlag
  // For example, you can use Firebase or any other condition to set rgbCycleFlag to false
  
  // Example: If Firebase signals a mode change
  // Note: Update these conditions based on your actual boolean values
  bool switchToWhiteMode = Firebase.RTDB.getBool(&fbdo, "/white_mode"); // Replace with your condition
  bool switchToWarmMode = Firebase.RTDB.getBool(&fbdo, "/warm_mode"); // Replace with your condition
  bool switchToRGBCycleMode = Firebase.RTDB.getBool(&fbdo, "/rgb_cycle_mode");  // Replace with your condition

  // Check if other modes are switched from false to true
  if (switchToWhiteMode) {
        Serial.println("Switching to White Mode");
    whiteMode();
    rgbCycleFlag = false; // Turn off RGB cycle mode
  } else if (switchToWarmMode) {
        Serial.println("Switching to Warm Mode");
    warmMode();
    rgbCycleFlag = false; // Turn off RGB cycle mode
  } else if (switchToRGBCycleMode) {
        Serial.println("Switching to RGB Cycle Mode");
    rgbCycleMode();
  }

  delay(1000); // Add a longer delay
}


void WifiConnect() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  Serial.print("System connected with IP address: ");
  Serial.println(WiFi.localIP());
  Serial.printf("RSSI: %d\n", WiFi.RSSI());
}
