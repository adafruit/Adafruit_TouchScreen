#include <WProgram.h>

class Point {
 public:
  Point(void);
  Point(int16_t x, int16_t y, int16_t z);
  int16_t x, y, z;
};

class TouchScreen {
 public:
  TouchScreen(uint8_t xp, uint8_t yp, uint8_t xm, uint8_t ym);
  TouchScreen(uint8_t xp, uint8_t yp, uint8_t xm, uint8_t ym, uint16_t rx);

  boolean isTouching(void);
  uint16_t pressure(void);
  int readTouchY();
  int readTouchX();
  Point getPoint();
  int16_t pressureThreshhold;

private:
  uint8_t _yp, _ym, _xm, _xp;
  uint16_t _rxplate;
};
