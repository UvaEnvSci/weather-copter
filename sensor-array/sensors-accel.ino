// Date and time functions using RX8025 RTC connected via I2C and Wire lib

#include <Wire.h>

#include "DS3231.h"
#include <SDI12.h>
#include <Fat16.h>
#include <Fat16util.h>

#include <L3G4200D.h>
#include <ADXL345.h>
#include <HMC5883L.h>
#include <Adafruit_BMP085.h>
#include <Sensirion.h>

float heading;

//10DOF
L3G4200D gyro;
ADXL345 accel; 
HMC5883L compass;
Adafruit_BMP085 bmp;

//sht75
const uint8_t dataPin =  9;              // SHT serial data
const uint8_t sclkPin =  8;              // SHT serial clock
const uint32_t TRHSTEP   = 5000UL;       // Sensor query period
Sensirion sht = Sensirion(dataPin, sclkPin);
uint16_t rawData;
float temperature;
float humidity;
float dewpoint;
byte measActive = false;
byte measType = TEMP;
unsigned long trhMillis = 0;             // Time interval tracking

//Linear Thermistor
int inPin = A1;
double inputValue;
double tempInCelsius;
double slope = -0.1302;    //Place calibrated thermistor slope here in degC/V
double offset = 89.113;        //Place calibrated thermistor offset here in degC

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
    pinMode(inPin, INPUT);
    gyro.enableDefault();
    accel.powerOn();
    compass = HMC5883L();
    compass.SetScale(1.3); // Set the scale of the compass.
    compass.SetMeasurementMode(Measurement_Continuous); // Set the measurement mode to Continuous 
    bmp.begin();
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
     printLT();
      Serial.print(',');
      file.print(',');
//     printSHT();
//      Serial.print(',');
//      file.print(',');
     printGryo();
      Serial.print(',');
      file.print(',');
     printAccel();
      Serial.print(',');
      file.print(',');
     printBMP();
      Serial.print(',');
      file.print(',');
     printHeading();
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

//linear thermistor
void printLT() {
  inputValue = analogRead(inPin);
  tempInCelsius = inputValue * slope + offset;
  Serial.print(tempInCelsius);
  file.print(tempInCelsius);
  Serial.print("C");
  file.print("C");
}

//sht75
void printSHT() {
  sht.meas(TEMP, &rawData, BLOCK);        // Start temp measurement
  temperature = sht.calcTemp(rawData);     // Convert raw sensor data
  sht.meas(HUMI, &rawData, BLOCK);      // Start humidity measurement
  humidity = sht.calcHumi(rawData, temperature); // Convert raw sensor data
  dewpoint = sht.calcDewpoint(humidity, temperature);
  Serial.print(temperature);
  file.print(temperature);
  Serial.print("C,");
  file.print("C,");
  Serial.print(humidity);
  file.print(humidity);
  Serial.print("%,");
  file.print("%,");
  Serial.print(dewpoint);
  file.print(dewpoint);
  Serial.print("C");
  file.print("C");
}

// gryo values
void printGryo() {
  gyro.read();
  Serial.print((int)gyro.g.x);
  Serial.print(",");
  Serial.print((int)gyro.g.y);
  Serial.print(",");
  Serial.print((int)gyro.g.z);
  file.print((int)gyro.g.x);
  file.print(",");
  file.print((int)gyro.g.y);
  file.print(",");
  file.print((int)gyro.g.z);
}

// accel values
void printAccel() {
  int x,y,z;
  accel.readAccel(&x, &y, &z);
  Serial.print(x);
  Serial.print(",");
  Serial.print(y);
  Serial.print(",");
  Serial.print(z);
  file.print(x);
  file.print(",");
  file.print(y);
  file.print(",");
  file.print(z);
}

// bmp values
void printBMP(){
  Serial.print(bmp.readTemperature());
  Serial.print(",");
  Serial.print(bmp.readPressure());
  Serial.print(",");
  Serial.print(bmp.readAltitude());
  file.print(bmp.readTemperature());
  file.print(",");
  file.print(bmp.readPressure());
  file.print(",");
  file.print(bmp.readAltitude());
}

// compass heading
void printHeading(){
  // Retrive the raw values from the compass (not scaled).
  MagnetometerRaw raw = compass.ReadRawAxis();
  // Retrived the scaled values from the compass (scaled to the configured scale).
  MagnetometerScaled scaled = compass.ReadScaledAxis();
   
  // Values are accessed like so:
  int MilliGauss_OnThe_XAxis = scaled.XAxis;// (or YAxis, or ZAxis)
 
  // Calculate heading when the magnetometer is level, then correct for signs of axis.
  heading = atan2(scaled.YAxis, scaled.XAxis);  
   
  float declinationAngle = 0.0457;
  heading += declinationAngle;
   
  // Correct for when signs are reversed.
  if(heading < 0)
    heading += 2*PI;
     
  // Check for wrap due to addition of declination.
  if(heading > 2*PI)
    heading -= 2*PI;
    
  // Convert radians to degrees for readability.
  heading = heading * 180/M_PI;
  Serial.print(heading);
  file.print(heading);
}
