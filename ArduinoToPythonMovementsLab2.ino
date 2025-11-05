// Serial-buttons.ino
//change the PINS below in accordance to design
//add library for Gyroscope knob controller, can be done through arduino
//
//
//
//Micro2 Lab2
//Olivia Pham & Ian Khoo

//this site is for the gyro
//https://howtomechatronics.com/tutorials/arduino/arduino-and-mpu6050-accelerometer-and-gyroscope-tutorial/
//more simple site for gyro
//https://projecthub.arduino.cc/Nicholas_N/how-to-use-the-accelerometer-gyroscope-gy-521-647e65

//this site is for the joystick, and where the correction 128 is from
//https://projecthub.arduino.cc/hibit/using-joystick-module-with-arduino-0ffdd4

#include <Wire.h>
#include "FastIMU.h"

#define IMU_ADDR 0x68

MPU6500 IMU;
calData calib = {0};
AccelData accelData;

const int bzrAlert = 8;

const int JoyPinSW = 0; //joystick main pin
const int YcoordPIN = 1; //Y coord pin in
const int XcoordPIN = 2; //X coord pin in

int buzzer_status = LOW;
int incomingByte = 0;

char currentDirection = ' ';

int TrueX;
int TrueY;
//----------------------------------------------------------------
//1 for joystick, 2 for gyro
int JoyORGyro = 1;
//----------------------------------------------------------------
void setup() {
  //joystick
  pinMode(JoyPinSW, INPUT);
  digitalWrite(JoyPinSW, HIGH);
  Serial.begin(9600);

  pinMode(bzrAlert, OUTPUT);
  digitalWrite(bzrAlert, LOW); 

  //gyroscope
  Wire.begin(); // Arduino Mega: SDA = 20, SCL = 21
//  Serial.begin(9600);

  IMU.init(calib, IMU_ADDR);
  IMU.setAccelRange(4);  // Adjust sensitivity as needed
}

void loop() {
  //select the mode
  //Serial.println("input what mode to use: \n");
  //Serial.println("1 = use the joystick \n");
  //Serial.println("2 = use the gyro \n");


  //read inputs including the "apple eaten"
    if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();
    Serial.println(incomingByte);
    
    if(incomingByte == 'A') {
      digitalWrite(bzrAlert, HIGH);
      delay(100);
      digitalWrite(bzrAlert, LOW);
    }
    //set to joystick
    if(incomingByte == '1') {
      JoyORGyro = 1;
      Serial.println("Now using Joystick");
    }
    //set to gyro
    if(incomingByte == '2') {
      JoyORGyro = 2;
      Serial.println("Now using Gyro");
    }
  }
  
  //Joystick controls
  while(JoyORGyro == 1) {
  TrueX = analogRead(XcoordPIN);
  TrueY = analogRead(YcoordPIN);
  //TrueX = analogRead(XcoordPIN) - 128;
  //TrueY = analogRead(YcoordPIN) - 128;
  //possibly might need the -128 to correct the reading to 0
  
  //code here is for the joystick to show current location in serial monitor 

  /* uncomment this in order to find values
  Serial.print("Switch: ");
  Serial.print(digitalRead(JoyPinSW));
  Serial.print("\n");
  Serial.print("X-axis: ");
  Serial.print(analogRead(XcoordPIN));
  Serial.print("\n");
  Serial.print("Y-axis: ");
  Serial.print(analogRead(YcoordPIN));
  Serial.print("\n\n");
  delay(500);
  */

//figure out what the quadrants are using the joystick in order to figure out what if ( && ) statements should be
//if (((TrueX < value) && (value < TrueX)) && ((TrueY < value) && (value < TrueY)))
//
//wires are on the left of the joystick for this config
  //lowerquadrant
  if (((300 < TrueX) && (TrueX < 700)) && ((701 < TrueY) && (TrueY < 1030))) {
      currentDirection = 's';
      delay(150); 
  }
  //leftquadrant
  if (((0 < TrueX) && (TrueX < 299)) && ((300 < TrueY) && (TrueY < 700))) {
      currentDirection = 'a';
      delay(150);
  }
  //upperquadrant
  if (((300 < TrueX) && (TrueX < 700)) && ((0 < TrueY) && (TrueY < 299))) {
      currentDirection = 'w';
      delay(150);
  }
  //Rightquadrant
  if (((701 < TrueX) && (TrueX < 1030)) && ((300 < TrueY) && (TrueY < 700))) {
      currentDirection = 'd';
      delay(150);
  }

  Serial.println(currentDirection);
  Serial.println("\n");
  delay(150); 
  }

//Gyro Controls
//wires are to the right of the gyro in this config (all wires are "bottom-right", in relation to "up")
  while(JoyORGyro == 2) {
  IMU.update();
  IMU.getAccel(&accelData);


  float ax = accelData.accelX;
  float ay = accelData.accelY;

  if (((-0.4 < ax) && (ax < 0)) && ((1.75 < ay) && (ay < 2.05))) {
      currentDirection = 'w';
      delay(150);
  }
  //leftquadrant
  if (((0 < ax) && (ax < 0.4)) && ((2.06 < ay) && (ay < 2.34))) {
      currentDirection = 'a';
      delay(150);
  }
  //Lowerquadrant
  if (((-0.4 < ax) && (ax < 0)) && ((2.4 < ay) && (ay < 2.7))) {
      currentDirection = 's';
      delay(150); 
  }
  //Rightquadrant
  if (((-1.1 < ax) && (ax < -0.4)) && ((2.06 < ay) && (ay < 2.39))) {
      currentDirection = 'd';
      delay(150);

  }  

  Serial.println(currentDirection);
  Serial.println("\n");
  delay(150); 
  }


}
