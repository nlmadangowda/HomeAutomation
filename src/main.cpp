#include "FFat.h"
#include "main.h"
#include <ArduinoJson.h>
#include "SPIFFS.h"

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


// char default_config[]={
//   #include "config.json"
// };

/*FUNCTION PROTO----------------------------------------------------------------------------------*/

/*GLOBEL VARIABLES----------------------------------------------------------------------------------*/
DevConfig g_dev_config;
static int GetDevConfig(){
  return 1;
}

void setup() {
  Serial.begin(115200);
  if(!StartFatFS()){
    GetDevConfig();
    return;
  }

  if( g_dev_config.confg_status == 1){
    // StartSensors();
    xTaskCreatePinnedToCore(WifiTask, "WifiTask", 4096, NULL, 2, NULL, 1);
    // xTaskCreatePinnedToCore(SensorTask, "SensorTask", 2048, NULL, 2, NULL, 0);
  }else{
    xTaskCreatePinnedToCore(BLETask, "BLETask", 2048, NULL, 1, NULL, 1);
  }
}

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


void loop(){}
