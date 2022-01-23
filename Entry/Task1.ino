#include <WiFi.h>
#include <WiFiMulti.h>
#include "secrets.h"
#include "ThingSpeak.h"


WiFiMulti wifiMulti;

void Task1(void *pvParameters){
  (void) pvParameters;
  while(1){
    if(wifiMulti.run() != WL_CONNECTED) {
        Serial.println("WiFi not connected!");
        delay(500);
    }else{
      Serial.println("Task1");
      delay(100);
    }
  }
}

void StartWiFi(){
  delay(100);
  wifiMulti.addAP("ACTFIBERNET_5G", "act12345");
  wifiMulti.addAP("BSNL", "8172295635");
  wifiMulti.addAP("ACTFIBERNET", "act12345");
  Serial.println("Connecting Wifi...");
    if(wifiMulti.run() == WL_CONNECTED) {
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
    }
}
