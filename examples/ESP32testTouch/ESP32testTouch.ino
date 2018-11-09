/* 
Test MCU Friend parallel display and resistive touchscreen by drawing touch points
on screen, use something pointed for more accuracy

Need this modified Touchscreen library and one of:
- TFT_eSPI        much faster for ESP32, must select correct display driver 
- MCUFRIEND_kbv   more display driver support, auto detects display driver
 */

#define TFT_eSPIlib  // comment out to use MCUFRIEND_kbv

#ifdef TFT_eSPIlib
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();
#else
#include <MCUFRIEND_kbv.h> 
MCUFRIEND_kbv tft;
#endif

#include <TouchScreen.h>

// adjust pressure sensitivity - note works 'backwards'
#define MINPRESSURE 200
#define MAXPRESSURE 1000

// some colours to play with
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
 
// Either run TouchScreen_Calibr_native.ino and apply results to the arrays below
// or just use trial and error from drawing on screen
// ESP32 coordinates at default 12 bit resolution have range 0 - 4095
// however the ADC cannot read voltages below 150mv and tops out around 3.15V
// so the actual coordinates will not be at the extremes
const int portCoords[] = {3800, 600, 200, 3900}; // portrait left, right, top, bottom
const int landCoords[] = {150, 3950, 300, 3800}; // landscape left, right, top, bottom

const int landscape = true; //  set to false for portrait

const int XP = 27, XM = 15, YP = 4, YM = 14; // default ESP32 Uno touchscreen pins
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

int pixel_x, pixel_y;     
int screenCoords[4];

void setup() {
    Serial.begin(115200);
    prepTouch();
#ifdef TFT_eSPIlib
    Serial.println("TFT_eSPI library");
    tft.begin();
#else
    Serial.println("MCUFRIEND_kbv library");
    idDisplay();
#endif
    // screen orientation and background
    if (landscape) Serial.println("Landscape"); else Serial.println("Portrait");
    tft.setRotation(landscape);  
    tft.fillScreen(BLACK);
}

void loop() {
    // display touched point with colored dot
    if (Touch_getXY()) tft.fillCircle(pixel_x, pixel_y, 2, YELLOW); 
}


bool Touch_getXY(void) {
    // obtain coordinates of touched point
    TSPoint p = ts.getPoint();
    pinMode(YP, OUTPUT);      //restore shared pins
    pinMode(XM, OUTPUT);
    digitalWrite(YP, HIGH);   //because TFT control pins
    digitalWrite(XM, HIGH);
    bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
    if (pressed) {
        if (landscape) {
          pixel_x = map(p.y, screenCoords[0], screenCoords[1], 0, tft.width()); 
          pixel_y = map(p.x, screenCoords[2], screenCoords[3], 0, tft.height());
        } else {
          pixel_x = map(p.x, screenCoords[0], screenCoords[1], 0, tft.width()); 
          pixel_y = map(p.y, screenCoords[2], screenCoords[3], 0, tft.height());
        }
    }
    return pressed;
}

void prepTouch() {
  // set screen coordinates for orientation setting
  if (landscape) {
    for (int i=0; i<4; i++) screenCoords[i] = landCoords[i];
  } else {
    for (int i=0; i<4; i++) screenCoords[i] = portCoords[i];
  }
}

#ifndef TFT_eSPIlib
void idDisplay() {
    // MCUFRIEND_kbv library only
    uint16_t ID = tft.readID();
    Serial.print("TFT ID = 0x");
    Serial.println(ID, HEX);
    if (ID == 0xD3D3) ID = 0x9486; // write-only shield
    tft.begin(ID);
}
#endif




