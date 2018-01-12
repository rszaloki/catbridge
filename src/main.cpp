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

int ledPin = 16;

BLECharacteristic *pSSID_Characteristic;
BLECharacteristic *pPWD_Characteristic;

void WiFiEvent(WiFiEvent_t event)
{
    Serial.print(".");
    digitalWrite(ledPin,LOW);

    switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        wifi_connecting = false;
        wifi_connected = true;
        digitalWrite(ledPin,HIGH);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        if(wifi_connected) Serial.println("WiFi lost connection");
        wifi_connecting = false;
        wifi_connected = false;
        break;
    }
}

void setup() {
  Serial.begin(115200);

  Serial.printf("BLE device %s \n", BLE_NAME);
  Serial.printf("SSID characteristic %s \n", SSID_CHARACTERISTIC_UUID);
  Serial.printf("PWD characteristic %s \n", PWD_CHARACTERISTIC_UUID);

  pinMode(ledPin, OUTPUT);

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
  pSSID_Characteristic->setValue("ssid");
  pPWD_Characteristic->setValue("password");

  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();

  WiFi.onEvent(WiFiEvent);
}

void loop() {
  if(!wifi_connected && !wifi_connecting) {
    digitalWrite(ledPin,HIGH);
    Serial.println(pSSID_Characteristic->getValue().c_str());
    Serial.println(pPWD_Characteristic->getValue().c_str());
    Serial.println(WiFi.isConnected());
    WiFi.disconnect(true);
    WiFi.begin(
      pSSID_Characteristic->getValue().c_str(),
      pPWD_Characteristic->getValue().c_str());
    wifi_connecting = true;
  }
  delay(1000);
}