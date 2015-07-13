// Touch screen library with X Y and Z (pressure) readings as well
// as oversampling to avoid 'bouncing'
// (c) ladyada / adafruit
// Code under MIT License

#ifndef _ADAFRUIT_TOUCHSCREEN_H_
#define _ADAFRUIT_TOUCHSCREEN_H_
#include <stdint.h>

class TSPoint {
 public:
  TSPoint(void);
  TSPoint(int16_t x, int16_t y, int16_t z);
  
  bool operator==(TSPoint);
  bool operator!=(TSPoint);

  int16_t x, y, z;
};

class TouchScreen {
 public:
  TouchScreen(uint8_t xp, uint8_t yp, uint8_t xm, uint8_t ym);
  TouchScreen(uint8_t xp, uint8_t yp, uint8_t xm, uint8_t ym, uint16_t rx);

  bool isTouching(void);
  uint16_t pressure(void);
  uint16_t readTouchY();
  uint16_t readTouchX();
  TSPoint getPoint();
  int16_t pressureThreshhold;

private:
  //Input pins
  uint8_t _yp, _ym, _xm, _xp;
	#if defined(__arm__)
	//No port manipulation in DUE and ARM boards
	#elde
		//Input pins port registers
		uint8_t xp_port, yp_port, xm_port, ym_port;
		//Input pins converted to mask
		uint8_t xp_pin, yp_pin, xm_pin ,ym_pin;
	#endif
	uint16_t _rxplate;
  //Internal cleanup function
  void cleanPins(void);
};

#endif
