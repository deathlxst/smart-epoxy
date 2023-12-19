#include <device.h>

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

void turnOffLEDs() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

void turnOnLEDs(){
  Serial.println("turning on LEDs");
  fill_solid(leds, NUM_LEDS, CRGB::White);
  FastLED.show();
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

void rgbCycleMode() {
  Serial.println("Entering RGB cycle mode"); 
  // Set the RGB cycle mode flag to true
  rgbCycleFlag = true;

  do {
    for (int i = 0; i < 256; ++i) {
      for (int j = 0; j < NUM_LEDS; ++j) {
        leds[j] = CHSV(i + (j*10), 255, 255);
      }
      FastLED.show();
      delay(10);
    }
    // // Check for conditions to interrupt the loop
if (Firebase.RTDB.getBool(&fbdo, "/")) {
    rgbCycleFlag = false;
    break;
  }
    
  } while (rgbCycleFlag);
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
    display.drawBitmap(96, 1, morning_icon, 32, 32, 1);
  } else {
    display.drawBitmap(96, 1, night_icon, 32, 32, 1);
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

void readAHTSensor() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  // display temperature
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

  // display humidity
  display.drawBitmap(30, 40, humidity_icon, 16, 16 ,1);
  display.setTextSize(3);
  display.setCursor(50, 40);
  display.print(humidity.relative_humidity, 0);
  display.print("%"); 

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

  if (modePath == "/turnOffLEDs") {
    Serial.println("In turn off condition");
      if (modeState) {
        Serial.println("turnOffLEDs called");
        turnOffLEDs();
      }
  } else if (modePath == "/turnOnLEDs") {
    Serial.println("In turn on condition");
    if (modeState){
      Serial.println("turnOnLEDs called");
      turnOnLEDs();
    }
  } else if (modePath == "/white_mode") {
    Serial.println("In white_mode condition");
    if (modeState) {
      Serial.println("whiteMode() called");
      whiteMode();
    } else {
      // If white_mode is turned off, turn off LEDs
      turnOffLEDs();
    }
  } else if (modePath == "/warm_mode") {
    Serial.println("In warm_mode condition");
    if (modeState) {
      Serial.println("warmMode() called");
      warmMode();
    } else {
      // If warm_mode is turned off, turn off LEDs
      turnOffLEDs();
    }
  } else if (modePath == "/rgb_cycle_mode") {
    Serial.println("In rgb_cycle_mode condition");
    if (modeState) {
      Serial.println("rgbCycleMode() called");
      rgbCycleMode();
    } else {
      // If rgb_cycle_mode is turned off, turn off LEDs
      turnOffLEDs();
    }
  } else if (modePath == "/amethyst_mode"){
    Serial.println("In amethyst_mode condition");
    if (modeState) {
      purpleMode();
    } else {
      turnOffLEDs();
    }
  } else if (modePath == "/aquamarine_mode"){
    Serial.println("in aquamarine_mode condition");
    if (modeState) {
      blueMode();
    } else {
      turnOffLEDs();
    }
  } else if (modePath == "/crimson_mode"){
    Serial.println("in crimson_mode condition");
    if (modeState) {
      redMode();
    } else {
      turnOffLEDs();
    }
  } else if (modePath == "/mint_mode"){
    Serial.println("in mint_mode condition");
    if (modeState) {
      mintMode();
    } else {
      turnOffLEDs();
    }
  } else if (modePath == "/sunflower_mode"){
    Serial.println("in sunflower_mode condition");
    if (modeState){
      sunflowerMode();
    } else {
      turnOffLEDs();
    }
  }
}
void setup() {
  Serial.begin(115200);
  WifiConnect(); 
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  Wire.begin();
  configTime(7 * 3600, 0, "pool.ntp.org");  // Set the UTC offset for Jakarta time and NTP server
  pinMode(OLED_SW, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  // Initialize with the I2C oled
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0X3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  // Initialize AHT10 sensor
  if (!aht.begin()) {
    Serial.println(F("Couldn't find AHT10 sensor!"));
    for (;;) {}
  }
  display.clearDisplay();
  Serial.println(" & Connecting to Firebase...");
  Firebase_Init("LED_mode");
  Serial.println("System ready.");
}
void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastAHT10data >= 1000) {
    lastAHT10data = currentMillis;
    // Read sensor data
    aht.getEvent(&humidity, &temperature);
       Serial.print("Temperature: ");
    Serial.print(temperature.temperature, 1);
    Serial.println(" °C");
    Serial.print("Humidity: ");
    Serial.print(humidity.relative_humidity, 1);
    Serial.println("%");
    // Set the temperature and humidity values in Firebase
    Firebase.RTDB.setFloat(&fbdo, "/sensors/temperature", temperature.temperature);
    Firebase.RTDB.setFloat(&fbdo, "/sensors/humidity", humidity.relative_humidity);
  }
  buttonStateOLED = digitalRead(OLED_SW);
  if (buttonStateOLED != lastButtonStateOLED) {     // Button state has changed
    if (buttonStateOLED == LOW) {               // Button is pressed
      OLEDshow = !OLEDshow;
      delay(50);                           // Debounce delay
    }
    lastButtonStateOLED = buttonStateOLED;
  }
  if (OLEDshow) {
    readAHTSensor();
  } else {
    showTime();
  }
 buttonStateLED = digitalRead(LED_SW);

  // Check if the button is pressed
  if (buttonStateLED == HIGH) {
    delay(50);  // Debounce delay

    // Increment the state
    currentState = (currentState + 1) % 2;

    // Call the corresponding void function based on the current state
    switch (currentState) {
      case 0:
        whiteMode();
        break;
      case 1:
        turnOffLEDs();
        break;
    }
    // Print the current state to the serial monitor
    Serial.print("Current State: ");
    Serial.println(currentState);
    // Wait for the button to be released before allowing another state change
    while (digitalRead(LED_SW) == HIGH) {
      delay(50);
    }
  }
}
