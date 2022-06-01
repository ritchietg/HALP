#include <SD.h>
#include <sd_defines.h>
#include <sd_diskio.h>

#include "FS.h"
#include "SPI.h"
#include "Arduino.h"
#include "sd_util.h"

SdUtil::SdUtil(){
  
}

bool SdUtil::initSdCard(){
  int failCount = 0;
  int failLimit = 5;
  while( !SD.begin(5) ) {
    if( failCount < failLimit ){
      failCount++;
      delay(500);
      Serial.println("SD Card Mount Failed: " + String(failCount));
    } else {
      return false;
    }
  }
  Serial.println("SD Card Mounted: " + String(failCount));
  isMounted = true;
  return true;
}

bool SdUtil::sDAttached(){
  int failCount = 0;
  int failLimit = 5;
  while( SD.cardType() == CARD_NONE ) {
    if( failCount < failLimit ){
      failCount++;
      delay(500);
      Serial.println("SD Card Attach Failed: " + String(failCount));
    } else {
      return false;
    }
  }
  Serial.println("SD Card Attached: " + String(failCount));
  isAttached = true;
  return true;
}

void SdUtil::getSpace(){
  cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  cardSpaceTotal = SD.totalBytes() / (1024 * 1024);
  Serial.printf("Total space: %lluMB\n", cardSpaceTotal);
  cardSpaceUsed = SD.usedBytes() / (1024 * 1024);
  Serial.printf("Used space: %lluMB\n", cardSpaceUsed);
  cardFreeSpace = cardSpaceTotal - cardSpaceUsed;
  Serial.printf("Free space: %lluMB\n", cardFreeSpace);
}

void SdUtil::logThis(String root, String fileName, String msg){
  String path = root + "/" + fileName + ".csv";
  //
  int failCount = 0;
  int failLimit = 5;
  //
  while( failCount < failLimit  ) {
    File file = SD.open(path, FILE_APPEND);
    if(file){
      if(file.println(msg)){
        Serial.println("Appended: " + msg + " to " + path);
        file.close();
        break;
      } else {
        Serial.println("Append failure: " + msg + " to " + path);
        failCount++;
        delay(500);
        file.close();
      }   
    } else {
      Serial.println("Failed to open file: " + String(failCount));
      failCount++;
      delay(500);
    }
  }
}
