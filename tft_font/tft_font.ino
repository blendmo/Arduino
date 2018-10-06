#include <Adafruit_GFX.h>    // Core graphics library
#include <MCUFRIEND_kbv.h>   // Hardware-specific library
MCUFRIEND_kbv tft;

#include <Fonts/FreeMonoBold12pt7b.h>

#include <FreeDefaultFonts.h>

#define BLACK   0x0000
#define RED     0xF800
#define GREEN   0x07E0
#define WHITE   0xFFFF
#define GREY    0x8410

void setup(void)
{
    Serial.begin(9600);
    uint16_t ID = tft.readID();
    tft.begin(ID);
    tft.setRotation(1);
    tft.fillScreen(BLACK);

    char testMsg[] = "Good morning. Rise and Smile. It's going to be another great day.  Just keep on doing what you do best, and bring a smile to everyone's face.";

    showmsgXY(0, 20, 1, &FreeMonoBold12pt7b, testMsg);
}

void loop(void)
{
}

void showmsgXY(int x, int y, int sz, const GFXfont *f, const char *msg)
{
    int16_t x1, y1;
    uint16_t wid, ht;
    //tft.drawFastHLine(0, y, tft.width(), WHITE);
    tft.setFont(f);
    tft.setCursor(x, y);
    tft.setTextColor(WHITE);
    tft.setTextSize(sz);
    tft.print(msg);
    delay(1000);
}
