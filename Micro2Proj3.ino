/*
  Ian Khoo & Olivia Pham
  Lab 3: Audio-Driven Kinetic Sculpture (Breath-Activated Version)
  EECE.4520 Microprocessor II and Embedded System Design
  
  Hardware connections(Arduino UNO-style):

  MOTOR / L293D
    EN1 (Enable 1,2)  -> D5 (PWM)
    IN1               -> D4
    IN2               -> D3
    OUT1              -> Motor +
    OUT2              -> Motor -
    +Vmotor           -> +5V (or external motor supply)
    L293D GND pins    -> GND

  LCD (16x2, 4-bit mode)
    RS  -> D7
    E   -> D8
    D4  -> D9
    D5  -> D10
    D6  -> D11
    D7  -> D12
    R/W -> GND
    VSS -> GND
    VDD -> +5V
    VO  -> pot wiper (for contrast)
    A   -> +5V (backlight)
    K   -> GND (backlight)

  DS1307 RTC
    VCC -> +5V
    GND -> GND
    SDA -> SDA (A4 on UNO)
    SCL -> SCL (A5 on UNO)

  Button (4-pin pushbutton)
    A-D -> D2
    B-C -> D6
    (D6 used as "ground" output, D2 as input with pull-up)

  Sound Sensor (KY-038 style)
    AO  -> A0
    DO  -> D13 (optional, not used here)
    +   -> +5V
    G   -> GND
*/

#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal.h>

// ---------------- Pin Definitions ----------------
const int PIN_MOTOR_EN   = 5;   // L293D EN1
const int PIN_MOTOR_IN1  = 4;   // L293D IN1
const int PIN_MOTOR_IN2  = 3;   // L293D IN2

const int PIN_BUTTON_IN  = 2;   // Button side A-D
const int PIN_BUTTON_REF = 6;   // Button side B-C (held LOW)

const int PIN_SOUND_AO   = A0;  // Analog from sound sensor
const int PIN_SOUND_DO   = 13;  // Digital out (unused here)

// LCD pins: RS, E, D4, D5, D6, D7
const int PIN_LCD_RS = 7;
const int PIN_LCD_E  = 8;
const int PIN_LCD_D4 = 9;
const int PIN_LCD_D5 = 10;
const int PIN_LCD_D6 = 11;
const int PIN_LCD_D7 = 12;

// ---------------- Globals ----------------
RTC_DS1307 rtc;
LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_E,
                  PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);

// time kept in software after initial read from RTC
volatile uint8_t curHour   = 0;
volatile uint8_t curMinute = 0;
volatile uint8_t curSecond = 0;

// flag set every second in timer ISR
volatile bool oneSecondElapsed = false;

// motor control state
// speedStep: 0 = stop, 1 = 1/2, 2 = 3/4, 3 = full
int speedStep = 0;
bool directionCW = true;   // true = clockwise ("C"), false = counter-clockwise ("CC")

// breath/sound control - keeps motor running after breath stops
int lastSpeedStep = 0;
unsigned long lastSoundTime = 0;
const unsigned long SOUND_TIMEOUT = 2000; // motor keeps running 2 seconds after breath

// button debounce
bool buttonState      = HIGH; // using INPUT_PULLUP logic
bool lastButtonRead   = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50; // ms

// ---------------- Function Prototypes ----------------
void setupTimer1();
void updateMotorOutputs();
void handleButton();
int  readSoundLevel();
void updateSpeedFromSound(int level);
void updateLCD(uint8_t h, uint8_t m, uint8_t s);

// ---------------- SETUP ----------------
void setup() {
  // Motor pins
  pinMode(PIN_MOTOR_EN, OUTPUT);
  pinMode(PIN_MOTOR_IN1, OUTPUT);
  pinMode(PIN_MOTOR_IN2, OUTPUT);

  // Button pins
  pinMode(PIN_BUTTON_IN, INPUT_PULLUP);  // internal pull-up
  pinMode(PIN_BUTTON_REF, OUTPUT);
  digitalWrite(PIN_BUTTON_REF, LOW);     // act as "ground" for button

  // Sound sensor
  pinMode(PIN_SOUND_AO, INPUT);
  pinMode(PIN_SOUND_DO, INPUT); // optional, not used here

  // LCD
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Lab 3 Kinetic");
  lcd.setCursor(0, 1);
  lcd.print("Sculpture init");

  // RTC
  Wire.begin();
  if (!rtc.begin()) {
    lcd.clear();
    lcd.print("RTC not found!");
    while (1) {
      // halt here if RTC not found
    }
  }

  DateTime now = rtc.now();
  noInterrupts();
  curHour   = now.hour();
  curMinute = now.minute();
  curSecond = now.second();
  interrupts();

  // Timer1 for 1 Hz interrupt
  setupTimer1();

  // Initialize motor stopped
  speedStep   = 0;
  directionCW = true;
  updateMotorOutputs();

  delay(1000);
  lcd.clear();
}

// ---------------- MAIN LOOP ----------------
void loop() {
  // 1) Handle direction button (toggle CW/CCW)
  handleButton();

  // 2) Read sound/breath level and map to speed step
  int soundLevel = readSoundLevel();
  updateSpeedFromSound(soundLevel);
  updateMotorOutputs();

  // 3) Once per second, update LCD clock and motor info
  if (oneSecondElapsed) {
    noInterrupts();
    uint8_t h = curHour;
    uint8_t m = curMinute;
    uint8_t s = curSecond;
    oneSecondElapsed = false;
    interrupts();

    updateLCD(h, m, s);
  }
}

// ---------------- TIMER1 SETUP ----------------
// Configure Timer1 to trigger ISR once per second
void setupTimer1() {
  noInterrupts();

  TCCR1A = 0;      // normal operation
  TCCR1B = 0;
  TCNT1  = 0;

  // For 16 MHz clock:
  // prescaler = 1024, OCR1A = 15624 -> 1 Hz
  OCR1A = 15624;
  TCCR1B |= (1 << WGM12);              // CTC mode
  TCCR1B |= (1 << CS12) | (1 << CS10); // prescaler 1024
  TIMSK1 |= (1 << OCIE1A);             // enable Timer1 compare interrupt

  interrupts();
}

// TIMER1 Compare A Interrupt: fires every second
ISR(TIMER1_COMPA_vect) {
  // increment software clock
  curSecond++;
  if (curSecond >= 60) {
    curSecond = 0;
    curMinute++;
    if (curMinute >= 60) {
      curMinute = 0;
      curHour++;
      if (curHour >= 24) {
        curHour = 0;
      }
    }
  }
  oneSecondElapsed = true;
}

// ---------------- BUTTON HANDLING ----------------
void handleButton() {
  int reading = digitalRead(PIN_BUTTON_IN);

  if (reading != lastButtonRead) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      // Button is active-low; trigger on the press (LOW)
      if (buttonState == LOW) {
        directionCW = !directionCW; // toggle direction
      }
    }
  }

  lastButtonRead = reading;
}

// ---------------- SOUND / BREATH SENSING ----------------
// Read maximum value to detect breath/air movement
int readSoundLevel() {
  int maxValue = 0;
  
  // Sample 100 times and find the peak
  for (int i = 0; i < 100; i++) {
    int reading = analogRead(PIN_SOUND_AO);
    if (reading > maxValue) {
      maxValue = reading;
    }
  }
  
  return maxValue;
}

// Map breath level to speed steps with persistence
void updateSpeedFromSound(int level) {
  // Thresholds for breath activation
  int newStep = 0;

  if (level < 200) {
    newStep = 0;       // No breath detected
  } else if (level < 500) {
    newStep = 1;       // Light breath -> 1/2 speed
  } else if (level < 800) {
    newStep = 2;       // Medium breath -> 3/4 speed
  } else {
    newStep = 3;       // Strong breath -> Full speed
  }

  // If breath detected, update speed and timestamp
  if (newStep > 0) {
    speedStep = newStep;
    lastSoundTime = millis();
    lastSpeedStep = newStep;
  } 
  // If no breath but recent breath was detected, keep running
  else if (millis() - lastSoundTime < SOUND_TIMEOUT) {
    speedStep = lastSpeedStep; // Maintain last speed
  }
  else {
    speedStep = 0;
  }
}

// ---------------- MOTOR CONTROL ----------------
void updateMotorOutputs() {
  int pwmValue;

  switch (speedStep) {
    case 0:  pwmValue = 0;   break;  // stop
    case 1:  pwmValue = 128; break;  // ~1/2
    case 2:  pwmValue = 192; break;  // ~3/4
    case 3:  pwmValue = 255; break;  // full
    default: pwmValue = 0;   break;
  }

  // Set direction lines
  if (speedStep == 0) {
    // When stopped, set both LOW (coast)
    digitalWrite(PIN_MOTOR_IN1, LOW);
    digitalWrite(PIN_MOTOR_IN2, LOW);
  } else if (directionCW) {
    digitalWrite(PIN_MOTOR_IN1, HIGH);
    digitalWrite(PIN_MOTOR_IN2, LOW);
  } else {
    digitalWrite(PIN_MOTOR_IN1, LOW);
    digitalWrite(PIN_MOTOR_IN2, HIGH);
  }

  // Enable / speed via PWM
  analogWrite(PIN_MOTOR_EN, pwmValue);
}

// ---------------- LCD DISPLAY ----------------
void updateLCD(uint8_t h, uint8_t m, uint8_t s) {
  lcd.setCursor(0, 0);
  lcd.print("Time ");

  if (h < 10) lcd.print('0');
  lcd.print(h);
  lcd.print(':');
  if (m < 10) lcd.print('0');
  lcd.print(m);
  lcd.print(':');
  if (s < 10) lcd.print('0');
  lcd.print(s);

  // Clear rest of first line if needed
  lcd.print("   ");

  // Second line: direction + speed
  lcd.setCursor(0, 1);
  lcd.print("Dir:");

  if (directionCW) {
    lcd.print("C ");
  } else {
    lcd.print("CC");
  }

  lcd.print(" Spd:");

  const char *speedText;
  switch (speedStep) {
    case 0: speedText = "0";     break;
    case 1: speedText = "1/2";   break;
    case 2: speedText = "3/4";   break;
    case 3: speedText = "Full";  break;
    default: speedText = "0";    break;
  }

  lcd.print(speedText);

  lcd.print("    ");
}
