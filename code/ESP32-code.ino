//MASTER CODE

#define BLYNK_TEMPLATE_ID "TMPLM"
#define BLYNK_TEMPLATE_NAME "solar0"
#define BLYNK_AUTH_TOKEN "Xxac3g6"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <EEPROM.h>

char ssid[] = "USERNAME";
char pass[] = "PASSWORD";

#define LDR1_PIN 34
#define LDR2_PIN 35
#define LDR3_PIN 32

#define LED1_PIN 25
#define LED2_PIN 26
#define LED3_PIN 27

#define NUM_SAMPLES 5
#define LED_ON_DELAY 80
#define LED_OFF_DELAY 80
#define INDOOR_SCALE 1.2
#define EMA_ALPHA 0.2

// =====================================================
// CLEANING SETTINGS
// =====================================================
#define DUST_THRESHOLD 65.0
#define REQUIRED_HIGH_READINGS 6
#define CLEANING_COOLDOWN 600000UL   // 10 min

HardwareSerial mySerial(2);
BlynkTimer timer;

// VARIABLES
float ledCleanReference = 500.0;
float ledDustPercent = 0.0;
float filteredReflection = 0.0;

int highDustCounter = 0;
unsigned long lastCleaningTime = 0;

bool cleaningNow = false;

void setup() {
  Serial.begin(115200);
  EEPROM.begin(64);

  EEPROM.get(0, ledCleanReference);

  if (ledCleanReference < 50 || ledCleanReference > 3000)
    ledCleanReference = 500;

  pinMode(LDR1_PIN, INPUT);
  pinMode(LDR2_PIN, INPUT);
  pinMode(LDR3_PIN, INPUT);

  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);

  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  digitalWrite(LED3_PIN, LOW);

  // ESP32 <-> UNO Serial Communication
  // RX = GPIO16 TX = GPIO17
  mySerial.begin(9600, SERIAL_8N1, 16, 17);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(500L, checkDust);

  Serial.println("ESP32 Master Ready");
  Serial.print("Loaded Clean Reference: ");
  Serial.println(ledCleanReference);
}

void loop() {
  Blynk.run();
  timer.run();
}
//Blynk calibration button
BLYNK_WRITE(V20) {

  if (param.asInt() == 1 && !cleaningNow) {

    Serial.println("Calibration Started...");
    Blynk.virtualWrite(V1, "Calibrating...");

    filteredReflection = 0;
    highDustCounter = 0;

    calibrateCleanPanel();

    Serial.println("Calibration Complete");
    Blynk.virtualWrite(V1, "Calibration Done");
  }
}
//Main check
void checkDust() {

  // During cleaning -> stop dust calculation
  if (cleaningNow) {
    readUnoSerial();
    return;
  }

  float reflectionValue = measureReflection();

  // Ignore false/noise readings
  if (reflectionValue < 5) {
    Serial.println("Ignoring false reading...");
    return;
  }

  // EMA smoothing
  if (filteredReflection == 0.0)
    filteredReflection = reflectionValue;

  filteredReflection =
      EMA_ALPHA * reflectionValue +
      (1.0 - EMA_ALPHA) * filteredReflection;

  // Dust %
  ledDustPercent =
      ((ledCleanReference - filteredReflection) /
       ledCleanReference) * 100.0;

  ledDustPercent = constrain(ledDustPercent, 0, 100);

  Serial.print("Reflection: ");
  Serial.print(filteredReflection, 2);
  Serial.print(" | Dust %: ");
  Serial.println(ledDustPercent, 2);

  Blynk.virtualWrite(V2, ledDustPercent);

  // Auto drift correction when nearly clean
  if (ledDustPercent < 5.0) {
    ledCleanReference =
      (0.995 * ledCleanReference) +
      (0.005 * filteredReflection);
  }

  // Cooldown protection
  if (millis() - lastCleaningTime < CLEANING_COOLDOWN)
    return;

  // Stable dust verification
  if (ledDustPercent >= DUST_THRESHOLD)
    highDustCounter++;
  else
    highDustCounter = 0;

  // Trigger only after stable readings
  if (highDustCounter >= REQUIRED_HIGH_READINGS) {
    highDustCounter = 0;
    startCleaningCycle();
  }

  readUnoSerial();
}

//Start Cleaning Cycle
void startCleaningCycle() {

  cleaningNow = true;
  lastCleaningTime = millis();

  Serial.println("Dust Stable -> Sending START to UNO");

  // Turn OFF LEDs during cleaning
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  digitalWrite(LED3_PIN, LOW);

  mySerial.println("START");

  Blynk.virtualWrite(V1, "Cleaning Cycle Running...");
}
//UNO Response
void readUnoSerial() {

  while (mySerial.available()) {

    String response = mySerial.readStringUntil('\n');
    response.trim();

    if (response.startsWith("DIST:")) {
      int distance = response.substring(5).toInt();
      Blynk.virtualWrite(V4, distance);
    }

    if (response == "DONE") {

      Serial.println("UNO Cleaning Cycle Finished");

      cleaningNow = false;
      filteredReflection = 0;

      Blynk.virtualWrite(V1, "Panel Clean");
    }
  }
}
//Calibration Function
void calibrateCleanPanel() {

  float total = 0;

  Serial.println("Keep panel CLEAN and room lights ON");

  for (int i = 0; i < 20; i++) {

    float value = measureReflection();
    total += value;

    Serial.print("Calibration Sample ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(value);

    delay(200);
  }

  ledCleanReference = total / 20.0;

  if (ledCleanReference < 1.0)
    ledCleanReference = 10.0;

  EEPROM.put(0, ledCleanReference);
  EEPROM.commit();

  Serial.print("New Clean Reference Saved: ");
  Serial.println(ledCleanReference, 2);
}
//Reflection Measure
float measureReflection() {

  float totalReflection = 0;

  for (int i = 0; i < NUM_SAMPLES; i++) {

    // LEDs OFF -> ambient
    digitalWrite(LED1_PIN, LOW);
    digitalWrite(LED2_PIN, LOW);
    digitalWrite(LED3_PIN, LOW);
    delay(LED_OFF_DELAY);

    float ambient =
      (analogRead(LDR1_PIN) +
       analogRead(LDR2_PIN) +
       analogRead(LDR3_PIN)) / 3.0;

    // LEDs ON -> reflection
    digitalWrite(LED1_PIN, HIGH);
    digitalWrite(LED2_PIN, HIGH);
    digitalWrite(LED3_PIN, HIGH);
    delay(LED_ON_DELAY);

    float ledValue =
      (analogRead(LDR1_PIN) +
       analogRead(LDR2_PIN) +
       analogRead(LDR3_PIN)) / 3.0;

    float reflection = ledValue - ambient;

    if (reflection < 0)
      reflection = 0;

    totalReflection += reflection * INDOOR_SCALE;
  }
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  digitalWrite(LED3_PIN, LOW);

  return totalReflection / NUM_SAMPLES;
}
