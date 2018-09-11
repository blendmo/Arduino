#include <Wire.h> // Enable this line if using Arduino Uno, Mega, etc.
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include <i2ckeypad.h>

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

// matrix index
int matrixIndex = 4;

// ***************************************************************************************** 
// Setup activities
// ***************************************************************************************** 
void setup() {
  Serial.begin(9600);

  Wire.begin();

  // initialize keypad
  kpd.init();

  // initialize 7seg display
  matrix.begin(SEG_DISP_ADDR);
}

// ***************************************************************************************** 
// Looping activities
// *****************************************************************************************
void loop() {
  char key = kpd.get_key();
  String dispNumber(key);

  // debug
  if(key != '\0') {
        Serial.print(dispNumber.toInt());
        matrix.writeDigitNum(matrixIndex, dispNumber.toInt(), false);
        matrix.writeDisplay();
        delay(10);

        // handle the goofiness of walking across the 7 segment display
        if (matrixIndex == 0)
        {
          matrixIndex = 4;
        }
        else if(matrixIndex == 3)
        {
          matrixIndex = 1;
        }
        else
        {
          --matrixIndex;
        }

        // play sound when key is pressed
        tone(3, 2000, 50);
  }
 

}
