#include <SPI.h>
#include <Wire.h>


// Kalibrasi sensor (sesuaikan dengan pengukuran aktual)
const int DRY_VALUE = 3620;   // Nilai ADC ketika sensor di udara (kering)
const int WET_VALUE = 1680;   // Nilai ADC ketika sensor terendam air (basah)
const int SENSOR_PIN = 15;    // GPIO15 (ADC2 - hati-hati jika pakai WiFi)

int soilMoistureValue = 0;
int soilMoisturePercent = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Soil Moisture Sensor Calibration");
  Serial.println("-------------------------------");
  Serial.print("Dry Value (Air): "); Serial.println(DRY_VALUE);
  Serial.print("Wet Value (Water): "); Serial.println(WET_VALUE);
  Serial.println();
}

void loop() {
  // put your main code here, to run repeatedly:
  soilMoistureValue = analogRead(SENSOR_PIN);
  soilMoisturePercent = map(soilMoistureValue, DRY_VALUE, WET_VALUE, 0, 100);
  soilMoisturePercent = constrain(soilMoisturePercent, 0, 100);

  Serial.print("Raw ADC: ");
  Serial.print(soilMoistureValue);
  Serial.print(" | Moisture: ");
  Serial.print(soilMoisturePercent);
  Serial.println("%");
  delay(2500);
}
