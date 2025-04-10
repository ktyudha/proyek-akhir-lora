#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>

// LoRa
#define ss 5
#define rst 14
#define dio0 2

// Sensor SoilMoisture
const int DRY_VALUE = 3620;   // Nilai ADC ketika sensor di udara (kering)
const int WET_VALUE = 1680;   // Nilai ADC ketika sensor terendam air (basah)
const int SENSOR_PIN = 15;    // GPIO15 (ADC2 - hati-hati jika pakai WiFi)

// Sensor BMP280
Adafruit_BMP280 bmp; // I2C

void setupBmp280(){
  unsigned status;
    status = bmp.begin(0x76);
    
    if (!status) {
      Serial.println(F("Could not find a valid BMP280 sensor!"));
      while (1) delay(10);
    }

    /* Default settings from datasheet. */
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                    Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                    Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                    Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                    Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Initial bmp280
  setupBmp280();
  
  Serial.println("LoRa Sender");
  LoRa.setPins(ss, rst, dio0);

  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  // Match these parameters with RAK3172 configuration
  LoRa.setSyncWord(0x12);
  LoRa.setSpreadingFactor(7);       // SF7
  LoRa.setSignalBandwidth(125E3);   // 125 kHz
  LoRa.setCodingRate4(5);           // CR 4/5
  LoRa.setPreambleLength(8);        // Preamble 8
  LoRa.setTxPower(20);              // 20 dBm (adjust as needed)

  Serial.println("LoRa Initializing OK!");
}

int readSoilMoisturePercent() {
  int soilMoistureValue = analogRead(SENSOR_PIN);
  int soilMoisturePercent = map(soilMoistureValue, DRY_VALUE, WET_VALUE, 0, 100);
  
  return constrain(soilMoisturePercent, 0, 100);
}


void loop() {
  float temperature = bmp.readTemperature();      // 10-40°C (avoid extremes)
  // int humidity = random(30, 37);
  int humidity = readSoilMoisturePercent();
  float ph = random(35, 91) / 10.0;      // pH 3.5-9.0 (1 decimal)
  int nitrogen = random(0, 201);         // 0-200 ppm N
  int fosfor = random(0, 101);           // 0-100 ppm P
  int kalium = random(0, 501);           // 0-500 ppm K

  String payload = String(temperature) + ";" +  String(humidity) + ";" +  String(ph, 1) + ";" + String(nitrogen) + ";" +  String(fosfor) + ";" + String(kalium);

  Serial.print("Sending packet: ");
  Serial.println(payload);

  // send packet
  LoRa.beginPacket();
  LoRa.print(payload);
  LoRa.endPacket();

  delay(1000);
}

