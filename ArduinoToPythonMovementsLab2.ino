// Serial-buttons.ino
//change the PINS below in accordance to design
const int btnUp = 2;
const int btnLeft = 3;
const int btnDown = 4;
const int btnRight = 5;
const int bzrAlert - 6;
int buzzer_status = LOW;
int incomingByte = 0;

void setup() {
  Serial.begin(9600);
  pinMode(btnUp, INPUT_PULLUP);
  pinMode(btnLeft, INPUT_PULLUP);
  pinMode(btnDown, INPUT_PULLUP);
  pinMode(btnRight, INPUT_PULLUP);
  pinMode(bzrAlert, OUTPUT);
  digitalWrite(bzrAlert, LOW); 
}

void loop() {
  if (digitalRead(btnUp) == LOW) {
    Serial.println("w\n");
    delay(150); 
  }
  if (digitalRead(btnLeft) == LOW) {
    Serial.println("a\n");
    delay(150);
  }
  if (digitalRead(btnDown) == LOW) {
    Serial.println("s\n");
    delay(150);
  }
  if (digitalRead(btnRight) == LOW) {
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
