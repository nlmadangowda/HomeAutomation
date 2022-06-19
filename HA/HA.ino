#include <Arduino.h>
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>
#include "SPIFFS.h"
#include "NewPing.h"

#define TINY_GSM_MODEM_SIM800      // Modem is SIM800
#define TINY_GSM_RX_BUFFER   1024  // Set RX buffer to 1Kb


#include "SinricPro.h"
#include "SinricProSwitch.h"
#include "SinricProTemperaturesensor.h"
#include "SinricProLight.h"
#include "SinricProFanUS.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <TinyGsmClient.h>


#include <Adafruit_Sensor.h>
#include "DHT.h"
#include <BH1750.h>
#include <Wire.h>
#include <ACROBOTIC_SSD1306.h>

#define DS18B20PIN 16


#define MODEM_TX             27
#define MODEM_RX             26

#define TRIGGER_PIN 5
#define ECHO_PIN 18

#define DHT_PIN_DI 2

#define SerialAT  Serial1




//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701


#define MAX_DISTANCE 400  


long duration;
float distanceCm;
float distanceInch;
const char simPIN[]   = "";


#define DHTTYPE    DHT11


#define WIFI_SSID         "Royalicon504"
#define WIFI_PASS         "Royalicon504"

#define APP_KEY           "dd7a77bc-8938-478b-95f1-3b04a1583663"      // Should look like "de0bxxxx-1x3x-4x3x-ax2x-5dabxxxxxxxx"
#define APP_SECRET        "6719d0f8-b1fa-48f7-b7cf-c0549f0e5d85-37abe8d1-9aad-4886-af4a-58d690516f5f"   // Should look like "5f36xxxx-x3x7-4x3x-xexe-e86724a9xxxx-4c4axxxx-3x3x-x5xe-x9x3-333d65xxxxxx"   // Should look like "5f36xxxx-x3x7-4x3x-xexe-e86724a9xxxx-4c4axxxx-3x3x-x5xe-x9x3-333d65xxxxxx"

#define RELAY_1 25
#define RELAY_2 33
#define RELAY_3 32
#define RELAY_4 14


// Fridge
#define SWITCH_ID_3           "629f4ff4ff3682115439a8b5"    // Should look like "5dc1564130xxxxxxxxxxxxxx"

// Fan
#define SWITCH_ID_1           "62aecc86fb740f77fc1184d4"    // Should look like "5dc1564130xxxxxxxxxxxxxx"

//Motor
#define SWITCH_ID_4           "62aeccd8fce0b9e02e726747"    // Should look like "5dc1564130xxxxxxxxxxxxxx"

// WM
#define SWITCH_ID_2           "62af020afb740f77fc11a0c7"

// reset
#define SWITCH_ID_5           "62af384dfce0b9e02e72a077"

#define DHT11_ID_4            "62ac098dfce0b9e02e71259b"    // Should look like "5dc1564130xxxxxxxxxxxxxx"

#define SERVICE_UUID          "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID   "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define SMS_TARGET            "8971367622"



#define BAUD_RATE         115200                // Change baudrate to your need
#define EVENT_WAIT_TIME   60000               // send event every 60 seconds



struct {
  bool powerState = false;
  int fanSpeed = 1;
} device_state;


#pragma pack (1)
typedef struct {
  uint8_t confg_status;
  char wifi_ssid[64];
  char wifi_pass[64];
  uint32_t thingspeak_c_id;
  char thingspeak_apikey[64];
  char mobile_num[12];
}DevConfig;

typedef struct _sensor_data_{
  double s_dht_temp;
  double s_dht_humi;
  float s_bh1750_lux;
}SensorData;


TinyGsm modem(SerialAT);

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

bool deviceIsOn;                              // Temeprature sensor on/off state
float temperature;                            // actual temperature
float humidity;                               // actual humidity
float lastTemperature;                        // last known temperature (for compare)
float lastHumidity;                           // last known humidity (for compare)
unsigned long lastEvent = (-EVENT_WAIT_TIME); // last time event has been sent

#define DHTPIN 2 
#define DHTTYPE DHT11
BLEServer* pServer = NULL;
BLEServiceMap BleService;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;
static char g_config_data[360]={0};
DevConfig g_dev_config;
DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter(0x23);
uint32_t delayMS;
SensorData g_sensor_data;

void UltraSonic(){
  Serial.print("Distance = ");
  Serial.print(sonar.ping_cm());
  Serial.println(" cm");
}

/*---------------------------------------------------------------------------------*/

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
  oled.clearDisplay();              // Clear screen
  oled.setTextXY(0,0);              // Set cursor position, start of line 0
  oled.putString("Device ");
  oled.setTextXY(1,0);              // Set cursor position, start of line 1
  oled.putString("Updated");
  oled.setTextXY(2,0);              // Set cursor position, start of line 1
  oled.putString("Restart.!");

  return 0;
}

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
      oled.clearDisplay();              // Clear screen
      oled.setTextXY(0,0); 
      oled.putString("Device");
      oled.setTextXY(1,0); 
      oled.putString("Connected");
      BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer* pServer) {
      Serial.println("Client Device DisConnected");
      oled.clearDisplay();              // Clear screen
      oled.setTextXY(0,0); 
      oled.putString("Device Dis-");
      oled.setTextXY(1,0); 
      oled.putString("Connected");
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
  oled.clearDisplay();              // Clear screen
  oled.setTextXY(0,0);              // Set cursor position, start of line 0
  oled.putString("Device Not");
  oled.setTextXY(1,0);              // Set cursor position, start of line 1
  oled.putString("Configured");
  oled.setTextXY(2,0);              // Set cursor position, start of line 2
  oled.putString("Connect to BLE");   
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
/*-----------------------------------------------------------------------------------*/
bool onPowerState1(const String &deviceId, bool &state) {
  Serial.printf("Device 1 turned %s\r\n", state?"on":"off");
  if(state){
    digitalWrite(RELAY_1, LOW);
  }else{
    digitalWrite(RELAY_1, HIGH);
  }
  return true; // request handled properly
}

bool onPowerState2(const String &deviceId, bool &state) {
  Serial.printf("Device 2 turned %s\r\n", state?"on":"off");
  if(state){
    digitalWrite(RELAY_2, LOW);
  }else{
    digitalWrite(RELAY_2, HIGH);
  }  return true; // request handled properly
}

bool onPowerState3(const String &deviceId, bool &state) {
  Serial.printf("Device 3 turned %s\r\n", state?"on":"off");
  if(state){
    digitalWrite(RELAY_3, LOW);
  }else{
    digitalWrite(RELAY_3, HIGH);
  }  return true; // request handled properly
}

bool onPowerState4(const String &deviceId, bool &state) {
  Serial.printf("Device 4 turned %s\r\n", state?"on":"off");
  if(state){
    digitalWrite(RELAY_4, LOW);
  }else{
    digitalWrite(RELAY_4, HIGH);
  }  return true; // request handled properly
}

bool onPowerState6(const String &deviceId, bool &state) {
  Serial.printf("Device 6 turned %s\r\n", state?"on":"off");
  deviceIsOn = true;
  return true; // request handled properly
}


bool onPowerState5(const String &deviceId, bool &state) {
  Serial.printf("Device 5 turned %s\r\n", state?"on":"off");
  return true; // request handled properly
}


// setup function for WiFi connection
void setupWiFi() {
  Serial.printf("\r\n[Wifi]: Connecting");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(250);
  }

  Serial.printf("connected!\r\n[WiFi]: IP-Address is %s\r\n", WiFi.localIP().toString().c_str());
}

// setup function for SinricPro
void setupSinricPro() {
  Serial.println("setupSinricPro entry-------------->");
  SinricProSwitch& mySwitch1 = SinricPro[SWITCH_ID_1];
  mySwitch1.onPowerState(onPowerState1);

  SinricProSwitch& mySwitch2 = SinricPro[SWITCH_ID_2];
  mySwitch2.onPowerState(onPowerState2);

  SinricProSwitch& mySwitch3 = SinricPro[SWITCH_ID_3];
  mySwitch3.onPowerState(onPowerState3);

  SinricProSwitch& mySwitch4 = SinricPro[SWITCH_ID_4];
  mySwitch4.onPowerState(onPowerState4);

  SinricProSwitch& mySwitch5 = SinricPro[SWITCH_ID_5];
  mySwitch5.onPowerState(onPowerState5);

  SinricProTemperaturesensor &mySensor = SinricPro[DHT11_ID_4];
  mySensor.onPowerState(onPowerState6);

  Serial.println("setupSinricPro entry-------------->");
  Serial.println("setupSinricPro entry-------------->");

  SinricPro.onConnected([](){ Serial.printf("Connected to SinricPro\r\n"); }); 
  Serial.println("setupSinricPro entry-------------->");

  SinricPro.onDisconnected([](){ Serial.printf("Disconnected from SinricPro\r\n"); });
  Serial.println("setupSinricPro entry-------------->");

  SinricPro.begin(APP_KEY, APP_SECRET);
  Serial.println("setupSinricPro entry-------------->");

}

/*------------------------------------------------------------------------------*/

void WifiTask(void *arg)
{
  setupWiFi();
  setupSinricPro();
  while(1){
      SinricPro.handle();
      handleTemperaturesensor();
  }
}



/*------------------------------------------------------------------------------*/

void handleTemperaturesensor() {
  if (deviceIsOn == false) return; // device is off...do nothing

  unsigned long actualMillis = millis();
  if (actualMillis - lastEvent < EVENT_WAIT_TIME) return; //only check every EVENT_WAIT_TIME milliseconds

  temperature +=1;//dht.readTemperature();          // get actual temperature in °C
  humidity +=1; //dht.readHumidity();                // get actual humidity

  if (isnan(temperature) || isnan(humidity)) { // reading failed... 
    Serial.printf("DHT reading failed!\r\n");  // print error message
    return;                                    // try again next time
  } 

  if (temperature == lastTemperature || humidity == lastHumidity) return; // if no values changed do nothing...

  SinricProTemperaturesensor &mySensor = SinricPro[DHT11_ID_4];  // get temperaturesensor device
  bool success = mySensor.sendTemperatureEvent(temperature, humidity); // send event
  if (success) {  // if event was sent successfuly, print temperature and humidity to serial
    Serial.printf("Temperature: %2.1f Celsius\tHumidity: %2.1f%%\r\n", temperature, humidity);
  } else {  // if sending event failed, print error message
    Serial.printf("Something went wrong...could not send Event to server!\r\n");
  }

  lastTemperature = temperature;  // save actual temperature for next compare
  lastHumidity = humidity;        // save actual humidity for next compare
  lastEvent = actualMillis;       // save actual time for next compare
}

void Setup_BH1750(){
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println(F("BH1750 Advanced begin"));
  }
  else {
    Serial.println(F("Error initialising BH1750"));
  }
}

void StartSensors(){
    // Setup_DHT();
    dht.begin();
    Setup_BH1750(); 
 
    // pinMode(TRIGGER_PIN, OUTPUT); // Sets the trigPin as an Output
    // pinMode(ECHO_PIN, INPUT); // Sets the echoPin as an Input
    pinMode(RELAY_1, OUTPUT);
    pinMode(RELAY_2, OUTPUT);
    pinMode(RELAY_3, OUTPUT);
    pinMode(RELAY_4, OUTPUT);

}

void Read_BH1750(){
  if (lightMeter.measurementReady()) {
    g_sensor_data.s_bh1750_lux = lightMeter.readLightLevel();
    Serial.print("Light: ");
//    int val = map(g_sensor_data.s_bh1750_lux,0,1024,0,8000);
    Serial.print(g_sensor_data.s_bh1750_lux);
    Serial.println(" lx");
  }
}

void SensorTask(void *pvParameters){
  while(1){
//    Read_BH1750();
//    UltraSonic();
    // Read_DHT11();
  }
}


/*------------------------------------------------------------------------------*/


int StartFatFS(){
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return 0;
  }
  
  File file = SPIFFS.open("/config.json");
  if(!file){
    Serial.println("Failed to open file for reading");
    return 0;
  }
  
  Serial.println("File Content:");
  file.read((uint8_t *)&g_dev_config,sizeof(g_dev_config));
  Serial.println(g_dev_config.mobile_num);
  Serial.println(g_dev_config.wifi_pass);
  Serial.println(g_dev_config.wifi_ssid);
  Serial.println(g_dev_config.thingspeak_apikey);
  Serial.println(g_dev_config.thingspeak_c_id);
  Serial.println(g_dev_config.confg_status);
  
  file.close();
  SPIFFS.end();
  return 1;
}

/*------------------------------------------------------------------------------*/

static int GetDevConfig(){
  return 1;
}


void setup() {
  Serial.begin(115200);
  Wire.begin(); 
  oled.init();
//  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
//  Serial.println("Initializing modem...");
//  modem.restart();
//  if (strlen(simPIN) && modem.getSimStatus() != 3 ) {
//    modem.simUnlock(simPIN);
//  }
//    String smsMessage = "Hello MICRODIGISOFT!";
//
//  if(modem.sendSMS(SMS_TARGET, smsMessage)){
//    Serial.println(smsMessage);
//  }
//  else{
//    Serial.println("SMS failed to send");
//  }
  
  if(!StartFatFS()){
    GetDevConfig();
    return;
  }

  if( g_dev_config.confg_status == 1){
   StartSensors();
  oled.clearDisplay();              // Clear screen
  oled.setTextXY(0,0);              // Set cursor position, start of line 0
  oled.putString("Device");
   xTaskCreatePinnedToCore(WifiTask, "WifiTask", 1024*48, NULL, 2, NULL, 1);
   xTaskCreatePinnedToCore(SensorTask, "SensorTask", 4096, NULL, 2, NULL, 1);
  }else{
    
    xTaskCreatePinnedToCore(BLETask, "BLETask", 2048, NULL, 1, NULL, 1);
  }
}

void loop() {
}
