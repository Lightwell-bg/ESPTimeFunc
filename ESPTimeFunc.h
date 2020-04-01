#ifndef ESPTimeFunc_h
#define ESPTimeFunc_h
#include <Arduino.h>
#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <sys/time.h>                   // struct timeval
#include <time.h>
#include "RTClib.h" //https://github.com/adafruit/RTClib

#define SECS_PER_HOUR 60*60
#define daylightOffset_sec 60*60
#define SECS_PER_DAY SECS_PER_HOUR*24

class ESPTimeFunc {
    public:
        ESPTimeFunc(bool haveRTC = false);
        bool begin(int8_t timeOffset, bool isDayLightSaving, String sNtpServerName,
                    const char* sNtpServerName2 = "pool.ntp.org", const char* sNtpServerName3 = "time.nist.gov",
                    bool useRTC = false, bool allwsSynchRTCfNTP = false);
        bool beginRTC(int8_t timeOffset, bool isDayLightSaving);
        time_t timeSynch();
        time_t getTimeUNIX(); 
        String getTimeStr(bool s = false);
        String getDateStr();
        struct tm getTimeStruct();
        void setTimeParam(bool useRTC, int8_t timeOffset, bool isDayLightSaving, String sNtpServerName);
        bool setTimeRTC(uint16_t year, uint8_t month, uint8_t day,
              uint8_t hour = 0, uint8_t min = 0, uint8_t sec = 0);
        bool setTimeRTC(time_t epoch_time);
        bool compTimeInt(float tFrom, float tTo, struct tm* timeNow);
        String beginTZ = "CET"; //"CET-2CEST,M3.5.0,M10.5.0/3"
        String endTZ = "CEST,M3.5.0,M10.5.0/3";
    private:
        int8_t _timeOffset;
        bool _isDayLightSaving;
        String _sNtpServerName;
        const char* _sNtpServerName2; 
        const char* _sNtpServerName3;
        RTC_DS3231 _rtc;
        bool _haveRTC;
        bool _useRTC;
        bool _allwsSynchRTCfNTP;
        uint32_t _timeUpdateTime; //last successful update time 
        bool _getNtpTime();
        void _getRtcTime();
};



#endif