// Date and time functions using RX8025 RTC connected via I2C and Wire lib

#include <Wire.h>
#include "DS3231.h"
#include <SDI12.h>
#include <Fat16.h>
#include <Fat16util.h>

#define DATAPIN 3         // change to the proper pin
SDI12 mySDI12(DATAPIN);
String myCommand = "0R0!";

SdCard card;
Fat16 file;

bool newline = false;

// store error strings in flash to save RAM
#define error(s) error_P(PSTR(s))

void error_P(const char* str) {
  PgmPrint("error: ");
  SerialPrintln_P(str);
  if (card.errorCode) {
    PgmPrint("SD error: ");
    Serial.println(card.errorCode, HEX);
  }
  while(1);
}

DS3231 RTC; //Create the R8025 object

void setup () 
{
  
    Serial.begin(9600);
    Wire.begin();
    RTC.begin();
    mySDI12.begin();
}

void loop () 
{
  
      // initialize the SD card
    if (!card.init()) error("card.init");
  
    // initialize a FAT16 volume
    if (!Fat16::init(&card)) error("Fat16::init");
  
    char name[] = "DATALOG.CSV";
    // clear write error
    file.writeError = false;
  
    // O_CREAT - create the file if it does not exist
    // O_APPEND - seek to the end of the file prior to each write
    // O_WRITE - open for write
    if (!file.open(name, O_CREAT | O_APPEND | O_WRITE))
        error("error opening file");  
  
  mySDI12.sendCommand(myCommand);
  delay(100);           // wait a while for a response
     printTime();
      Serial.print(',');
      file.print(',');
  while(mySDI12.available()){    // write the response to the screen
    char i = mySDI12.read();
    if (i != '\n') {
      Serial.write(i);
      file.write(i);
    } 
  }
    Serial.println();
//    file.println();
    
    if (!file.close()) 
        error("error closing file");
    
    delay(900);
}

void printTime () {
  DateTime now = RTC.now(); //get the current date-time
    char timestp[15];
    sprintf(timestp, "%04d-%02d-%02d-%02d:%02d:%02d", now.year(), now.month(), now.date(), now.hour(), now.minute(), now.second());
    file.print(timestp);
    Serial.print(timestp);
}


