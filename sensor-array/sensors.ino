#include <Wire.h>
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

void setup() {
  Serial.begin(9600);
  pinMode(inPin, INPUT);
  Wire.begin();
  gyro.enableDefault();
  accel.set_bw(ADXL345_BW_12);
  compass = HMC5883L();
  compass.SetScale(1.3); // Set the scale of the compass.
  compass.SetMeasurementMode(Measurement_Continuous); // Set the measurement mode to Continuous 
  bmp.begin();
}

void loop() {
  
  //linear thermistor
  inputValue = analogRead(inPin);
  tempInCelsius = inputValue * slope + offset;
  Serial.print(tempInCelsius);
  Serial.println("\t deg Celsius");

  //sht75
  unsigned long curMillis = millis();
  if (curMillis - trhMillis >= TRHSTEP) {      // Time for new measurements?
    measActive = true;
    measType = TEMP;
    sht.meas(TEMP, &rawData, NONBLOCK);        // Start temp measurement
    trhMillis = curMillis;
  }
  if (measActive && sht.measRdy()) {           // Note: no error checking
    if (measType == TEMP) {                    // Process temp or humi?
      measType = HUMI;
      temperature = sht.calcTemp(rawData);     // Convert raw sensor data
      sht.meas(HUMI, &rawData, NONBLOCK);      // Start humidity measurement
    } else {
      measActive = false;
      humidity = sht.calcHumi(rawData, temperature); // Convert raw sensor data
      dewpoint = sht.calcDewpoint(humidity, temperature);
      Serial.print("Temperature = ");   Serial.print(temperature);
      Serial.print(" C, Humidity = ");  Serial.print(humidity);
      Serial.print(" %, Dewpoint = ");  Serial.print(dewpoint);
      Serial.println(" C");
    }
  }
  
  // Print out the gryo values
  gyro.read();
  Serial.print("G: ");
  Serial.print((int)gyro.g.x);
  Serial.print(",");
  Serial.print((int)gyro.g.y);
  Serial.print(",");
  Serial.println((int)gyro.g.z);

  // Print out the accel values
  double acc_data[3];
  accel.get_Gxyz(acc_data);
  Serial.print("A: ");
  float length = 0.;
  for(int i = 0; i < 3; i++){
    length += (float)acc_data[i] * (float)acc_data[i];
    Serial.print(acc_data[i]);
    Serial.print(",");
  }
  length = sqrt(length);
  Serial.print(length);
  Serial.println("");
  
  // Print out the bmp values
  Serial.print("B: ");
  Serial.print(bmp.readTemperature());
  Serial.print(",");
  Serial.print(bmp.readPressure());
  Serial.print(",");
  Serial.print(bmp.readAltitude());
  Serial.println("");
  
  // Print out the compass heading
  getHeading();
  Serial.print("H: ");
  Serial.print(heading);
  Serial.println("");
  
}

void getHeading(){
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
}

