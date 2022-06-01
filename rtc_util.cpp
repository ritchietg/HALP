#include "rtc_util.h"
#include "Wire.h"
#include "RtcDS1307.h"
#include "Arduino.h"

RtcDS1307<TwoWire> Rtc(Wire);


#define countof(a) (sizeof(a) / sizeof(a[0]))

RtcUtil::RtcUtil(){  
}

bool RtcUtil::initRtc(){
  Rtc.Begin();
  //
  if (!Rtc.IsDateTimeValid()) {
    if (Rtc.LastError() != 0) {
      Serial.print("RTC communications error = ");
      Serial.println(Rtc.LastError());
      return false;
    } else {
      Serial.println("RTC lost confidence in the DateTime!");
      RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
      Rtc.SetDateTime(compiled);
    }
  }
  if (!Rtc.GetIsRunning()) {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }
  Rtc.SetSquareWavePin(DS1307SquareWaveOut_Low);
  isRunning = true;
  return true;
}

bool RtcUtil::fetchDateTime(){
  if ( isRunning ) {
    RtcDateTime dt = Rtc.GetDateTime();
    bool gotTime = false;
    for ( int t=0; t<10; t++ ) {
      if ( dt.Hour() > 24 || dt.Minute() > 60 || dt.Month() > 12 || dt.Day() > 31 ) {
        Serial.println("RTC time is invalid: " + String(t));
        delay(2000);
        dt = Rtc.GetDateTime();
      } else {
        gotTime = true;
        break;
      }
    }
    if ( ! gotTime ) {
      Serial.println("Failed to get date/time!");
      return false;
    }
    hrInt = dt.Hour();
    minInt = dt.Minute();
    monInt = dt.Month();
    dayInt = dt.Day();
    yrInt = dt.Year();

    char datestr[20];
    snprintf_P(datestr, 
            countof(datestr),
            PSTR("%02u/%02u/%04u"),
            dt.Month(),
            dt.Day(),
            dt.Year());
    // used for naming files in logs folder
    String buf = String(datestr);
    dateStr = buf;
    buf.replace("/", "_");
    formattedDateStr = buf;
    //
    char timestr[20];
    snprintf_P(timestr, 
            countof(timestr),
            PSTR("%02u:%02u:%02u"),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    timeStr = String(timestr);
    return true;    
  } else {
    Serial.println("RTC is not running");
    return false;
  }
}
