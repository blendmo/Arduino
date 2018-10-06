#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;
#include <TouchScreen_kbv.h>

// ************************************************************************
// Global Declarations
// ************************************************************************
#define MINPRESSURE 200

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

int XP = 6, XM = A2, YP = A1, YM = 7; //ID=0x9341
TouchScreen_kbv ts(XP, YP, XM, YM, 300);
Adafruit_GFX_Button on_btn, off_btn;
TSPoint_kbv p;
unsigned int downCounter = 0;

// ************************************************************************
// Monitor screen touch
// ************************************************************************
bool Touch_getXY(void)
{
    int count = 0;
    bool state, oldstate;
    while (count < 10) {
      p = ts.getPoint();
      pinMode(YP, OUTPUT);      //restore shared pins
      pinMode(XM, OUTPUT);
      digitalWrite(YP, HIGH);   //because TFT control pins
      digitalWrite(XM, HIGH);

      state = (p.z > MINPRESSURE /*&& p.z < MAXPRESSURE*/);
      //Serial.println(p.z);
      if (state == oldstate) count++;
      else count = 0;
      oldstate = state;
      delay(5);
    }
   
    return oldstate;  
}

// ************************************************************************
// Sensitivity Calibration
// ************************************************************************
boolean diagnose_pins()
{
    int i, j, value, Apins[2], Dpins[2], Values[2], found = 0;
    Serial.println(F("Making all control and bus pins INPUT_PULLUP"));
    Serial.println(F("Typical 30k Analog pullup with corresponding pin"));
    Serial.println(F("would read low when digital is written LOW"));
    Serial.println(F("e.g. reads ~25 for 300R X direction"));
    Serial.println(F("e.g. reads ~30 for 500R Y direction"));
    Serial.println(F(""));
    for (i = A0; i < A5; i++) pinMode(i, INPUT_PULLUP);
    for (i = 2; i < 10; i++) pinMode(i, INPUT_PULLUP);
    for (i = A0; i < A4; i++) {
        pinMode(i, INPUT_PULLUP);
        for (j = 5; j < 10; j++) {
            pinMode(j, OUTPUT);
            digitalWrite(j, LOW);
            value = analogRead(i);               // ignore first reading
            value = analogRead(i);
            if (value < 100 && value > 0) {
                //showpins(i, j, value, "Testing :");
                if (found < 2) {
                    Apins[found] = i;
                    Dpins[found] = j;
                    Values[found] = value;
                }
                found++;
            }
            pinMode(j, INPUT_PULLUP);
        }
        pinMode(i, INPUT_PULLUP);
    }
    if (found == 2) {
        Serial.println(F("Diagnosing as:-"));
        int idx = Values[0] < Values[1];

        XM = Apins[!idx]; XP = Dpins[!idx]; YP = Apins[idx]; YM = Dpins[idx];
        ts = TouchScreen_kbv(XP, YP, XM, YM, 300);    //re-initialise with pins
        return true;                              //success
    }
    Serial.println(F("BROKEN TOUCHSCREEN"));
    return false;
}

// ************************************************************************
// Setup
// ************************************************************************
void setup() {
  Serial.begin(9600);
  
  diagnose_pins(); // calibrates the pressure value 
  
  uint16_t ID = tft.readID();
  tft.begin(ID);
  tft.setRotation(0);            //PORTRAIT
  tft.fillScreen(BLACK);
}

// ************************************************************************
// Looping
// ************************************************************************
void loop() {
  bool down = Touch_getXY();

  if (down)
  {
    Serial.println(++downCounter);
  }
}
