#include <SPI.h>
#include <LoRa.h>

#define ss 5
#define rst 14
#define dio0 2

void setup() {
  Serial.begin(115200);
  while (!Serial);

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

void loop() {
  int first = random(10, 100);   // Random between 10-99
  int second = random(10, 100);  // Random between 10-99
  int third = random(1, 20);     // Random between 1-19

  String payload = String(first) + ";" + String(second) + ";" + String(third);

  Serial.print("Sending packet: ");
  Serial.println(payload);

  // send packet
  LoRa.beginPacket();
  LoRa.print(payload);
  LoRa.endPacket();

  delay(1000);
}
