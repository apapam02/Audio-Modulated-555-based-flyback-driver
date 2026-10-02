/*
  PWM Signal Generator for the Musical Flyback Driver
  ---------------------------------------------------
  Outputs a PWM signal on pin 9 (Timer1) that drives the 555's RESET pin,
  gating the flyback carrier. Frequency and duty cycle are set from the
  Serial Monitor (9600 baud, line ending: Newline).

  - Frequency: 1 Hz to 1 MHz  -> sets the pitch of the arc
  - Duty cycle: 0 to 100 %    -> sets burst length, i.e. arc intensity / volume

  Requires the TimerOne library.
*/

#include <TimerOne.h>

const uint8_t PWM_PIN = 9;                   // Timer1 output, wired to 555 pin 4

const unsigned long MAX_FREQUENCY = 1000000; // 1 MHz
const unsigned long MIN_FREQUENCY = 1;       // 1 Hz
const unsigned int  MAX_DUTY      = 100;     // %
const unsigned int  MIN_DUTY      = 0;       // %
const float         MIN_ON_TIME   = 0.2;     // µs, shortest allowed on/off time

unsigned long frequency = 1000;              // default 1 kHz
unsigned int  dutyCycle = 50;                // default 50 %

void setup() {
  delay(200);                                // let serial settle, avoids garbage output
  Serial.begin(9600);
  pinMode(PWM_PIN, OUTPUT);

  Timer1.initialize(periodMicros(frequency));
  applyPWM();

  Serial.println(F("Signal generator 1 Hz - 1 MHz, 0-100% duty, min Ton/Toff 0.2 us"));
  printStatus();
}

void loop() {
  unsigned long newFrequency = readFrequency();
  unsigned int  newDuty      = readDutyCycle();

  if (timingIsValid(newFrequency, newDuty)) {
    frequency = newFrequency;
    dutyCycle = newDuty;
    applyPWM();
    Serial.println(F("Output updated:"));
    printStatus();
  } else {
    Serial.println(F("On/Off time too short. Output unchanged, please try again."));
  }
}

// Blocks until a line is entered, returns it as an integer
long readNumber() {
  while (Serial.available() == 0) { }
  String input = Serial.readStringUntil('\n');
  input.trim();
  return input.toInt();
}

unsigned long readFrequency() {
  while (true) {
    Serial.print(F("Enter frequency 1 to 1000000 [Hz]: "));
    long value = readNumber();
    Serial.print(value);
    Serial.println(F(" Hz"));
    if (value >= (long)MIN_FREQUENCY && value <= (long)MAX_FREQUENCY) {
      return (unsigned long)value;
    }
    Serial.println(F("Frequency out of range, please try again."));
  }
}

unsigned int readDutyCycle() {
  while (true) {
    Serial.print(F("Enter duty cycle 0 to 100 [%]: "));
    long value = readNumber();
    Serial.print(value);
    Serial.println(F(" %"));
    if (value >= (long)MIN_DUTY && value <= (long)MAX_DUTY) {
      return (unsigned int)value;
    }
    Serial.println(F("Duty cycle out of range, please try again."));
  }
}

// On and off times must both be >= MIN_ON_TIME, unless the output is fully on or off
bool timingIsValid(unsigned long f, unsigned int duty) {
  if (duty == 0 || duty == 100) return true;
  float onTime  = 1000000.0f * (duty / 100.0f) / f;
  float offTime = 1000000.0f * (1.0f - duty / 100.0f) / f;
  return onTime >= MIN_ON_TIME && offTime >= MIN_ON_TIME;
}

unsigned long periodMicros(unsigned long f) {
  return 1000000UL / f;
}

void applyPWM() {
  // TimerOne duty range is 0-1023
  unsigned int duty10bit = (unsigned long)dutyCycle * 1023UL / 100UL;
  Timer1.pwm(PWM_PIN, duty10bit, periodMicros(frequency));
}

void printStatus() {
  float onTime  = 1000000.0f * (dutyCycle / 100.0f) / frequency;
  float offTime = 1000000.0f * (1.0f - dutyCycle / 100.0f) / frequency;
  Serial.print(F("Frequency: "));
  Serial.print(frequency);
  Serial.print(F(" Hz, Duty: "));
  Serial.print(dutyCycle);
  Serial.print(F(" %, Ton: "));
  Serial.print(onTime);
  Serial.print(F(" us, Toff: "));
  Serial.print(offTime);
  Serial.println(F(" us"));
}
