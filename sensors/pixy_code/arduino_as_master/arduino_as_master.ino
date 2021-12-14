#include <Wire.h>
#include <SoftwareSerial.h>
#include <ComponentObject.h>
#include <RangeSensor.h>
#include <SparkFun_VL53L1X.h>
#include <vl53l1x_class.h>
#include <vl53l1_error_codes.h>
#include "SparkFun_VL53L1X.h"

#include <Pixy2.h>

#define SHUTDOWN_PIN 2
#define INTERRUPT_PIN 3
SFEVL53L1X distanceSensor(Wire, SHUTDOWN_PIN, INTERRUPT_PIN);

Pixy2 pixy;

void setup()
{
  Wire.begin();                // join i2c bus as master
  pixy.init();
  pixy.changeProg("block");
  if (distanceSensor.init() == false) 
    Serial.println("Sensor online!");
}

void loop() 
{ 
  byte sensor_type = Wire.requestFrom(102, 1);
  Serial.println(sensor_type);
//  if (sensor_type == 0) {
//    // PIXY
//    uint16_t blocks = pixy.ccc.getBlocks();
//    if (blocks) { // change the frequency of printing 
//      int16_t mx = pixy.ccc.blocks[0].m_x;
//    
//      byte * data = (byte *) &mx; // prepare data and ship
//      Wire.beginTransmission(102); // transmit to device #4
//      Wire.write(data, 1);
//      Wire.endTransmission();    // stop transmitting
//    }
//  } else {
//    // Distance Sensor
//    distanceSensor.startRanging(); //Write configuration bytes to initiate measurement
//    while (!distanceSensor.checkForDataReady())
//    {
//      delay(1);
//    }
//    int distance = distanceSensor.getDistance(); //Get the result of the measurement from the sensor
//    distanceSensor.clearInterrupt();
//    distanceSensor.stopRanging();
//    
//    float distanceInches = distance * 0.0393701;
//    float distanceFeet = distanceInches / 12.0;
//    
//    byte * data = (byte *) &distanceInches;
//    
//    Wire.beginTransmission(102); // transmit to device #4
//    Wire.write(data, 1);
//    Wire.endTransmission();    // stop transmitting
//  }
  
  delay(100);
} 