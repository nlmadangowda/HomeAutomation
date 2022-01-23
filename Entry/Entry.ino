#include "FFat.h"

/*MACROS----------------------------------------------------------------------------------*/

/*MACROS----------------------------------------------------------------------------------*/
#define DHT_PIN_DI 2
#define DHTTYPE    DHT11
/*FUNCTION PROTO----------------------------------------------------------------------------------*/
void Task1( void *pvParameters );
void Task2( void *pvParameters );
void Task3( void *pvParameters );
void StartWiFi();
void StartBLE();
void StartSensors();
int StartFatFS();
/*FUNCTION PROTO----------------------------------------------------------------------------------*/

/*GLOBEL VARIABLES----------------------------------------------------------------------------------*/
int g_dev_res_status = 1;


void setup() {
  Serial.begin(115200);
//  if(!StartFatFS()){
//    return;
//  }
  if(g_dev_res_status == 1){
    StartWiFi();
    StartSensors();
    xTaskCreatePinnedToCore(Task1, "Task1", 4096, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(Task3, "Task3", 2048, NULL, 2, NULL, 0);

  }else{
    StartBLE();
    xTaskCreatePinnedToCore(Task2, "Task2", 2048, NULL, 1, NULL, 1);
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
