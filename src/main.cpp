#include "FFat.h"
#include "main.h"
#include <ArduinoJson.h>

/*MACROS----------------------------------------------------------------------------------*/

/*MACROS----------------------------------------------------------------------------------*/
#define DHT_PIN_DI 2
#define DHTTYPE    DHT11
/*FUNCTION PROTO----------------------------------------------------------------------------------*/
void WifiTask( void *pvParameters );
void BLETask( void *pvParameters );
void SensorTask( void *pvParameters );
void StartWiFi();
void StartBLE();
void StartSensors();
int StartFatFS();


char default_config[]={
  #include "config.json"
};

/*FUNCTION PROTO----------------------------------------------------------------------------------*/

/*GLOBEL VARIABLES----------------------------------------------------------------------------------*/
DevConfig g_dev_config;

int UpdateDevConfig(){
  StaticJsonDocument<360> doc;
  DeserializationError error = deserializeJson(doc, default_config);
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
  Serial.println(g_dev_config.mobile_num);
  Serial.println(g_dev_config.wifi_pass);
  Serial.println(g_dev_config.wifi_ssid);
  Serial.println(g_dev_config.thingspeak_apikey);
  Serial.println(g_dev_config.thingspeak_c_id);
  Serial.println(g_dev_config.confg_status);
  return 1;
}


void setup() {
  Serial.begin(115200);
//  if(!StartFatFS()){
//    return;
//  }

  // if(UpdateDevConfig()!=1){
  //   Serial.println("\nFailed to update teh device config, plz reconfigure the settings");
  // }

  if( g_dev_config.confg_status == 1){
    StartWiFi();
    StartSensors();
    xTaskCreatePinnedToCore(WifiTask, "WifiTask", 4096, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(SensorTask, "SensorTask", 2048, NULL, 2, NULL, 0);
  }else{
    StartBLE();
    xTaskCreatePinnedToCore(BLETask, "BLETask", 2048, NULL, 1, NULL, 1);
  }
}

int StartFatFS(){
  if(!FFat.begin(true)){
    Serial.println("Failed to Mount the Memory");
    return 0;
  }
  return 1;
}


void loop(){}
