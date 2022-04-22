#include "main.h"
#include <WiFiMulti.h>
#include "ThingSpeak.h"


WiFiMulti wifiMulti;
WiFiClient  client;

unsigned long myChannelNumber = 1680155;
const char * myWriteAPIKey = "7TXEYK6XZO446CJZ";
String myStatus = "";

void UploadSensorDataToThingSpeak(){
    static int number1= 0;
    ThingSpeak.setField(1, number1);
    ThingSpeak.setField(2, number1);
    ThingSpeak.setField(3, number1);
    ThingSpeak.setField(4, number1);
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    if(x == 200){
      Serial.println("Channel update successful.");
    }
    else{
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
    number1++;
    if(number1 > 99){
      number1 = 0;
    }
    delay(21000); // Wait 20 seconds to update the channel again
}

void WifiTask(void *pvParameters){
  StartWiFi();
  (void) pvParameters;
  while(1){
    if(wifiMulti.run() != WL_CONNECTED) {
        Serial.println("WiFi not connected!");
        delay(1000);
    }else{
      UploadSensorDataToThingSpeak();
    }
  }
}

void StartWiFi(){
  delay(100);
  wifiMulti.addAP("royalicon204", "royalicon204");
  Serial.println("Connecting Wifi...");
    if(wifiMulti.run() == WL_CONNECTED) {
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
    }
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}
