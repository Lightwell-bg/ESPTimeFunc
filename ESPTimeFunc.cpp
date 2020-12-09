#include <Arduino.h>
#include <ESPTimeFunc.h>

ESPTimeFunc::ESPTimeFunc(bool haveRTC) {
    _haveRTC = haveRTC;
}

bool ESPTimeFunc::begin(int8_t timeOffset, bool isDayLightSaving, String sNtpServerName, const char* sNtpServerName2, const char* sNtpServerName3, bool useRTC, bool allwsSynchRTCfNTP) {
    _timeOffset = timeOffset;
    _isDayLightSaving = isDayLightSaving;
    _sNtpServerName = sNtpServerName;
    _sNtpServerName2 = sNtpServerName2;
    _sNtpServerName3 = sNtpServerName3;
    _haveRTC ? _useRTC = useRTC : _useRTC = false;
    _allwsSynchRTCfNTP = allwsSynchRTCfNTP;
    if (_haveRTC) {
        uint8_t _tries = 5;
        while (--_tries && !_rtc.begin()) {
            Serial.println(F("Couldn't find RTC"));
            //while (1);
            delay(100);
        }  
        if (_tries > 0) {
            Serial.println(F("RTC found"));
            if (_rtc.lostPower()) {
                Serial.println(F("RTC lost power, lets set the time!"));
                _rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // following line sets the RTC to the date & time this sketch was compiled
                // This line sets the RTC with an explicit date & time, for example to set
                // January 21, 2014 at 3am you would call:
                // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
            } 
        }
    }
    timeSynch();
}

bool ESPTimeFunc::beginRTC(int8_t timeOffset, bool isDayLightSaving) {
    _haveRTC = true;
    _useRTC = true;
    _timeOffset = timeOffset;
    _isDayLightSaving = isDayLightSaving;
    uint8_t _tries = 5;
    while (--_tries && !_rtc.begin()) {
        Serial.println(F("Couldn't find RTC"));
        //while (1);
        delay(100);
    }  
    if (_tries > 0) {
        Serial.println(F("RTC found"));
        if (_rtc.lostPower()) {
            Serial.println(F("RTC lost power, lets set the time!"));
            _rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // following line sets the RTC to the date & time this sketch was compiled
            // This line sets the RTC with an explicit date & time, for example to set
            // January 21, 2014 at 3am you would call:
            // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
        } 
    }
    timeSynch();    
}

time_t ESPTimeFunc::timeSynch() {
    time_t tn;
    struct tm* tm;
    if (!_haveRTC) {
        if (WiFi.status() == WL_CONNECTED) { 
            if (_getNtpTime()) { 
                tn = time(NULL);
                tm = localtime(&tn);
                Serial.println("Time Ready NTP!");      
                return _timeUpdateTime = millis(); 
            }
        }
        else    {Serial.println("Time NOT Ready NTP!"); return _timeUpdateTime = 0;}
    }        
    else {
        if (_useRTC) {
            _getRtcTime();
            Serial.println("Time Ready RTS!");      
            return _timeUpdateTime = millis(); 
        }
        else { //!_useRTC
            if (WiFi.status() == WL_CONNECTED) { 
                if (_getNtpTime()) { 
                    tn = time(NULL);
                    tm = localtime(&tn);
                    if (_allwsSynchRTCfNTP) { //if got time from NTP set RTC
                        setTimeRTC(tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
                    } 
                    return _timeUpdateTime = millis(); 
                }
                else {Serial.println("Time NOT Ready NTP!"); return _timeUpdateTime = 0;}
            }
            else {
                _getRtcTime();
                Serial.println("Time Ready RTS instead NTP!");
                return _timeUpdateTime = millis(); 
            }
        }
    }
}

bool ESPTimeFunc::_getNtpTime() {
    time_t t;  
    uint8_t count = 0;
    struct tm *timeinfo;
    unsigned long summerOffsetVal = 0;
    if (WiFi.status() == WL_CONNECTED) {
        String strTimeOffset; // "CET-2CEST,M3.5.0,M10.5.0/3"
        String fullTZ;
        if (_timeOffset > 0) 
            strTimeOffset =  "-" + String(_timeOffset);   
        else 
            strTimeOffset =  "+" + String(abs(_timeOffset));  
        configTime(0, 0, _sNtpServerName.c_str(), _sNtpServerName2, _sNtpServerName3); // enable NTP
        if(_isDayLightSaving) {
            fullTZ = beginTZ + strTimeOffset + endTZ;
        }
        else {
            //configTime(_timeOffset*SECS_PER_HOUR, 0, _sNtpServerName.c_str(), _sNtpServerName2, _sNtpServerName3); // enable NTP
            fullTZ = beginTZ + strTimeOffset;
        }
        //Serial.print("fullTZ "); Serial.println(fullTZ);
        setenv("TZ", fullTZ.c_str(), 3);   // this sets TZ 
        //setenv("TZ", "CET-2CEST,M3.5.0,M10.5.0/3", 3); 
        tzset();    
        Serial.println("\nWaiting for time");
        t = time(NULL);
        timeinfo = localtime(&t);
        while (timeinfo->tm_year < (2017 - 1900) && count < 10) {
            t = time(NULL);
            timeinfo = localtime(&t);
            Serial.print(".");
            Serial.print("t "); Serial.println(t);
            Serial.print("timeinfo->tm_year "); Serial.println(timeinfo->tm_year);
            count ++;
            delay(500);
        }
        if (timeinfo->tm_year > (2017 - 1900)) {
            Serial.println("Time NTP ready.");
            Serial.print("timeinfo: "); Serial.printf("%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min); Serial.println();
            return true;             
        }
        else return false;
    }
    else return false;
}

void ESPTimeFunc::_getRtcTime() {
    struct timeval epoch;
    time_t epoch_time;
    epoch_time = _rtc.now().unixtime();// - timezone * SECS_PER_HOUR - daylightOffset_sec;; 
    epoch = {epoch_time, 0};
    //const timeval *tv = &epoch;
    //timezone utc = {0,0};
    //const timezone *tz = &utc;
    //settimeofday(tv, tz);
    settimeofday(&epoch, nullptr);
    /*if(_isDayLightSaving) {
        setenv("TZ", "CET-0CEST,M3.5.0,M10.5.0/3", 3);   // UTC + DST
        tzset();
    }
    else {
        setenv("TZ", "CET-0", 3);   // UTC
        tzset();    
    }*/
}

/*time_t ESPTimeFunc::_getRtcTime() {
    time_t t;  
    struct tm *timeinfo;
    t = _rtc.now().unixtime();// - timezone * SECS_PER_HOUR - daylightOffset_sec;; 
    timeinfo = localtime(&t);
    return t;// + timezone * SECS_PER_HOUR; 
}*/

time_t ESPTimeFunc::getTimeUNIX() {
    //struct tm *tm;
    time_t tn = time(NULL);
    //tm = localtime(&tn);
    //if (locTime) return mktime(tm);
    return tn; 
}

String ESPTimeFunc::getTimeStr(bool s) { //s - show seconds
    struct tm *tm;
    time_t tn = time(NULL);
    tm = localtime(&tn);
    String Time;
    if (s) Time = String(tm->tm_hour)+":"+ (tm->tm_min < 10 ? "0"+String(tm->tm_min) : String(tm->tm_min)) +":"+ String(tm->tm_sec);  
    else Time = String(tm->tm_hour)+":"+ (tm->tm_min < 10 ? "0"+String(tm->tm_min) : String(tm->tm_min));
    //Serial.println("GetTime() "+Time);
    return Time; 
}

String ESPTimeFunc::getDateStr() {
    struct tm *tm;
    time_t tn = time(NULL);
    tm = localtime(&tn);
    String Date = String(tm->tm_mday)+"."+((tm->tm_mon+1)<10 ? "0" + String(tm->tm_mon+1) :  String(tm->tm_mon+1)) +"."+String(tm->tm_year+1900);//""; // Строка для результатов времени 
    //String Date = String(day(tn))+" "+monthStr(month(tn)) +" "+String(year(tn)) + ", " + dayStr(weekday(tn));//""; // Строка для результатов времени  
    //String Date = String(day(tn))+" "+month_table[lang][month(tn)-1] +" "+String(year(tn)) + ", " + day_table[lang][weekday(tn)-1];
    //String Date = String(day(tn))+ "." + (month(tn)<10 ? "0" + String(month(tn)) :  String(month(tn))) + "." + String(year(tn)) + ", " + day_table[lang][weekday(tn)-1];
    //Serial.println(Date);
    Date.toLowerCase();
    return Date; 
}

struct tm ESPTimeFunc::getTimeStruct() {
    struct tm *tm;
    time_t tn = time(NULL);
    tm = localtime(&tn);
    return *tm;
}

bool ESPTimeFunc::setTimeRTC(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec) {
    ESP.eraseConfig();
    _rtc.adjust(DateTime(year, month, day, hour, min, sec));
}

bool ESPTimeFunc::setTimeRTC(time_t epoch_time) {
    //ESP.eraseConfig();
    struct timeval epoch;
    epoch = {epoch_time, 0};
    //const timeval *tv = &epoch;
    //timezone utc = {0,0};
    //timezone tz = {0,0};
    //const timezone *tz = &utc;
    //settimeofday(&epoch, &tz); 
    settimeofday(&epoch, nullptr);
    _rtc.adjust(DateTime(epoch_time));    
}

void ESPTimeFunc::setTimeParam(bool useRTC, int8_t timeOffset, bool isDayLightSaving, String sNtpServerName) {
    _haveRTC ? _useRTC = useRTC : _useRTC = false;
    //_useRTC = useRTC;
    _timeOffset = timeOffset;
    _isDayLightSaving = isDayLightSaving;
    _sNtpServerName = sNtpServerName;
}

bool ESPTimeFunc::compTimeInt(float tFrom, float tTo, struct tm* timeNow) { //Comparing time for proper processing from 18.00 to 8.00
    float tNow = timeNow->tm_hour + float(timeNow->tm_min)/100;;
    if (tFrom < tTo)  {
        if ((tFrom <= tNow) && (tTo >= tNow)) return true; else return false;
    }
    else if (tFrom > tTo) {
        if (tNow <= 23.59 && tFrom <= tNow)  return true;
        else if (tNow >= 0 && tTo >= tNow)  return true;
        else return false;  
    }
    else {
        return false;
    }
}
