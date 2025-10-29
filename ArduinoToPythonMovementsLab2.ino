// Serial-buttons.ino
//change the PINS below in accordance to design
//add library for Gyroscope knob controller, can be done through arduino

#include <MPU9250_asukiaa.h>

MPU9250_asukiaa mpu;

//const int btnUp = 2;
//const int btnLeft = 3;
//const int btnDown = 4;
//const int btnRight = 5;
const int bzrAlert - 6;

const int JoyPinSW = 1; //joystick main pin
const int YcoordPIN = 0; //Y coord pin in
const int XcoordPIN = 2; //X coord pin in


int buzzer_status = LOW;
int incomingByte = 0;

void setup() {
  pinMode(JoyPinSW, INPUT);
  digitalWrite(JoyPinSW, HIGH);
  Serial.begin(9600);

  pinMode(bzrAlert, OUTPUT);
  digitalWrite(bzrAlert, LOW); 
}

void loop() {

  //code here is for the joystick to show current location in serial monitor 
  Serial.print("Switch: ");
  Serial.print(digitalRead(JoyPinSW));
  Serial.print("\n");
  Serial.print("X-axis: ");
  Serial.print(analogRead(XcoordPIN));
  Serial.print("\n");
  Serial.print("Y-axis: ");
  Serial.print(analogRead(YcoordPIN));
  Serial.print("\n\n");
  delay(500)


//figure out what the quadrants are using the joystick in order to figure out what if ( && ) statements should be
  if (Upperquadrant) {
    Serial.println("w\n");
    delay(150); 
  }
  if (LeftQuadrant) {
    Serial.println("a\n");
    delay(150);
  }
  if (LowerQuadrant) {
    Serial.println("s\n");
    delay(150);
  }
  if (RightQuadrant) {
    Serial.println("d\n");
    delay(150);
  }

  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();
    Serial.println(incomingByte);
    
    if(incomingByte == 'A') {
      digitalWrite(bzrAlert, HIGH);
      delay(100);
      digitalWrite(bzrAlert, LOW);
    }
  }
  
}
