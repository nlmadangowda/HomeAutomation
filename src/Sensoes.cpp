#include "main.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <BH1750.h>
/*MACROS----------------------------------------------------------------------------------*/

/*MACROS----------------------------------------------------------------------------------*/
#define DHT_PIN_DI 2
#define DHTTYPE    DHT11

/*GLOBEL VARIABLES----------------------------------------------------------------------------------*/
DHT_Unified dht(DHT_PIN_DI, DHTTYPE);
BH1750 lightMeter(0x23);
uint32_t delayMS;

typedef struct _sensor_data_{
  double s_dht_temp;
  double s_dht_humi;
  float s_bh1750_lux;
}SensorData;

SensorData g_sensor_data;

void Setup_DHT(){
  dht.begin();
  Serial.println(F("DHTxx Unified Sensor Example"));
  sensor_t sensor;
  
  dht.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("°C"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("°C"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("°C"));
  Serial.println(F("------------------------------------"));
  
  dht.humidity().getSensor(&sensor);
  Serial.println(F("Humidity Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("%"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("%"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("%"));
  Serial.println(F("------------------------------------"));
  delayMS = sensor.min_delay / 1000;
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
    
    Wire.begin();
    Setup_BH1750();
}

void Read_DHT11(){
  
  sensors_event_t event;
  
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    g_sensor_data.s_dht_temp = event.temperature;
    Serial.print(F("Temperature: "));
    Serial.print(event.temperature);
    Serial.println(F("°C"));
  }
  
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    g_sensor_data.s_dht_humi = event.relative_humidity;
    Serial.print(F("Humidity: "));
    Serial.print(event.relative_humidity);
    Serial.println(F("%"));
  }  
}


void Read_BH1750(){
  if (lightMeter.measurementReady()) {
    g_sensor_data.s_bh1750_lux = lightMeter.readLightLevel();
    Serial.print("Light: ");
    Serial.print(g_sensor_data.s_bh1750_lux);
    Serial.println(" lx");
  }
}

void SensorTask(void *pvParameters){
  while(1){
    delay(500);
    // Read_BH1750();
    delay(500);
    // Read_DHT11();
  }
}
