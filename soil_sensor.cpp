// LED.cpp
#include "soil_sensor.h"
#include "Arduino.h"

SoilSensor::SoilSensor(int sensorPin, int mi, int ma) {
  pin = sensorPin;
  minRate = mi;
  maxRate = ma;
}

int SoilSensor::getRate(int count, int wait) {
  if ( pin != -1 ) {
    int bufferAvg = 0;
    for ( int r=0; r < count; r++ ) {
      int rate = analogRead(pin);
      if ( rate < minRate ) {
        rate = minRate;
      } else if ( rate > maxRate ) {
        rate = maxRate;
      }
      bufferAvg += rate;
      delay(wait);
    }
    rate = bufferAvg / count; //obj
    if ( minimum == -1 || rate < minimum ) { minimum = rate; }
    if ( maximum == -1 || rate > maximum ) { maximum = rate; }
    formattedRate = map(rate, minRate, maxRate, 100, 0);
    return bufferAvg / count;    
  } else {
    Serial.print("Sensor: " + String(pin) + " is not set up!");
    rate = -1;
    return -1;
  }
}
