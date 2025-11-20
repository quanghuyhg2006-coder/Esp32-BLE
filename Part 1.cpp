#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#define SERVICE_UUID       "e5d25641-6d62-467e-8194-c2a1b6f703f6"
#define LED_CHAR_UUID      "127a319c-89be-41cb-bc70-5072119cbd75"

BLEServer *pServer = nullptr;
BLECharacteristic *pCharacteristic = nullptr;

bool deviceConnected = false;

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("BLE Connected");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("BLE Disconnected");
    pServer->startAdvertising();
  }
};

class MyCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string rx = pCharacteristic->getValue();
    if (rx.length() > 0) {
      Serial.print("Received: ");
      Serial.println(rx.c_str());
      if (rx == "ON") {
        digitalWrite(2, HIGH);
        Serial.println("LED ON");
      } else if (rx == "OFF") {
        digitalWrite(2, LOW);
        Serial.println("LED OFF");
      }
    }
  }
};

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);

  BLEDevice::init("ESP32-BLE");
  Serial.print("BLE MAC Address: ");
  Serial.println(BLEDevice::getAddress().toString().c_str());

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
      LED_CHAR_UUID,
      BLECharacteristic::PROPERTY_READ   |
      BLECharacteristic::PROPERTY_WRITE  |
      BLECharacteristic::PROPERTY_NOTIFY |
      BLECharacteristic::PROPERTY_INDICATE
  );

  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setCallbacks(new MyCallback());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->start();
  Serial.println("BLE Advertising started...");
}

void loop() {
  static unsigned long lastSend = 0;
  if (deviceConnected && millis() - lastSend > 1000) {
    lastSend = millis();
    pCharacteristic->setValue("Hello from ESP32!");
    pCharacteristic->notify();
    Serial.println("Notify sent");
  }
  delay(10);
}
