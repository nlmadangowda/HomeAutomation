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
static int GetDevConfig(){

}

void setup() {
  Serial.begin(115200);
//  if(!StartFatFS()){
//    return;
//  }

  // if(UpdateDevConfig()!=1){
  //   Serial.println("\nFailed to update teh device config, plz reconfigure the settings");
  // }
g_dev_config.confg_status = 1;
  if( g_dev_config.confg_status == 1){
    StartWiFi();
    // StartSensors();
    xTaskCreatePinnedToCore(WifiTask, "WifiTask", 4096, NULL, 2, NULL, 1);
    // xTaskCreatePinnedToCore(SensorTask, "SensorTask", 2048, NULL, 2, NULL, 0);
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
