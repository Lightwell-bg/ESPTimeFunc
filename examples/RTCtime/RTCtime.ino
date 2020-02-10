#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include "ESPTimeFunc.h"

bool useRTC = true; //Set "true" if use RTC DS3231 (ESP8266 SDA -> D2, SCL ->D1; ESP32 SDA -> D21, SCL ->D22)

int8_t timezone = 2;               // UTC
String sNtpServerName = "us.pool.ntp.org";
const char sNtpServerName2[] = "pool.ntp.org";
const char sNtpServerName3[] = "time.nist.gov";
bool isDayLightSaving = false; //Summer time use
bool updateRTCfromNTP = true; //

const char* ssid = "ssid";
const char* password = "passw";

ESPTimeFunc myESPTime(useRTC);

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.printf("WiFi Failed!\n");
        return;
    }
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    myESPTime.begin(timezone, isDayLightSaving, sNtpServerName, sNtpServerName2, sNtpServerName3, useRTC, updateRTCfromNTP);
    Serial.println(F("Start Time"));
}

void loop() {
  static uint8_t lastSec= 0;
  struct tm tnow = myESPTime.getTimeStruct();
  if (lastSec != tnow.tm_sec) {
      Serial.print("Ok, RTC Time = "); print2digits(tnow.tm_hour);
      Serial.print(':'); print2digits(tnow.tm_min);
      Serial.print(':');    print2digits(tnow.tm_sec);
      Serial.print(", Date (D/M/Y) = ");    Serial.print(tnow.tm_mday);
      Serial.print('/');    Serial.print(tnow.tm_mon + 1);
      Serial.print('/');    Serial.print(tnow.tm_year + 1900);
      Serial.println();
      time_t timeUnix = myESPTime.getTimeUNIX();
      Serial.print("UNIX time: "); Serial.print(timeUnix);
      Serial.println();
      String timeString = myESPTime.getTimeStr(true); // or getTimeStr(false) without second
      Serial.print("STRING time: "); Serial.print(timeString);
      Serial.println();
      lastSec = tnow.tm_sec;
  }
}

void print2digits(uint8_t number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}
