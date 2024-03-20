/*
 * Reads the measured distance from the SF11/C LiDAR sensor using an 
 * Arduino Nano using the I2C protocol. Connect the SCL connection from 
 * the SF11/C to pin A5 on the Arduino, and the SDA connection to pin A4.
 * 
 * The code for reading from the SF11/C and storing the value on the Arduino
 * were given in the datasheet for the LiDAR sensor. Refer to Github Datasheet section (SF11 datasheet)
 * 
 * Modifications were made by:
 *  - Brycen Hillukka (Spring 2023) 
 *  - Yu Sheng Chan   (Spring 2023)
 * 
 * 
 * Further modifications were made by:
 *  - Yu Sheng Chan   (Fall 2023)
 *  
 *  
 * New addition: GPS and Electronic Compass module 
 * 
 * Reads tbe GPS location and Electronic Compass moudle reading to determine the coordinates of a measured point 
 * Math is done on the AR Headset / UWP side instead of Arduino for quicker transfer rate 
 * 
 * The Electronic Compass module can be changed to either QMC or HMC chip. 
 * Please follow the datasheet for specific pinout / voltage requirements of the chip. 
 * If neither chip is used, please use your own library. 
 * Refer to READE.MD files attached 
 * 
 * Use a high gain antenna if GPS disconnects frequently or could not connect during outdoor use. 
 * Please note that the GPS is intended for outdoor use. It is expected to have no signal indoors. 
 * Please refer to the specifications of the datasheet of GPS for antenna compatibality. 
 */

// Include necessary libraries 
#include <Wire.h>
#include <MPU6050_light.h>
#include <math.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <DFRobot_QMC5883.h>

// Creates an mpu object
MPU6050 mpu(Wire);                      

//Using pin 2 and 3 as software serial pin
int RXPin = 2;
int TXPin = 3;
SoftwareSerial gpsSerial(RXPin,TXPin);

//Creats gps object
TinyGPSPlus gps;

//Creates compass object 
DFRobot_QMC5883 compass;

float distance;                         // Distance will be in meters
byte a, b;                              // Bytes read by the I2C port


void setup() {
  Serial.begin(9600);                   // Ensure serial monitor set to this value also
  gpsSerial.begin(9600);                // Start serial monitor of gps 
  Wire.begin();                         // Creates a wire object

  //Infinite while loop to check if compass is present
  while (!compass.begin())
  {
    Serial.println("Could not find a valid QMC5883 sensor, check wiring!");
    delay(500);
  }

  //2 Models of compass, will initialize to whichever model it detects
  //Change the Range,Measurements,DataRate and Sampling rate according to datasheet / library file 
  if(compass.isHMC())
  {
        Serial.println("Initialize HMC5883");
        compass.setRange(HMC5883L_RANGE_1_3GA);
        compass.setMeasurementMode(HMC5883L_CONTINOUS);
        compass.setDataRate(HMC5883L_DATARATE_15HZ);
        compass.setSamples(HMC5883L_SAMPLES_8);
  }
  else if(compass.isQMC())
  {
        Serial.println("Initialize QMC5883");
        compass.setRange(QMC5883_RANGE_2GA);
        compass.setMeasurementMode(QMC5883_CONTINOUS); 
        compass.setDataRate(QMC5883_DATARATE_200HZ);
        compass.setSamples(QMC5883_SAMPLES_8);
  }
  
  mpu.begin();                          // Creates connection for writing to the MPU6050
  Serial.println(F("Calculating gyro offset, do not move MPU6050"));
  delay(1000);
  mpu.calcGyroOffsets();                // This does the calibration for MPU6050
  Wire.endTransmission(true);           // Ends transmission with MPU6050
  
  Wire.beginTransmission(0x66);         // Enable transmission to the SF11C
  Wire.write(0);                        // Write the distance location -> 0
  Wire.endTransmission(true);           // End the transmission
}
 
void loop() 
{
  //When GPS is ready to be read, display the data
  while(gpsSerial.available() > 0)
  {
    //Read the data from GPS
    if(gps.encode(gpsSerial.read()))
    {
      //Display the data / send it via Bluetooth 
      displayInfo();
    }
  }

  // If 5000 milliseconds pass and there are no characters coming in
  // over the software serial port, show a "No GPS detected" error
  /*
  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println("No GPS detected");
    while(true);
  } 
  */ 

  Vector norm = compass.readNormalize();

  // Calculate heading
  float heading = atan2(norm.YAxis, norm.XAxis);

  // Set declination angle on your location and fix heading
  // You can find your declination on: http://magnetic-declination.com/
  // (+) Positive or (-) for negative
  // For Bytom / Poland declination angle is 4'26E (positive)
  // Formula: (deg + (min / 60.0)) / (180 / M_PI);
  float declinationAngle = (0 + (38.0 / 60.0)) / (180 / PI);
  heading += declinationAngle;

  // Correct for heading < 0deg and heading > 360deg
  if (heading < 0){
    heading += 2 * PI;
  }

  if (heading > 2 * PI){
    heading -= 2 * PI;
  }

  // Convert to degrees
  float headingDegrees = heading * 180/M_PI; 

  // Output
  //Serial.print(" Heading = ");
  //Serial.print(heading);
  //Serial.print(" Degress = ");
  Serial.print(headingDegrees);
  Serial.println("AN");

  delay(50);
  
  Wire.beginTransmission(0x66);         // Begin transmission with SF11C
  Wire.write(0);                        // 
  Wire.endTransmission();               // End transmission for writing to the SF11C
  Wire.requestFrom(0x66, 2);            // Request 2 bytes from the SF11
  
  while(Wire.available())               // Make sure the SF11 sends the required number of bytes
  {
    a = Wire.read();                    // Fetch the high byte of the distance
    b = Wire.read();                    // Fetch the low byte of the distance
  }

  distance = (float)(a * 256 + b)/100;  // Convert to a floating point value in meters
  
  Serial.print(distance);               // Print the distance to the serial port
  Serial.println("W");
  
  delay(50);

  mpu.begin();                          // Creates connection for writing to the MPU6050
  Wire.write(0);                        //
  Wire.endTransmission();               // Ends transmission with MPU6050
  mpu.update();                         // 
  
  int phi = mpu.getAngleY();            // Measure the angle between x-y plane and point
  int theta = mpu.getAngleZ() + 90;     // Measure the angle between x axis and point
  
  // Get the z distance
  float distanceZ = distance * sin(phi * PI / 180);

  // Get the x-y plane distance
  float distanceXY = distance * cos(phi * PI / 180);

  // Get the y distance
  float distanceY = distanceXY * sin(theta * PI / 180);

  // Get the z distance
  float distanceX = distanceXY * cos(theta * PI / 180);

  
  Serial.print(distanceX);
  Serial.println("X");
  
  delay(50);
  
  Serial.print(distanceY);
  Serial.println("Y");
  
  delay(50);
  
  Serial.print(distanceZ);
  Serial.println("Z");
  
  delay(50);
  
}


void displayInfo()
{
  if (gps.location.isValid())
  {
    //Display latitude
    Serial.print(gps.location.lat(), 6);
    Serial.println("LA");
    
    delay(50);

    //Display longitude 
    Serial.print(gps.location.lng(), 6);
    Serial.println("LO");
    
    delay(50);

    //Display altitude 
    Serial.print(gps.altitude.meters());
    Serial.println("AL");
    
    delay(50);
  }
}
