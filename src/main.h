#ifndef MAIN_H
#define MAIN_H
#include <Arduino.h>

#pragma pack (1)
typedef struct {
  uint8_t confg_status;
  char wifi_ssid[64];
  char wifi_pass[64];
  uint32_t thingspeak_c_id;
  char thingspeak_apikey[64];
  char mobile_num[12];
}DevConfig;

void Task1(void *pvParameters);
void StartWiFi();
void Task3(void *pvParameters);
void Task2(void *pvParameters);
void StartBLE();

#endif //MAIN_H