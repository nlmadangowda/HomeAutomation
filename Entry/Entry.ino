#include "FFat.h"


void Task1( void *pvParameters );
void Task2( void *pvParameters );
void StartWiFi();
void StartBLE();
int StartFatFS();


int g_dev_res_status = 1;


void setup() {
  Serial.begin(115200);
  if(!StartFatFS()){
    return;
  }
  if(g_dev_res_status == 1){
    StartWiFi();
    xTaskCreatePinnedToCore(Task1, "Task1", 2048, NULL, 2, NULL, 1);
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
