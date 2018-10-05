/*
 * This script combines the technologies of a 3x4 Keypad, a stepper motor and a 4-digit
 * seven segment display, to provide a coded locking digital safe.
 * 
 * CONOP:  User enter 4 numbers on the keypad then presses the [#] key, this will
 *         store the 4 digit number in NVM and trigger the motor to engage a locking
 *         bolt.  The user may now enter a new set of 4 digits, and if they match the
 *         locking set, the motor will retract the bolt.  
 *         
 *         The [*] key will clear any starting digit set.  If power is removed while
 *         the safe is in the lock state, it will retain the locked state even when
 *         powered on, and will still require the same password.
 */

#include <Wire.h> // Enable this line if using Arduino Uno, Mega, etc.
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include <i2ckeypad.h>
#include <Stepper.h>
#include <EEPROM.h>

// ***************************************************************************************** 
// Global Declarations
// ***************************************************************************************** 
#define ROWS 4
#define COLS 3

// set I2C addresses (no conflicts)
#define PCF8574_ADDR   0x20  // With A0, A1 and A2 of PCF8574 to ground I2C address is 0x20
#define SEG_DISP_ADDR 0x70

// Get instances of keypad and 7seg display
i2ckeypad kpd = i2ckeypad(PCF8574_ADDR, ROWS, COLS);
Adafruit_7segment matrix = Adafruit_7segment();

// Stepper Motor instance
const unsigned int stepsPerRevolution = 200;  
Stepper myStepper(stepsPerRevolution, 8, 10, 9, 11);
  const unsigned int motorRevSteps = stepsPerRevolution * 4;

// matrix index
int matrixIndex = 0;
bool isLocked = false;
unsigned char numberList[4];
unsigned char lockedList[4];
const unsigned int eepromLockAddr = 0;
const unsigned int eepromDataAddr = 1;
const unsigned int delayTime = 10; // ms
const unsigned int dispTimeout = 1000; // 10 sec
const unsigned int motorSwitchPin = 7;
unsigned int tickCounter = 0;
const unsigned int motorDelay = 100;

enum LockState
{
  LS_IDLE,
  WAIT_FOR_2,
  WAIT_FOR_3,
  WAIT_FOR_4,
  WAIT_FOR_GO,
  LS_RESET
};

LockState state = LS_IDLE;

// store get the flash pointer
// ***************************************************************************************** 
// Helper Functions
// *****************************************************************************************
void writeToSevenSeg(int index, int number)
{
  // remap numbers (reverse order and skipping dots)
  if (index == 0) matrixIndex = 4;
  else if (index == 1) matrixIndex = 3;
  else if (index == 2) matrixIndex = 1;
  else if (index == 3) matrixIndex = 0;
  else return;
  
  Serial.print(number);
  matrix.writeDigitNum(matrixIndex, number, false);
  matrix.writeDisplay();
}

// ***************************************************************************************** 
// Clear the seven segment display
// ***************************************************************************************** 
void clearDisplay()
{
  // write nothing to display
  matrix.clear();
  matrix.writeDisplay();
}

// ***************************************************************************************** 
// Sound played when a key is pressed
// ***************************************************************************************** 
void playKeySound()
{
  // play sound when key is pressed
  tone(3, 2000, 50);
}

// ***************************************************************************************** 
// Sound played when a number entry is incorrect
// ***************************************************************************************** 
void playErrorSound()
{
  // play sound when key is pressed
  tone(3, 500, 100);
}

// ***************************************************************************************** 
// Sound played when a number entry is correct
// ***************************************************************************************** 
void playGoodSound()
{
  // play sound when key is pressed
  tone(3, 5000, 100);
}

// ***************************************************************************************** 
// Setup activities
// ***************************************************************************************** 
void setup() 
{
  Serial.begin(9600);

  // intialize Wire for I2C comms
  Wire.begin();

  // initialize keypad
  kpd.init();

  // initialize 7seg display
  matrix.begin(SEG_DISP_ADDR);
  clearDisplay();

  // get data from NVM (in case power was removed while it was locked"
  isLocked = (EEPROM.read(eepromLockAddr) == 1);

  if (isLocked)
  {
    Serial.println("Safe Came up Locked");
    Serial.print("Lock Combo is: ");
    
    for (int i = 0; i < 4; ++i)
    {
      lockedList[i] = EEPROM.read(eepromDataAddr + i);
      Serial.println(lockedList[i]);
    }
  }

  // finish stepper initialization
  pinMode(motorSwitchPin, OUTPUT);
  myStepper.setSpeed(120);
  myStepper.step(0);   
  digitalWrite(motorSwitchPin, HIGH);  // disable motor power

  // report start
  Serial.println("Start Digital Safe");
}

// ***************************************************************************************** 
// Looping activities
// *****************************************************************************************
void loop() 
{
  char key = kpd.get_key();
  String dispNumber(key);

  // check if key was pressed from keypad
  if(key != '\0') 
  {
    // reset timeout counter every time key is pressed
    tickCounter = 0;

    // state machine switch
    switch(state)
    {
    case LS_IDLE:
      clearDisplay();  // clear seven seg display
      myStepper.step(0);   // stop current to the motor
      
      if (key == '#' || key == '*') 
      {
        state = LS_RESET;  
        break;
      }
      else
      {
        numberList[0] = dispNumber.toInt();
        writeToSevenSeg(0, numberList[0]);
        playKeySound();
        state = WAIT_FOR_2;
      }
      break;
    case WAIT_FOR_2:
      if (key == '#' || key == '*')
      {
        state = LS_RESET;  
        break;
      }
      else
      {
        numberList[1] = numberList[0];
        numberList[0] = dispNumber.toInt();
        writeToSevenSeg(1, numberList[1]);
        writeToSevenSeg(0, numberList[0]);
        playKeySound();
        state = WAIT_FOR_3;
      }      
      break;
    case WAIT_FOR_3:
      if (key == '#' || key == '*')
      {
        state = LS_RESET;  
        break;
      }
      else
      {
        numberList[2] = numberList[1];
        numberList[1] = numberList[0];
        numberList[0] = dispNumber.toInt();
        writeToSevenSeg(2, numberList[2]);
        writeToSevenSeg(1, numberList[1]);
        writeToSevenSeg(0, numberList[0]);
        
        playKeySound();
        state = WAIT_FOR_4;
      }      
      break;
    case WAIT_FOR_4:
      if (key == '#' || key == '*')
      {
        state = LS_RESET;  
        break;
      }
      else
      {
        numberList[3] = numberList[2];
        numberList[2] = numberList[1];
        numberList[1] = numberList[0];
        numberList[0] = dispNumber.toInt();
        writeToSevenSeg(3, numberList[3]);
        writeToSevenSeg(2, numberList[2]);
        writeToSevenSeg(1, numberList[1]);
        writeToSevenSeg(0, numberList[0]);

        if (isLocked)
        {
          // check if code matches
          if (memcmp(lockedList, numberList, 4) == 0)
          {
            //-------------------------------------------------------
            // TRIGGERED THE UN-LOCK STATE
            //-------------------------------------------------------
            playGoodSound();

            // disgenage the locking bolt
            digitalWrite(motorSwitchPin, LOW);
            delay(motorDelay);
            myStepper.step(-motorRevSteps);
            delay(motorDelay);
            digitalWrite(motorSwitchPin, HIGH);
                       
            clearDisplay();
            state = LS_IDLE;
            isLocked = false;

            // store in eeprom
            EEPROM.write(eepromLockAddr, 0);        
          }
          else
          {
            Serial.println("Sorry Wrong Code");
            playErrorSound();

            state = LS_IDLE;
            clearDisplay();
          }
        }
        else
        {
          playKeySound();
          state = WAIT_FOR_GO;
        }        
      }      
      break;
    case WAIT_FOR_GO:
      if (key == '#')
      {              
        //-------------------------------------------------------
        // TRIGGERED THE LOCK STATE
        //-------------------------------------------------------
        playGoodSound();       

        // save the locked state in NVM
        memcpy(lockedList, numberList, 4);
        EEPROM.write(eepromLockAddr, 1);
        for (int i = 0; i < 4; ++i)
        {
          EEPROM.write(eepromDataAddr + i, lockedList[i]);
        }

        // lock the hatch
        digitalWrite(motorSwitchPin, LOW);
        delay(motorDelay);
        myStepper.step(motorRevSteps);  
        delay(motorDelay);
        digitalWrite(motorSwitchPin, HIGH);

        clearDisplay();
        state = LS_IDLE;
        isLocked = true;
      }
      else if (key == '*')
      {
        state = LS_RESET;
      }
      break;
    default:
      Serial.println("DEFAULT");
      state = LS_IDLE;
      break;
    }  // end of switch
  } // if valid key
  else
  {
      ++tickCounter;
  }

  if (state == LS_RESET)
  {
    clearDisplay();
    state = LS_IDLE;
  }

  // check for wait timeout
  if ((tickCounter > dispTimeout) && (state != LS_IDLE))
  {
    state = LS_RESET;
  }

  delay(delayTime);
}
