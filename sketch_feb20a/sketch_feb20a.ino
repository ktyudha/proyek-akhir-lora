#include <ArduinoJson.h>
#define OTAA_PERIOD (1000)
#define OTAA_BAND (RAK_REGION_US915)
#define OTAA_DEVEUI \
  { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x9F, 0x6A, 0x7B }  // node-a US915
#define OTAA_APPEUI \
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
#define OTAA_APPKEY \
  { 0x4E, 0xE7, 0x84, 0x5F, 0xA0, 0xA5, 0xBA, 0x6D, 0x81, 0x38, 0x92, 0x61, 0xA7, 0x14, 0x0E, 0x5B }

//   #define OTAA_DEVEUI \
//   { 0x7E, 0xDF, 0x1C, 0x17, 0xA8, 0xEA, 0xC5, 0x40 }  // node-a US915
// #define OTAA_APPEUI \
//   { 0x60, 0x9E, 0x1B, 0x2C, 0x8C, 0x5F, 0xE9, 0x3A }
// #define OTAA_APPKEY \
//   { 0x4E, 0x3F, 0x19, 0x87, 0x35, 0x69, 0x06, 0xE1, 0xBD, 0x36, 0x1A, 0xCE, 0x9E, 0x7A, 0xF7, 0x93 }

uint8_t collected_data[64] = { 0 };

double upSnr;
double upRssi;
String recAt;
String sendAt;

void recvCallback(SERVICE_LORA_RECEIVE_T *data) {
  String jsonString;
  if (data->BufferSize > 0) {
    Serial.println("Something received!");

    for (int i = 0; i < data->BufferSize; i++) {
      Serial.write(data->Buffer[i]);
      jsonString += (char)data->Buffer[i];
    }

    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, jsonString);

    upRssi = doc["rssi"];
    upSnr = doc["snr"];
    recAt = doc["received_at"].as<String>();

    Serial.println("");

    char buffer[256];
    sprintf(buffer, "/*%f,%f,%d,%d,%s*/", upRssi, upSnr, api.lorawan.rssi.get(), api.lorawan.snr.get(), recAt.c_str());
    Serial.println("Serial Studio");
    Serial.println(buffer);

    Serial.println(" ");

    Serial.println("Time Slot 1");
    Serial.print("Node A -> Gateway : ");
    Serial.print(" RSSI ");
    Serial.print(upRssi);
    Serial.print(" SNR ");
    Serial.print(upSnr);
    Serial.print(" Received at ");
    Serial.println(recAt);

    Serial.println("Time Slot 2");
    Serial.print("Node A <- Gateway : ");
    Serial.print(" RSSI ");
    Serial.print(api.lorawan.rssi.get());
    Serial.print(", SNR ");
    Serial.print(api.lorawan.snr.get());
    Serial.println("");
    Serial.print("\r\n");
  }
}

void joinCallback(int32_t status) {
  Serial.printf("Join status: %d\r\n", status);
}

void sendCallback(int32_t status) {
  if (status == RAK_LORAMAC_STATUS_OK) {
    Serial.println("Successfully sent");
  } else {
    Serial.println("Sending failed");
  }
}

void setup() {
  Serial.begin(115200, RAK_AT_MODE);
  delay(2000);

  Serial.println("RAKwireless LoRaWan OTAA Example");
  Serial.println("------------------------------------------------------");

  if (api.lorawan.nwm.get() != 1) {
    Serial.printf("Set Node device work mode %s\r\n",
                  api.lorawan.nwm.set() ? "Success" : "Fail");
    api.system.reboot();
  }

  // OTAA Device EUI MSB first
  uint8_t node_device_eui[8] = OTAA_DEVEUI;
  // OTAA Application EUI MSB first
  uint8_t node_app_eui[8] = OTAA_APPEUI;
  // OTAA Application Key MSB first
  uint8_t node_app_key[16] = OTAA_APPKEY;

  if (!api.lorawan.appeui.set(node_app_eui, 8)) {
    Serial.printf("LoRaWan OTAA - set application EUI is incorrect! \r\n");
    return;
  }
  if (!api.lorawan.appkey.set(node_app_key, 16)) {
    Serial.printf("LoRaWan OTAA - set application key is incorrect! \r\n");
    return;
  }
  if (!api.lorawan.deui.set(node_device_eui, 8)) {
    Serial.printf("LoRaWan OTAA - set device EUI is incorrect! \r\n");
    return;
  }

  if (!api.lorawan.band.set(OTAA_BAND)) {
    Serial.printf("LoRaWan OTAA - set band is incorrect! \r\n");
    return;
  }
  if (!api.lorawan.deviceClass.set(RAK_LORA_CLASS_A)) {
    Serial.printf("LoRaWan OTAA - set device class is incorrect! \r\n");
    return;
  }
  if (!api.lorawan.njm.set(RAK_LORA_OTAA))
  {
    Serial.printf("LoRaWan OTAA - set network join mode is incorrect! \r\n");
    return;
  }
  if (!api.lorawan.rx1dl.set(1000)) {
    Serial.printf("set RX1 delay failed! \r\n");
    return;
  }
  if (!api.lorawan.join())
  {
    Serial.printf("LoRaWan OTAA - join fail! \r\n");
    return;
  }
  
  /** Wait for Join success */
  while (api.lorawan.njs.get() == 0) {
    Serial.println("Wait for LoRaWAN join...");
    api.lorawan.join();
    delay(10000);
  }

  if (!api.lorawan.adr.set(true)) {
    Serial.printf("LoRaWan OTAA - set adaptive data rate is incorrect! \r\n");
    return;
  }
  if (!api.lorawan.rety.set(1)) {
    Serial.printf("LoRaWan OTAA - set retry times is incorrect! \r\n");
    return;
  }
  if (!api.lorawan.cfm.set(1)) {
    Serial.printf("LoRaWan OTAA - set confirm mode is incorrect! \r\n");
    return;
  }

  /** Check LoRaWan Status*/
  Serial.printf("Duty cycle is %s\r\n", api.lorawan.dcs.get() ? "ON" : "OFF");           
  Serial.printf("Packet is %s\r\n", api.lorawan.cfm.get() ? "CONFIRMED" : "UNCONFIRMED"); 
  uint8_t assigned_dev_addr[4] = { 0 };
  api.lorawan.daddr.get(assigned_dev_addr, 4);
  Serial.printf("Device Address is %02X%02X%02X%02X\r\n", assigned_dev_addr[0], assigned_dev_addr[1], assigned_dev_addr[2], assigned_dev_addr[3]);  // Check Device Address
  Serial.printf("Uplink period is %ums\r\n", OTAA_PERIOD);
  Serial.printf("Rx1 delay period is %ums\r\n", api.lorawan.rx1dl.get());
  Serial.println("");
  api.lorawan.registerRecvCallback(recvCallback);
  api.lorawan.registerJoinCallback(joinCallback);
  api.lorawan.registerSendCallback(sendCallback);
}

void uplink_routine() {
  /** Payload of Uplink */
  uint8_t data_len = 0;
  collected_data[data_len++] = (uint8_t)'p';
  collected_data[data_len++] = (uint8_t)'i';
  collected_data[data_len++] = (uint8_t)'n';
  collected_data[data_len++] = (uint8_t)'g';

  Serial.println("Data Packet:");
  for (int i = 0; i < data_len; i++) {
    Serial.printf("0x%02X ", collected_data[i]);
  }
  Serial.println("");

  /** Send the data package */
  if (api.lorawan.send(data_len, (uint8_t *)&collected_data, 2, true, 1)) {
    Serial.println("Sending is requested");
  } else {
    Serial.println("Sending failed");
  }
}

void loop() {
  RAK_LORA_chan_rssi chan_arssi;
  uplink_routine();
  delay(OTAA_PERIOD);
}