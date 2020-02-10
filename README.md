# ESPTimeFunc
A library for working with time functions for ESP8266 and ESP32. To obtain accurate time using NTP servers and/or IC DS3231

## Methods
```cpp
		ESPTimeFunc(bool haveRTC = false); //constructor
        bool begin(uint8_t timeOffset, bool isDayLightSaving, String sNtpServerName,
				const char* sNtpServerName2 = "pool.ntp.org", const char* sNtpServerName3 = "time.nist.gov",
        bool useRTC = false, bool allwsSynchRTCfNTP = false);
        bool beginRTC(uint8_t timeOffset, bool isDayLightSaving);
        time_t timeSynch();
        time_t getTimeUNIX(); 
        String getTimeStr(bool s = false);
        String getDateStr();
        struct tm getTimeStruct();
        void setTimeParam(bool useRTC, uint8_t timeOffset, bool isDayLightSaving, String sNtpServerName);
        bool setTimeRTC(uint16_t year, uint8_t month, uint8_t day,
				uint8_t hour = 0, uint8_t min = 0, uint8_t sec = 0);
        bool setTimeRTC(time_t epoch_time);
        bool compTimeInt(float tFrom, float tTo, struct tm* timeNow);
```

#### This works with the ESP8266 and ESP32 Arduino platform with a recent stable release