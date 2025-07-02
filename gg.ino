#include <Wire.h>
#include <MAX30105.h>
#include "heartRate.h"
#include <U8g2lib.h>

#define GREEN_LED_PIN 7
#define RED_LED_PIN 8

U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0);
MAX30105 particleSensor;

const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute;
int beatAvg;

void setup() {
  Serial.begin(9600);

  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);

  display.begin();
  display.clearBuffer();
  display.setFont(u8g2_font_ncenB08_tr);
  display.drawStr(10, 20, "Initializing...");
  display.sendBuffer();

  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
    Serial.println("MAX30102 not found");
    while (1);
  }

  particleSensor.setup(); 
  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeGreen(0);
}

void loop() {
  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue)) {
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);
    if (beatsPerMinute < 255 && beatsPerMinute > 20) {
      rates[rateSpot++] = (byte)beatsPerMinute;
      rateSpot %= RATE_SIZE;

      beatAvg = 0;
      for (byte x = 0; x < RATE_SIZE; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }

  display.clearBuffer();
  display.setFont(u8g2_font_ncenB14_tr);
  display.setCursor(0, 24);
  display.print("BPM: ");
  display.print(beatAvg);

  // Check BPM and LED(s)
  if (beatAvg < 50 || beatAvg > 120) {
    display.setFont(u8g2_font_6x10_tf);
    display.setCursor(0, 60);
    display.print("Arrhythmia detected!");
    digitalWrite(GREEN_LED_PIN, LOW); // Close Green
    digitalWrite(RED_LED_PIN, HIGH); // Open Red
  } else {
    digitalWrite(GREEN_LED_PIN, HIGH); // Open Green
    digitalWrite(RED_LED_PIN, LOW); // Close Red
  }

  display.sendBuffer();
  delay(100);
}
