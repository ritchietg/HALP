// LED.cpp
#include "led_bar.h"
#include "Arduino.h"

LedBar::LedBar(int p1, int p2, int p3, int p4, int p5, int p6) {
  ledPins[0] = p1; ledPins[1] = p2; ledPins[2] = p3;
  ledPins[3] = p4; ledPins[4] = p5; ledPins[5] = p6;
  level = 0;
  brightness[0] = 10; brightness[1] = 10; brightness[2] = 30; brightness[3] = 30; brightness[4] = 255; brightness[5] = 255;
  delays[0] = 50; delays[1] = 50; delays[2] = 25; delays[3] = 25; delays[4] = 5; delays[5] = 5;
  for ( int x = 0; x < 6; x++ ) {
    //pinMode(ledPins[x], OUTPUT);
    ledcSetup(x, 8, 5000);
    ledcAttachPin(ledPins[x], x);
    ledcWrite(x, 0);
    pinLevels[x] = 0;
  }
}

int * LedBar::getLevels() {
  return pinLevels;
}

void LedBar::setLevel(int level) {
  Serial.println("Level:" + String(level));
  if ( level > -1 ) {
    for ( int x = 5; x > -1; x-- ) {
      //off [x][x][x][<][<][<]
      if ( x > level ) {
        while ( pinLevels[x] > 0 ) {
         pinLevels[x] = pinLevels[x]-5;
         ledcWrite(x, pinLevels[x]);
         delay(delays[x]);
       }
       //Serial.println(String(x) + " <- " + String(pinLevels[x]));
      }
    }
    //on [>][>][>][x][x][x]
    for ( int x = 0; x < 6; x++ ) {
      if ( x <= level ) {
        while ( pinLevels[x] < brightness[x] ) {
         pinLevels[x] = pinLevels[x]+5;
         ledcWrite(x, pinLevels[x]);
          delay(delays[x]);
        }
        //Serial.println(String(x) + " -> " + String(pinLevels[x]));
      }
    }    
  } else {
    for ( int x = 0; x < 6; x++ ) {
      ledcWrite(x, 0);
      pinLevels[x] = 0;
    }
  }
}

void LedBar::showIssue(int err, bool warning, bool forever){
  /*
   * 1 sec - init rtc
   * 2 sec - init sd
   * 3 sec - attach sd
   * 4 sec - sd low free space
   * 5 sec - sd out of free space
   */
   // turn off all leds
   Serial.println( "Issue Raised: " + String(err) );
   for ( int x = 0; x < 6; x++ ) {
      ledcWrite(x, 0);
      pinLevels[x] = 0;
   }
   int pinOne; int pinTwo;
   if (!warning) {
    pinOne = 0;
    pinTwo = 1;
   } else {
    pinOne = 2;
    pinTwo = 3;
   }
   
   if ( forever ) {
      //blink forever
      Serial.println( "Blinking Forever " + String(err*1000) );
      Serial.println( "p1: " + String(pinOne) + " p2: " + String(pinTwo) );
      while(true){
        ledcWrite(pinOne, 50); ledcWrite(pinTwo, 50); delay(err*1000);
        ledcWrite(pinOne, 0); ledcWrite(pinTwo, 0); delay(err*1000);
      }
   } else {
      ledcWrite(pinOne, 50); ledcWrite(pinTwo, 50); delay(err*1000);
      ledcWrite(pinOne, 0); ledcWrite(pinTwo, 0); delay(err*1000);    
   }
   setLevel(level);
}
