#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <WiFi.h>
#include <FS.h>
#include "SPIFFS.h"

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define BLE_NAME                  "catbridge"
#define SERVICE_UUID              "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define SSID_CHARACTERISTIC_UUID  "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define PWD_CHARACTERISTIC_UUID   "3e391f2f-e0da-41e1-b4d1-469aa606c490"
#define WIFI_FILE                 "/wifi.txt"


bool wifi_connected = false;
bool wifi_connecting = false;

BLECharacteristic *pSSID_Characteristic;
BLECharacteristic *pPWD_Characteristic;

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file || file.isDirectory()){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
}

void WiFiEvent(WiFiEvent_t event)
{
    wifi_connecting = false;

    switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        wifi_connected = true;
        writeFile(SPIFFS, WIFI_FILE, pSSID_Characteristic->getValue().c_str());
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        if(wifi_connected) Serial.println("WiFi lost connection");
        wifi_connected = false;
        break;
    }
}

void setup() {
  Serial.begin(115200);

  Serial.printf("BLE device %s \n", BLE_NAME);
  Serial.printf("SSID characteristic %s \n", SSID_CHARACTERISTIC_UUID);
  Serial.printf("PWD characteristic %s \n", PWD_CHARACTERISTIC_UUID);

  BLEDevice::init(BLE_NAME);
  BLEServer *pServer = BLEDevice::createServer();

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pSSID_Characteristic = pService->createCharacteristic(
                                         SSID_CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pPWD_Characteristic = pService->createCharacteristic(
                                         PWD_CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  if(!SPIFFS.begin()) {
    Serial.println("SPIFFS Mount Failed");
    pSSID_Characteristic->setValue("ssid");
    pPWD_Characteristic->setValue("password");
  } else {
    readFile(SPIFFS, WIFI_FILE);
    pSSID_Characteristic->setValue("ssid");
    pPWD_Characteristic->setValue("password");
  }

  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();

  WiFi.onEvent(WiFiEvent);
}

void loop() {
  delay(1000);
  if(!wifi_connected && !wifi_connecting) {
    Serial.print(".");
    WiFi.begin( 
      pSSID_Characteristic->getValue().c_str(), 
      pPWD_Characteristic->getValue().c_str()
    );
    wifi_connecting = true;
  }
}