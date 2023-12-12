#include <Arduino.h>
#include <FastLED.h>
#include <WiFi.h>
#include "firebase.h"
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <icon.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_SSD1306.h>

// WiFi credentials
const char* ssid = "fAnus";
const char* password = "makanterus";
void whiteMode();
void warmMode();
void rgbCycleMode();
void onFirebaseStream(FirebaseStream data);
void WifiConnect();
void readAHTSensor(void *pvParameters);
void updateAHTData();

// OLED configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Button configuration
#define OLED_SW 12 // Assuming the button is connected to pin 0
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
int buttonState = 0;
int lastButtonState = 0;
bool OLEDshow = false;

// Defines
#define LED_PIN 13   // Define the pin connected to the data input of the LED strip
#define NUM_LEDS 37  // Define the number of LEDs in the strip

CRGB leds[NUM_LEDS];
Adafruit_AHTX0 aht;
volatile bool rgbCycleFlag = false;
sensors_event_t humidity, temperature;
unsigned long lastAHT10data = 0;

void setup() {
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(115200);
  while (!Serial) {}
  Serial.println("ESP32 Startup");

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  Wire.begin();

  // Initialize AHT10 sensor
  if (!aht.begin()) {
    Serial.println(F("Couldn't find AHT10 sensor!"));
    for (;;) {}
  }

  // Initialize with the I2C OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0X3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

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
    delay(10);
  }
  if (Firebase.RTDB.getBool(&fbdo, "/white_mode") || 
      Firebase.RTDB.getBool(&fbdo, "/warm_mode") || 
      Firebase.RTDB.getBool(&fbdo, "/amethyst_mode") || 
      Firebase.RTDB.getBool(&fbdo, "/aquamarine_mode") || 
      Firebase.RTDB.getBool(&fbdo, "/crimson_mode") || 
      Firebase.RTDB.getBool(&fbdo, "/mint_mode") || 
      Firebase.RTDB.getBool(&fbdo, "/sunflower_mode")) {
    rgbCycleFlag = false;
    break;
  }
} while (rgbCycleFlag);
}


void readAHTSensor() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // Display temperature
  display.drawBitmap(30, 5, temperature_icon, 16, 16 ,1);
  display.setTextSize(3);
  display.setCursor(50,5);
  display.print(temperature.temperature, 0);
  display.print("");
  display.setTextSize(1);
  display.cp437(true);
  display.write(167);
  display.setTextSize(2);
  display.print("C");

  // Display humidity
  display.drawBitmap(30, 40, humidity_icon, 16, 16 ,1);
  display.setTextSize(3);
  display.setCursor(50, 40);
  display.print(humidity.relative_humidity, 0);
  display.print("%"); 

  display.display();
}

void showTime() {
  time_t currentTime = time(nullptr);
  struct tm *localTime = localtime(&currentTime);

  char formattedDay[20];
  strftime(formattedDay, sizeof(formattedDay), "%A", localTime);

  char formattedDate[20];
  strftime(formattedDate, sizeof(formattedDate), "%d %b %Y", localTime); // Format for day, day-month-year

  char formattedTime[20];
  strftime(formattedTime, sizeof(formattedTime), "%H:%M", localTime);

  // Serial.println(formattedDate);
  // Serial.println(formattedTime);

  // Determine whether it's morning or night
  bool isMorning = localTime->tm_hour >= 5 && localTime->tm_hour < 18;
  display.clearDisplay();

  if (isMorning) {
    display.drawBitmap(96, 1, morning_icon, 32, 32, 1);  // Replace with your morning icon
  } else {
    display.drawBitmap(96, 1, night_icon, 32, 32, 1);  // Replace with your night icon
  }

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);  // Text size for day
  int dayX = 5;  
  int dayY = 1;  

  display.setCursor(dayX, dayY);
  display.println(formattedDay);

  display.setTextSize(1);  // Text size for date
  int dateX = 5;  
  int dateY = 10;  

  display.setCursor(dateX, dateY);
  display.println(formattedDate);

  display.setTextSize(3);  // Text size for time
  int timeX = 5;
  int timeY = 32;

  // Print the formatted time on the OLED display
  display.setCursor(timeX, timeY);
  display.println(formattedTime);

  display.display();
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

//   if (modeState) {
//     // If the mode is turning on, determine which mode to activate
//     if (modePath == "/white_mode") {
//       Firebase.RTDB.setBool(&fbdo, "/warm_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/rgb_cycle_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/amethyst_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/aquamarine_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/crimson_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/mint_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/sunflower_mode", false);
//       whiteMode();
//     } else if (modePath == "/warm_mode") {
//   Firebase.RTDB.setBool(&fbdo, "/white_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/rgb_cycle_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/amethyst_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/aquamarine_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/crimson_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/mint_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/sunflower_mode", false);
//       warmMode();
//     } else if (modePath == "/amethyst_mode"){
//         Firebase.RTDB.setBool(&fbdo, "/white_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/rgb_cycle_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/warm_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/aquamarine_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/crimson_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/mint_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/sunflower_mode", false);
//     } else if (modePath == "/aquamarine_mode"){
//          Firebase.RTDB.setBool(&fbdo, "/white_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/rgb_cycle_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/warm_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/crimson_mode", false);
//       Firebase.RTDB .setBool(&fbdo, "/mint_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/sunflower_mode", false);
//        Firebase.RTDB.setBool(&fbdo, "/amethyst_mode", false);
//     } else if (modePath == "/crimson_mode"){
//      Firebase.RTDB.setBool(&fbdo, "/white_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/rgb_cycle_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/warm_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/mint_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/sunflower_mode", false);
//        Firebase.RTDB.setBool(&fbdo, "/amethyst_mode", false);
//         Firebase.RTDB.setBool(&fbdo, "/aquamarine_mode", false);
//     } else if (modePath == "/mint_mode"){
//         Firebase.RTDB.setBool(&fbdo, "/white_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/rgb_cycle_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/warm_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/sunflower_mode", false);
//        Firebase.RTDB.setBool(&fbdo, "/amethyst_mode", false);
//         Firebase.RTDB.setBool(&fbdo, "/aquamarine_mode", false);
//          Firebase.RTDB.setBool(&fbdo, "/crimson_mode", false);
//     } else if (modePath == "/rgb_cycle_mode"){
//       Firebase.RTDB.setBool(&fbdo, "/white_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/warm_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/sunflower_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/amethyst_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/aquamarine_mode", false);
//        Firebase.RTDB.setBool(&fbdo, "/crimson_mode", false);
//        Firebase.RTDB.setBool(&fbdo, "/mint_mode", false);
//     } else if (modePath == "/sunflower_mode"){
//         Firebase.RTDB.setBool(&fbdo, "/white_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/warm_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/amethyst_mode", false);
//       Firebase.RTDB.setBool(&fbdo, "/aquamarine_mode", false);
//        Firebase.RTDB.setBool(&fbdo, "/crimson_mode", false);
//        Firebase.RTDB.setBool(&fbdo, "/mint_mode", false);
//        Firebase.RTDB.setBool(&fbdo, "/rgb_cycle_mode", false);
//     }
//     // Repeat for other modes
//   } else {
//     // If the mode is turning off, turn off all LEDs
//     fill_solid(leds, NUM_LEDS, CRGB::Black);
//     FastLED.show();
//   }
// }

void loop() {
  unsigned long currentMillis = millis ();
  if (currentMillis - lastAHT10data >= 1000) {
    lastAHT10data - currentMillis;
      // Read sensor data
  aht.getEvent(&humidity, &temperature);
   Serial.print("Temperature: ");
  Serial.print(temperature.temperature, 1);
  Serial.println(" °C");
  Serial.print("Humidity: ");
  Serial.print(humidity.relative_humidity, 1);
  Serial.println("%");

  // Set the temperature and humidity values in Firebase
  Firebase.RTDB.setFloat(&fbdo, "sensors/temperature", temperature.temperature);
  Firebase.RTDB.setFloat(&fbdo, "sensors/humidity", humidity.relative_humidity);
  }
  
  buttonState = digitalRead(OLED_SW);
  if (buttonState != lastButtonState) {     // Button state has changed
    if (buttonState == LOW) {               // Button is pressed
      OLEDshow = !OLEDshow;
      delay(50);                           // Debounce delay
    }
    lastButtonState = buttonState;
  }

  if (OLEDshow) {
    readAHTSensor();
  } else {
    showTime();
  }
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
