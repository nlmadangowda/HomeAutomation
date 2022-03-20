#include "main.h"
#include <WiFiMulti.h>
#include "ThingSpeak.h"


WiFiMulti wifiMulti;
WiFiClient  client;

unsigned long myChannelNumber = 1680155;
const char * myWriteAPIKey = "7TXEYK6XZO446CJZ";
String myStatus = "";

void WifiTask(void *pvParameters){
  (void) pvParameters;
  int number1= 0;
  while(1){
    if(wifiMulti.run() != WL_CONNECTED) {
        Serial.println("WiFi not connected!");
        delay(1000);
    }
    // set the fields with the values
    // ThingSpeak.setField(1, number1);
    // figure out the status message
    // myStatus = String("field1 equals field2");
    
    // set the status
    // ThingSpeak.setStatus(myStatus);
    
    // write to the ThingSpeak channel
    int x = ThingSpeak.writeField(myChannelNumber, 1, number1, myWriteAPIKey);
    if(x == 200){
      Serial.println("Channel update successful.");
    }
    else{
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
    
    // change the values
    number1++;
    if(number1 > 99){
      number1 = 0;
    }
    
    delay(10000); // Wait 20 seconds to update the channel again
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
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}
