// LED.cpp
#include "my_button.h"
#include "Arduino.h"

MyButton::MyButton(int p) {
  pin = p;
}

void MyButton::setupPin(){
  pinMode(pin, INPUT);
  Serial.println("pin setup -> " + String(pin) + " : " + String(digitalRead(pin))); 
}

bool MyButton::isDePressed(){
  return digitalRead(pin);
}

bool MyButton::isLongPress(){
  float t = 0.0;
  while ( digitalRead(pin) ) {
    if ( t <= 2.5 ) {
      t += 0.10;
      delay(10);
    } else {
      //raise warning
      break;
    }
  }
  if ( t >= 2.5 ) {
    Serial.println("Long press on pin: " + String(pin) + " : " + String(t));
    return true;
  } else {
    Serial.println("Short press on pin: " + String(pin) + " : " + String(t));    
    return false;    
  }
}
