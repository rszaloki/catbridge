#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <WiFi.h>


const char* ssid     = "your-ssid";
const char* password = "your-password";

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define BLE_NAME                  "catbridge"
#define SERVICE_UUID              "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define SSID_CHARACTERISTIC_UUID  "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define PWD_CHARACTERISTIC_UUID   "3e391f2f-e0da-41e1-b4d1-469aa606c490"


bool wifi_connected = false;
bool wifi_connecting = false;



void WiFiEvent(WiFiEvent_t event)
{
    Serial.printf("[WiFi-event] event: %d\n", event);
    wifi_connecting = false;

    switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        wifi_connected = true;
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("WiFi lost connection");
        wifi_connected = false;
        break;
    }
}


class SetSSID: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      strcpy(ssid, value.c_str());
    }
};

class SetPassword: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      strcpy(password, value.c_str());
    }
};

void setup() {
  Serial.begin(115200);

  Serial.printf("BLE device %s \n", BLE_NAME);
  Serial.printf("SSID characteristic %s \n", SSID_CHARACTERISTIC_UUID);
  Serial.printf("PWD characteristic %s \n", PWD_CHARACTERISTIC_UUID);

  BLEDevice::init(BLE_NAME);
  BLEServer *pServer = BLEDevice::createServer();

  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pSSID_Characteristic = pService->createCharacteristic(
                                         SSID_CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pSSID_Characteristic->setCallbacks(new SetSSID());

  BLECharacteristic *pPWD_Characteristic = pService->createCharacteristic(
                                         PWD_CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pPWD_Characteristic->setCallbacks(new SetPassword());

  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();


  WiFi.onEvent(WiFiEvent);

}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);
  if(!wifi_connected && !wifi_connecting) {
    Serial.printf("%s %s \n", ssid, password);
    WiFi.begin(ssid, password);
    wifi_connecting = true;
  }
}