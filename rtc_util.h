#ifndef RTC_UTIL
#define RTC_UTIL
#include "Arduino.h"

class RtcUtil {
  private:
    
  public:
    RtcUtil();
    bool isRunning = false;
    int hrInt = 0;
    int minInt = 0;
    int monInt = 0;
    int dayInt = 0;
    int yrInt = 0;
    String dateStr = "unset";
    String timeStr = "unset";
    String formattedDateStr = "unset";
    bool initRtc();
    bool fetchDateTime();
};
#endif
