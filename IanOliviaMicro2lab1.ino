#include <Keypad.h>

// ==== Pin Definitions ====
#define Buzzer     10
#define GreenLED   11
#define YellowLED  12
#define RedLED     13

// ==== Keypad wiring ====
const byte KeyRows = 4;
const byte KeyCols = 4;
byte KeyRowPins[KeyRows] = {9, 8, 7, 6};  // rows (top to bottom)
byte KeyColPins[KeyCols] = {5, 4, 3, 2};  // columns (left to right)

// ==== Key layout ====
char hexaKeys[KeyRows][KeyCols] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), KeyRowPins, KeyColPins, KeyRows, KeyCols);

// ==== Timing Variables ====
int redDuration = 0;
int greenDuration = 0;
const int yellowDuration = 3; // fixed 3 seconds

bool redSet = false;
bool greenSet = false;
bool running = false;

void setup() {
  Serial.begin(9600);
  pinMode(Buzzer, OUTPUT);
  pinMode(GreenLED, OUTPUT);
  pinMode(YellowLED, OUTPUT);
  pinMode(RedLED, OUTPUT);

  Serial.println("Traffic Light System Initialized");
  Serial.println("Set Red duration: A-##-#");
  Serial.println("Set Green duration: B-##-#");
  Serial.println("Press * to start once both durations are set.");
}

void loop() {
  if (!running) {
    // Flash red LED until both durations set and * pressed
    static unsigned long lastFlash = 0;
    static bool flashState = false;
    if (millis() - lastFlash >= 1000) {
      lastFlash = millis();
      flashState = !flashState;
      digitalWrite(RedLED, flashState);
    }

    char key = customKeypad.getKey();
    if (key) handleSetupInput(key);
  } 
  else {
    runTrafficLights();
  }
}

void key_input() {
  char keyPress = customKeypad.getKey();
    if (keyPress) {

      if (keyPress == "1"){
        Serial.println("1");
      }

      if (keyPress == "2"){
        Serial.println("2");
      }

      if (keyPress == "3"){
        Serial.println("3");
      }

      if (keyPress == "4"){
        Serial.println("4");
      }

      if (keyPress == "5"){
        Serial.println("5");
      }

      if (keyPress == "6"){
        Serial.println("6");
      }

      if (keyPress == "7"){
        Serial.println("7");
      }

      if (keyPress == "8"){
        Serial.println("8");
      }

      if (keyPress == "9"){
        Serial.println("9");
      }

      if (keyPress == "B"){
        Serial.println("B");
      }

      if (keyPress == "C"){
        Serial.println("C");
      }

      if (keyPress == "D"){
        Serial.println("D");
      }

      if (keyPress == "A") {
        Serial.println("A");
      }

      if (keyPress == "B") {
        Serial.println("B");
      }

      if (keyPress == "#") {
        Serial.println("#");
      }

      if (keyPress == "*") {
        Serial.println("*");
      }
    }

    // Set compare match register for 1Hz increments in assembly
    cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  // OCR1A = (16,000,000 / (1 * 1024)) - 1 = 7812 
  // Set the value for .5Hz
  OCR1A = 7812; 
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS12) | (1 << CS10);
  // Enable Timer1 compare match interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei(); 
}

void handleSetupInput(char key) {
  static String inputBuffer = "";
  
  if (key == 'A' || key == 'B') {
    inputBuffer = "";
    inputBuffer += key;
    Serial.print("Setting ");
    Serial.println((key == 'A') ? "Red duration..." : "Green duration...");
  } 
  else if (isdigit(key)) {
    inputBuffer += key;
  } 
  else if (key == '#') {
    if (inputBuffer.length() >= 2) {
      char which = inputBuffer[0];
      int val = inputBuffer.substring(1).toInt();
      if (which == 'A') {
        redDuration = val;
        redSet = true;
        Serial.print("Red Duration Set: ");
        Serial.print(val);
        Serial.println(" seconds");
      } 
      else if (which == 'B') {
        greenDuration = val;
        greenSet = true;
        Serial.print("Green Duration Set: ");
        Serial.print(val);
        Serial.println(" seconds");
      }
    }
  } 
  else if (key == '*') {
    if (redSet && greenSet) {
      Serial.println("Starting Traffic Light Sequence...");
      running = true;
      digitalWrite(RedLED, LOW);
    } else {
      Serial.println("Please set both durations first!");
    }
  }
}

void runTrafficLights() {
  // Red Phase
  trafficLightPhase(RedLED, redDuration);

  // Green Phase
  trafficLightPhase(GreenLED, greenDuration);

  // Yellow Phase (no flashing)
  digitalWrite(GreenLED, LOW);
  digitalWrite(YellowLED, HIGH);
  delay(yellowDuration * 1000);
  digitalWrite(YellowLED, LOW);
}

void trafficLightPhase(int ledPin, int duration) {
  unsigned long startTime = millis();

  while (millis() - startTime < (duration - 3) * 1000UL) {
    digitalWrite(ledPin, HIGH);
  }
  
  // Last 3 seconds flash
  unsigned long flashStart = millis();
  while (millis() - flashStart < 3000) {
    digitalWrite(ledPin, HIGH);
    digitalWrite(Buzzer, HIGH);
    delay(500);
    digitalWrite(ledPin, LOW);
    digitalWrite(Buzzer, LOW);
    delay(500);
  }
}

