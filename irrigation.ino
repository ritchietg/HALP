#include "rtc_util.h"
#include "sd_util.h"
#include "led_bar.h"
#include "soil_sensor.h"
#include "my_button.h"

#include "HCSR04.h"

String wateringMode = "auto"; // or manual
String operationMode = "main"; //config or edit
/*  opertaing mode is the state the system is in
 *  0:main - show sensor levels. allow entering to config
 *    - buttons can be short pressed to manually cycle through sensor levels
 *    - buttons can be long pressed to <(pump) (config)> (both held is go to sleep)
 *  1:config
 *    - buttons can be short pressed to manually cycle through sensor levels <(hold to edit/save)
 *    - (save -> exit config mode)>
 *  2:edit
 *    - use pot to change value
 */
String targetRates[4] = {"soil 1","soil 2","water level","water interval"};
uint8_t targetRatesID[4][4] = {
  {0x6D, 0x5C, 0x00, 0x06},
  {0x6D, 0x5C, 0x00, 0x5B},
  {0x3E, 0x3E, 0x00, 0x06},
  {0x3E, 0x3E, 0x00, 0x30}
};
String currentShownTargetRate = targetRates[0];

String displayRates[6] = {"time", "mode", "soil 1","soil 2","water","voltage"};
uint8_t displayRateID[6][4] = {
  {0x78, 0x30, 0x54, 0x79},
  {0x54, 0x5C, 0x5E, 0x79},
  {0x6D, 0x5C, 0x00, 0x06},
  {0x6D, 0x5C, 0x00, 0x5B},
  {0x3E, 0x5F, 0x78, 0x50},
  {0x1C, 0x5C, 0x38, 0x78}
};

String currentShownRate = displayRates[0];
int store_soilRate1 = 0;
int store_soilRate2 = 0;
int store_waterRate = 0;
int store_voltageRate = 0;

int target_soilRate1 = 80;
int target_soilRate2 = 80;
int target_waterRate = 2;
int target_waterInterval = 3;

String configMode = "save";

bool buttonisPressed = false;
long buttonPreviousMillis = 0; // time of last press
#define WAKE_UP_BITMASK 0x900000000 // GPIOs 32 and 35
MyButton buttonOne(35);
  //press: maunal (wakeup/sleep) / press: sensor left / hold: pump override
  //config mode: press: sensor left / hold: enter or exit edit mode (ue pot to adjust values (0-100) for values, (0-24) for time)
  /* main modes:
   *  1) wake on time and check soil
   *  2) wake every 3 hours and check soil
   *  double button hex: 0x900000000
  */
MyButton buttonTwo(32);
  //press: maunal (wakeup/sleep) / press: sensor right / hold: config mode
  //config mode: press: sensor right / hold: ecit config mode

int voltagePin = A5;
int pumpPin = 4;

SoilSensor soilSensor1(A0, 1500, 3500);
SoilSensor soilSensor2(A3, 1500, 3500);

HCSR04 hc(2, 15); // distance sensor
SdUtil sdUtil;
RtcUtil rtcUtil;

LedBar ledBar(13,12,14,27,26,25);
int potPin = A6;

void toSleep(int secs){
  // go to sleep. provide 0 or less to ignore time wake up
  if ( secs > 0 ) {
    esp_sleep_enable_timer_wakeup(1000000 * secs);    
  }
  for ( int s=0; s<10; s++ ) {
    
  }
  esp_deep_sleep_start();
}

void setup() {
  Serial.begin(115200);
  
  // enable waking the esp by pressing both buttons
  esp_sleep_enable_ext1_wakeup(WAKE_UP_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);

  buttonOne.setupPin();
  buttonTwo.setupPin();
  
  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, LOW);

  if ( rtcUtil.initRtc() ) {
    rtcUtil.fetchDateTime();
    if ( sdUtil.initSdCard() ) {
      if ( sdUtil.sDAttached() ) {
        sdUtil.getSpace();
        if ( sdUtil.cardFreeSpace < 1000 ) {
          
          if ( rtcUtil.fetchDateTime() ) {
            sdUtil.logThis( sdUtil.issuesPath, rtcUtil.formattedDateStr, rtcUtil.timeStr + ",free space warning: " + String(int(sdUtil.cardFreeSpace)) + "MB left");            
          }
          
          ledBar.showIssue(4, true, false);
        } else if ( sdUtil.cardFreeSpace < 3 ) {
          // should not write to sd here. might want to delete files if specified in the config file
          ledBar.showIssue(5, true, true);
        }
        sdUtil.logThis( sdUtil.logsPath, "test", "test");
      } else {
        ledBar.showIssue(3, false, true);
      }
    } else {
      ledBar.showIssue(2, false, true);
    }
  } else {
    ledBar.showIssue(1, false, true);
  }

  
  xTaskCreate(loopOne, "loopOne", 2048, NULL, 1, NULL);
  xTaskCreate(loopTwo, "loopTwo", 2048, NULL, 1, NULL);
}

void buttonDelay( int d ) {
  //used in UI thread to avoid sleeping as buttons are pressed
  float waited = 0;
  while ( waited < d ) {
    if ( buttonisPressed ) {
      break;
    } else {
      waited += 10;
      delay(10);
    }
  }
}

void loopOne(void *arg) {
  while ( 1 ) {
    if ( operationMode == "main" ) {
      if ( ! buttonisPressed ) {
        if (currentShownRate == "time") {
            if ( rtcUtil.fetchDateTime() ) {
              Serial.println("Date: " + rtcUtil.dateStr);
              Serial.println("Time: " + rtcUtil.timeStr);
              buttonDelay(2000);
              buttonDelay(2000);
            }
            buttonDelay(2000);
        } else if (currentShownRate == "mode") {
            Serial.println("Mode: " + wateringMode);
            buttonDelay(2000);
        } else if (currentShownRate == "soil 1") {
            int s1 = soilSensor1.getRate(3,10);
            store_soilRate1 = s1;
            Serial.println("moisture: 1=" + String(s1) + " % " + String(soilSensor1.formattedRate));
            buttonDelay(2000);
            buttonDelay(2000);
        } else if (currentShownRate == "soil 2") {
            int s2 = soilSensor2.getRate(3,10);
            store_soilRate2 = s2;
            Serial.println("moisture: 2=" + String(s2) + " % " + String(soilSensor2.formattedRate));
            buttonDelay(2000);
            buttonDelay(2000);
        } else if (currentShownRate == "water") {
            float distAvg = 0;
            for ( int d=0; d<3; d++ ) {
              distAvg += (hc.dist()/2.54) + 0.50; //0.50 is a manual offset given my sensor's readings
              delay(10);
            }
            store_waterRate = distAvg;
            Serial.println("Distance: " + String(distAvg/3));
            buttonDelay(2000);
            buttonDelay(2000);
        } else if (currentShownRate == "voltage") {
            int v1 = map(analogRead(voltagePin), 0, 3300, 0, 100);
            store_voltageRate = v1;
            buttonDelay(2000);
            buttonDelay(2000);
        }
        // cycle through if button hasn't been pressed
        unsigned long currentMillis = millis();
        if(currentMillis - buttonPreviousMillis > 15000) {
          buttonPreviousMillis = currentMillis; 
          for ( int x=0; x < 6; x++ ) {
            if ( displayRates[x] == currentShownRate ) {
              if ( x == 5 ) { //end of list
                currentShownRate = displayRates[0];             
              } else {
                currentShownRate = displayRates[x+1];
              }
              Serial.println("shown rate = " + currentShownRate);
              break;
            }
          }
        }
      }
    } else if ( operationMode == "config" ) {
      if ( ! buttonisPressed ) {
        if ( currentShownTargetRate == "soil 1" ) {
          Serial.println("config = " + currentShownTargetRate);
          buttonDelay(2000);
          buttonDelay(2000);          
        } else if ( currentShownTargetRate == "soil 2" ) {
          Serial.println("config = " + currentShownTargetRate);
          buttonDelay(2000);
          buttonDelay(2000);
        } else if ( currentShownTargetRate == "water level" ) {
          Serial.println("config = " + currentShownTargetRate);
          buttonDelay(2000);
          buttonDelay(2000);
        } else if ( currentShownTargetRate == "water interval" ) {
          Serial.println("config = " + currentShownTargetRate);
          buttonDelay(2000);
          buttonDelay(2000);
        }
      }
    } else if ( operationMode == "edit" ) {
      if ( ! buttonisPressed ) {
        int newRate;
        if ( currentShownTargetRate == "soil 1" ) {
          while ( operationMode == "edit" ) {
            newRate = analogRead(potPin);
            newRate = map(newRate, 0, 4096, 0, 100);
            Serial.println("--pot map: " + String(newRate));
            buttonDelay(1000);
          }
          Serial.println("value changed:soil-1: " + String(target_soilRate1) + " to " + newRate);
          target_soilRate1 = newRate;
        } else if ( currentShownTargetRate == "soil 2" ) {
          while ( operationMode == "edit" ) {
            newRate = analogRead(potPin);
            newRate = map(newRate, 0, 4096, 0, 100);
            Serial.println("--pot map: " + String(newRate));
            buttonDelay(1000);
          }
          Serial.println("value changed:soil-2: " + String(target_soilRate2) + " to " + newRate);
          target_soilRate2 = newRate;
        } else if ( currentShownTargetRate == "water level" ) {
           while ( operationMode == "edit" ) {
            newRate = analogRead(potPin);
            newRate = map(newRate, 0, 4096, 0, 100);
            Serial.println("--pot map: " + String(newRate));
            buttonDelay(1000);
          }
          Serial.println("value changed:water-level: " + String(target_waterRate) + " to " + newRate);
          target_waterRate = newRate;
        } else if ( currentShownTargetRate == "water interval" ) {
           while ( operationMode == "edit" ) {
            newRate = analogRead(potPin);
            newRate = map(newRate, 0, 4096, 0, 6);
            Serial.println("--pot map: " + String(newRate));
            buttonDelay(1000);
          }
          Serial.println("value changed:water-interval: " + String(target_waterInterval) + " to " + newRate);
        }
        
      }
      buttonDelay(2000);
    }
    delay(5);    
  }
}
/*
int target_soilRate1 = 80;
int target_soilRate2 = 80;
int target_waterRate = 2;
int target_waterInterval = 3;
String targetRates[4] = {"soil 1","soil 2","water level","water interval"};
*/
void loopTwo(void *arg) {
  while ( 1 ) {
    if (buttonOne.isDePressed()) {
      buttonPreviousMillis = millis();
      buttonisPressed = true; // used as a display interrupt
      if ( buttonOne.isLongPress() ) {
        if ( operationMode == "main" ) {
          operationMode = "config";
          Serial.println("operation mode: " + operationMode);
          delay(1000);
        } else if ( operationMode == "config" ) {
          operationMode = "main";
          Serial.println("operation mode: " + operationMode);
          delay(1000);
        }
      } else {
        // short press
        buttonisPressed = true;
        if ( operationMode == "main" ) {
          for ( int x=0; x < 6; x++ ) {
            if ( displayRates[x] == currentShownRate) {
              if ( x == 0 ) { //end of list
                currentShownRate = displayRates[5];
              } else {
                currentShownRate = displayRates[x-1];
              }
              Serial.println("shown rate = " + currentShownRate);
              break;
            }
          }
        } else if ( operationMode == "config" ) {
          for ( int x=0; x < 4; x++ ) {
            if ( targetRates[x] == currentShownTargetRate) {
              if ( x == 0 ) { //end of list
                currentShownTargetRate = targetRates[3];
              } else {
                currentShownTargetRate = targetRates[x-1];
              }
              Serial.println("shown target rate = " + currentShownTargetRate);
              break;
            }
          }
        }
      }
      delay(500);
    } else if (buttonTwo.isDePressed()) {
      buttonPreviousMillis = millis();
      buttonisPressed = true;
      if ( buttonTwo.isLongPress() ) {
        if ( operationMode == "main" ) {
          while ( buttonTwo.isDePressed() ) {
            if ( !digitalRead(pumpPin) ) {
              Serial.println("pump is ON");
              digitalWrite(pumpPin, HIGH);
            }
            delay(100);
          }
          Serial.println("pump is OFF");
          digitalWrite(pumpPin, LOW);
        } else if ( operationMode == "config" ) {
          operationMode = "edit";
          Serial.println("operation mode: " + operationMode);
          delay(1000);
        } else if ( operationMode == "edit" ) {
          operationMode = "config";
          Serial.println("operation mode: " + operationMode);
          delay(1000);
        }
      } else {
        // short press
        if ( operationMode == "main" ) {
          for ( int x=0; x < 6; x++ ) {
            if ( displayRates[x] == currentShownRate) {
              if ( x == 5 ) { //end of list
                currentShownRate = displayRates[0];             
              } else {
                currentShownRate = displayRates[x+1];
              }
              Serial.println("shown rate = " + currentShownRate);
              break;
            }
          }
        } else if ( operationMode == "config" ) {
          for ( int x=0; x < 4; x++ ) {
            if ( targetRates[x] == currentShownTargetRate) {
              if ( x == 3 ) { //end of list
                currentShownTargetRate = targetRates[0];             
              } else {
                currentShownTargetRate = targetRates[x+1];
              }
              Serial.println("shown target rate = " + currentShownTargetRate);
              break;
            }
          }
        }
      }
      delay(500);
    } else {
      buttonisPressed = false;
    }
    delay(5);
  }
}


void loop() {
  /*
  rtcUtil.fetchDateTime();
  Serial.println("Date: " + rtcUtil.dateStr);
  Serial.println("Time: " + rtcUtil.timeStr);
  
  // - 0 -> 682.6
  // - 682.6 -> 1365.2
  // - 1365.2 -> 2047.8
  // - 2047.8 -> 2730.4
  // - 2730.4 -> 3413
  // - 3413 -> 4095
  int soilPotAvg[3] = {0,0,0};
  for ( int s=0; s<3; s++ ) {
    
    delay(10);
  }
  int soilPotRate = ( soilPotAvg[0]+soilPotAvg[1]+soilPotAvg[2] ) / 3 ;
  if ( previousSoilPotRate != soilPotRate && abs(soilPotRate - previousSoilPotRate) > 25 ) {  
    Serial.println(soilPotRate); //0-4096 (/6=682.6)
    if ( soilPotRate > 1 && soilPotRate < 682 ) {
      ledBar.setLevel(0);
    } else if ( soilPotRate > 682 && soilPotRate <= 1365 ) {
      ledBar.setLevel(1);
    } else if ( soilPotRate > 1365 && soilPotRate <= 2047 ) {
      ledBar.setLevel(2);
    } else if ( soilPotRate > 2047 && soilPotRate <= 2730 ) {
      ledBar.setLevel(3);
    } else if ( soilPotRate > 2730 && soilPotRate <= 3413 ) {
      ledBar.setLevel(4);
    } else if ( soilPotRate > 3413 && soilPotRate <= 4096 ) {
      ledBar.setLevel(5);
    } else {
      ledBar.setLevel(-1);
      delay(100);
    }
    previousSoilPotRate = soilPotRate;
    delay(10);
  } else {
    
  }
  //sdUtil.logThis( sdUtil.logsPath, "test", "test");

  
  Serial.println("voltage:" + String(map(analogRead(voltagePin), 0, 3300, 0, 100)));

  float distBuffer[3];
  for ( int d=0; d<3; d++ ) {
    distBuffer[d] = (hc.dist()/2.54) + 0.50; //0.50 is a manual offset given my sensor's readings
    delay(250);
  }
  float distAvg = (distBuffer[0] + distBuffer[1] + distBuffer[2]) / 3;
  Serial.println("Distance: " + String(distAvg));
  if ( distAvg != 1 ) { // if sensor is too close. produces a 0. we add 1.
    if ( distAvg > 6 ) { // min measuring distance
      Serial.println("Distance: " + String(distAvg));
    } else {
      Serial.println("distance is too short");
    }
  } else {
    Serial.println("invalid distance reading");
  }
  */
}
