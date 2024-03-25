# Arduino Nano code 

## Introduction 

### Authors:
- Brycen Hillukka (Spring 2023)
- Yu Sheng Chan   (Spring 2023)

### Spring 2023 
Reads the measured distance from the SF11/C LiDAR sensor using an 
Arduino Nano with the I2C protocol. Connect the SCL connection from 
the SF11/C to pin A5 on the Arduino, and the SDA connection to pin A4.
 
The code for reading from the SF11/C and storing the value on the Arduino
were given in the datasheet for the LiDAR sensor. Refer to Github Datasheet section (SF11 datasheet)

### Fall 2023
Further modifications were made by:
- Yu Sheng Chan   (Fall 2023)
 
New addition: GPS and Electronic Compass module 
 
- Reads the GPS location and Electronic Compass moudle reading to determine the coordinates of a measured point 
  
- Math is done on the AR Headset / UWP side instead of Arduino for quicker transfer rate 

- The Electronic Compass module can be changed to either QMC or HMC chip.
> Refer to the datasheet of the chip itself to see if it requires 3.3V or 5V. A voltage selector is included on the right side of the Arduino PCB, where user can select either 3.3 or 5V.

- If neither chip is used, please use your own library. 
 
> Use a high gain antenna if GPS disconnects frequently or could not connect during outdoor use. 
  
>**Please note that the GPS is intended for outdoor use. It is expected to have no signal when its being used indoors.** 
 
>Please refer to the specifications of the datasheet of GPS for antenna compatibality. 
 
## Before you start 

### Program / hardware needed 

- [Arduino IDE](https://downloads.arduino.cc/arduino-1.8.19-windows.exe).

- Mini-B USB cable 

### Libraries

- Refer to each libraries installation guide below.

#### Library of DFROBOT_QMC5883
* Download the included zip libraries or go to [DFRobot Link](https://github.com/DFRobot/DFRobot_QMC5883) to download the zip folder

* To include the libraries, go to Sketch -> Include Library -> Add ZIP Library and add your downloaded library

* Use #include <DFRobot_QMC5883.h> as your header file


#### Library of MPU6050_light
* To include the libraries, go to Sketch -> Include Library -> Manage Library

* Search for [MPU6050_light](https://github.com/rfetick/MPU6050_light) by rfetick and click install

* Use #include <MPU6050_light.h> as your header file


#### Library of GT-U7 GPS module
* To include the libraries, go to Sketch -> Include Library -> Manage Library

* Search for [TinyGPSPlus](https://github.com/mikalhart/TinyGPSPlus) by Mikah Hart and click install

* Use #include <TinyGPS++.h> as your header file


**NOTE FOR GPS Module Connection**

> * To get a good and stable GPS connection, it is best to use the system outdoors / close to window.

> * Use your OWN location. Find it via [Magnetic Declination Website](http://magnetic-declination.com/) to get your inclination or declination angle.
 
> * Replace your angle at the following line:
>
>  **For example:**
>
>  **float declinationAngle = (X + (Y / 60.0)) / (180 / PI);**
>
>  **St Cloud / Waite Park Angle = 0'38E**
>  
>  ***Replace X and Y with the following angle above***
>
>  **float declinationAngle = (0 + (38.0 / 60.0)) / (180 / PI);**

## Programming / Debugging the Arduino Nano

* Remove **ALL** serial connection before programming. This includes BLE module and GPS sensor. 

* If Arduino errors out during the upload, check if the board used is correct. Make sure the board selected is correct (In this case its Arduino Nano).
> Change the processor to the one with "old bootloader" in its name if it does not program.  

* Make sure the GPS, MPU6050 gyroscopic sensor, and LiDAR module are getting powered (indicated by the LED's).

* Calibrate the compass sensor by turning the entire system 360 degrees. 

* Use the Serial Monitor to observe the data being collected. If data(distance) measured is not consistent when being left idle, reseat the LiDAR cable.


## Purchase link 
Purchase link can be found below if replacement part is needed. 

**Please note that I am not affiliated with any of the listed product, and the link is provided solely for your convenience.**

[**GPS SENSOR**](https://www.amazon.com/Navigation-Satellite-Compatible-Microcontroller-Geekstory/dp/B07PRGBLX7?th=1)

[**BLE module**](https://www.amazon.com/DSD-TECH-Bluetooth-iBeacon-Arduino/dp/B06WGZB2N4)

[**MPU 6050**](https://www.amazon.com/HiLetgo-MPU-6050-Accelerometer-Gyroscope-Converter/dp/B01DK83ZYQ/ref=asc_df_B01DK83ZYQ/?tag=hyprod-20&linkCode=df0&hvadid=642109977814&hvpos=&hvnetw=g&hvrand=8430803264096386904&hvpone=&hvptwo=&hvqmt=&hvdev=c&hvdvcmdl=&hvlocint=&hvlocphy=1020086&hvtargid=pla-1260970162741&gclid=EAIaIQobChMI7fH7s9OPggMV_1J_AB3O2gLQEAQYASABEgKCrPD_BwE&th=1)

[**HMC 5883**](https://amazon.com/HiLetgo-GY-271-QMC5883L-Compass-Magnetometer/dp/B008V9S64E/ref=asc_df_B008V9S64E/?tag=hyprod-20&linkCode=df0&hvadid=241933244562&hvpos=&hvnetw=g&hvrand=4729607995161709394&hvpone=&hvptwo=&hvqmt=&hvdev=c&hvdvcmdl=&hvlocint=&hvlocphy=1020086&hvtargid=pla-650173234261&psc=1)

[**Arduino NANO Microcontroller**](https://store.arduino.cc/products/arduino-nano)

[**SF11/C LiDAR Sensor**](https://www.mouser.com/ProductDetail/LightWare-LiDAR/SF11-C?qs=iLbezkQI%252BsjRfbQfbtoV7g%3D%3D&mgh=1&gad_source=1&gclid=EAIaIQobChMIi9Sq6PL3gwMVoVdHAR0QQQRgEAQYAiABEgKM2_D_BwE)
