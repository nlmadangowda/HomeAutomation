#include "main.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>
#include "SPIFFS.h"

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLEServer* pServer = NULL;
BLEServiceMap BleService;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;

extern DevConfig g_dev_config;

int UpdateDevConfig(char*str){
  StaticJsonDocument<360> doc;
  DeserializationError error = deserializeJson(doc, str);
  if (error) {
    Serial.print("deserializeMsgPack() failed: ");
    Serial.println(error.f_str());
    return 0;
  }
  Serial.println("Updating Device Config");
  g_dev_config.confg_status = doc["confg_status"];
  const char * doc_mobile_num = doc["mobile_num"];
  const char * doc_thingspeak_apikey = doc["thingspeak_apikey"];
  g_dev_config.thingspeak_c_id = doc["thingspeak_c_id"];
  const char * doc_wifi_ssid = doc["wifi_ssid"];
  const char * doc_wifi_pass = doc["wifi_pass"];
  if(strlen(doc_mobile_num)<=sizeof(g_dev_config.mobile_num)){
    memcpy(&g_dev_config.mobile_num[0],doc_mobile_num,strlen(doc_mobile_num));
  }
  if(strlen(doc_thingspeak_apikey)<=sizeof(g_dev_config.thingspeak_apikey)){
    memcpy(&g_dev_config.thingspeak_apikey[0],doc_thingspeak_apikey,strlen(doc_mobile_num));
  }
  if(strlen(doc_wifi_ssid)<=sizeof(g_dev_config.wifi_ssid)){
    memcpy(&g_dev_config.wifi_ssid[0],doc_wifi_ssid,strlen(doc_wifi_ssid));
  }
  if(strlen(doc_wifi_pass)<=sizeof(g_dev_config.wifi_pass)){
    memcpy(&g_dev_config.wifi_pass[0],doc_wifi_pass,strlen(doc_wifi_pass));
  }
  return 1;
}

int UpdateDevConfigFile(DevConfig *config,size_t size){
  const uint8_t *buff = (const uint8_t *)config;
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return 0;
  }
  
  File file = SPIFFS.open("/config.json",FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file for reading");
    return 0;
  }

  size_t w_size = file.write(buff,size);
  if(w_size!=size){
    Serial.println("Failed to Update config file to device");
  }

  file.close();
  SPIFFS.end();
  return 0;
}

char g_config_data[360]={0};
static int ParseConfigJson(std::string str){
  if (str.length() > 0) {
    memset(g_config_data,0,sizeof(g_config_data));
    Serial.print("New value: ");
    for (int i = 0; i < str.length(); i++){
      g_config_data[i]=str[i];
    }
    Serial.println(g_config_data);
    Serial.println("*********");
    UpdateDevConfig(g_config_data);
    UpdateDevConfigFile(&g_dev_config,sizeof(g_dev_config));
    return 1;
  }
  return 0;
}

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Client Device Connected");
      BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer* pServer) {
      Serial.println("Client Device DisConnected");
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      ParseConfigJson(value);
    }
};

void StartBLE(){
  BLEDevice::init("ESP32");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );
  pCharacteristic->setCallbacks(new MyCallbacks());

  pCharacteristic->addDescriptor(new BLE2902());

  pCharacteristic->setValue("Hello World");

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

void BLETask(void *pvParameters){
  (void) pvParameters;
  StartBLE();
  while(1){
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
  }
}
