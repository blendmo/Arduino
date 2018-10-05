#include <Wire.h> // Enable this line if using Arduino Uno, Mega, etc.
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include <i2ckeypad.h>
#include <Stepper.h>
#include <EEPROM.h>

#define ROWS 4
#define COLS 3


// ***************************************************************************************** 
// Global Declarations
// ***************************************************************************************** 

// set I2C addresses (no conflicts)
#define PCF8574_ADDR   0x20  // With A0, A1 and A2 of PCF8574 to ground I2C address is 0x20
#define SEG_DISP_ADDR 0x70

// Get instances of keypad
i2ckeypad kpd = i2ckeypad(PCF8574_ADDR, ROWS, COLS);
Adafruit_7segment matrix = Adafruit_7segment();

// Stepper Motor instance
const unsigned int stepsPerRevolution = 200;  
Stepper myStepper(stepsPerRevolution, 8, 10, 9, 11);
  const unsigned int motorRevSteps = stepsPerRevolution;

// matrix index
int matrixIndex = 0;
const unsigned int eepromLockAddr = 0;
const unsigned int eepromDataAddr = 1;
const unsigned int delayTime = 10; // ms
const unsigned int motorSwitchPin = 7;
const unsigned int motorDelay = 100;

void playKeySound()
{
  // play sound when key is pressed
  tone(3, 2000, 50);
}

void setup() {

  Serial.begin(9600);

  // initialize keypad
  kpd.init();

  // initialize 7seg display
  matrix.begin(SEG_DISP_ADDR);

  // force eeprom lock code to be all zeros
  for (int i = 0; i < 4; ++i)
  {
    EEPROM.write(eepromDataAddr + i, 0);
  }

  // finish stepper initialization
  pinMode(motorSwitchPin, OUTPUT);
  myStepper.setSpeed(60);
  myStepper.step(0);   

  Serial.println("Unlock code set to 0000");

  // enable motor
  digitalWrite(motorSwitchPin, LOW);
  delay(motorDelay);
}

void loop() {
  // put your main code here, to run repeatedly:
  char key = kpd.get_key();

  // check if key was pressed from keypad
  if(key != '\0') 
  {
    if (key == '*')
    {
      playKeySound();
      // move bolt out
      myStepper.step(motorRevSteps);
      Serial.println("Nudge bolt out");
    }
    else if (key == '#')
    {
      playKeySound();
      // move bolt in
      myStepper.step(-motorRevSteps);
      Serial.println("Nudge bolt out");
    }
  }

  delay(delayTime);
}
