// Touch screen library with X Y and Z (pressure) readings as well
// as oversampling to avoid 'bouncing'
// (c) ladyada / adafruit
// Code under MIT License

#include "pins_arduino.h"
#include "wiring_private.h"
#include <avr/pgmspace.h>
#include "TouchScreen.h"

// increase or decrease the touchscreen oversampling. This is a little different than you make think:
// 1 is no oversampling, whatever data we get is immediately returned
// 2 is double-sampling and we only return valid data if both points are the same
// 3+ uses insert sort to get the median value.
// We found 2 is precise yet not too slow so we suggest sticking with it!

#define NUMSAMPLES 2

TSPoint::TSPoint(void) {
  x = y = 0;
}

TSPoint::TSPoint(int16_t x0, int16_t y0, int16_t z0) {
  x = x0;
  y = y0;
  z = z0;
}

bool TSPoint::operator==(TSPoint p1) {
  return  ((p1.x == x) && (p1.y == y) && (p1.z == z));
}

bool TSPoint::operator!=(TSPoint p1) {
  return  ((p1.x != x) || (p1.y != y) || (p1.z != z));
}

#if (NUMSAMPLES > 2)
static void insert_sort(int array[], uint8_t size) {
  uint8_t j;
  int save;
  
  for (int i = 1; i < size; i++) {
    save = array[i];
    for (j = i; j >= 1 && save < array[j - 1]; j--)
      array[j] = array[j - 1];
    array[j] = save; 
  }
}
#endif

TSPoint TouchScreen::getPoint(void) {
  int x, y, z;
  int samples[NUMSAMPLES];
  uint8_t i, valid;
  
  valid = 1;

  pinMode(_yp, INPUT);
  pinMode(_ym, INPUT);
  #if defined(__arm__)
	digitalWrite(_yp, LOW);
	digitalWrite(_ym, LOW);
  #else
  *portOutputRegister(yp_port) &= ~yp_pin;
  *portOutputRegister(ym_port) &= ~ym_pin;
  #endif
   
	pinMode(_xp, OUTPUT);
	pinMode(_xm, OUTPUT);
  #if defined(__arm__)
	digitalWrite(_xp, HIGH);
	digitalWrite(_xm, LOW);
  #else
	*portOutputRegister(xp_port) |= xp_pin;
	*portOutputRegister(xm_port) &= ~xm_pin;
   #endif
   
   for (i=0; i<NUMSAMPLES; i++) {
     samples[i] = analogRead(_yp);
   }
#if NUMSAMPLES > 2
   insert_sort(samples, NUMSAMPLES);
#endif
#if NUMSAMPLES == 2
   if (samples[0] != samples[1]) { valid = 0; }
#endif
   x = (1023-samples[NUMSAMPLES/2]);

   pinMode(_xp, INPUT);
   pinMode(_xm, INPUT);
   #if defined(__arm__)
		digitalWrite(_xp, LOW);
   #else
		*portOutputRegister(xp_port) &= ~xp_pin;
   #endif
   
   pinMode(_yp, OUTPUT);
	#if defined(__arm__)
		digitalWrite(_yp, HIGH);
	#else
		*portOutputRegister(yp_port) |= yp_pin;
	#endif
   pinMode(_ym, OUTPUT);
  
   for (i=0; i<NUMSAMPLES; i++) {
     samples[i] = analogRead(_xm);
   }

#if NUMSAMPLES > 2
   insert_sort(samples, NUMSAMPLES);
#endif
#if NUMSAMPLES == 2
   if (samples[0] != samples[1]) { valid = 0; }
#endif

   y = (1023-samples[NUMSAMPLES/2]);

   
   pinMode(_xp, OUTPUT);
   #if defined(__arm__)
		digitalWrite(_xp, LOW);
		digitalWrite(_ym, HIGH);
		digitalWrite(_yp, LOW);
	#else
		// Set X+ to ground
		*portOutputRegister(xp_port) &= ~xp_pin;
		// Set Y- to VCC
		*portOutputRegister(ym_port) |= ym_pin;
		// Hi-Z X- and Y+
		*portOutputRegister(yp_port) &= ~yp_pin;
	#endif
	
   pinMode(_yp, INPUT);
  
   uint16_t z1 = analogRead(_xm); 
   uint16_t z2 = analogRead(_yp);

   if (_rxplate != 0) {
     // now read the x 
     float rtouch;
     rtouch = z2;
     rtouch /= z1;
     rtouch -= 1;
     rtouch *= x;
     rtouch *= _rxplate;
     rtouch /= 1024;
     
     z = rtouch;
   } else {
     z = (1023-(z2-z1));
   }

   if (! valid) {
     z = 0;
   }
   
   // Clean pins for LCD fix
   cleanPins();
   
   return TSPoint(x, y, z);
}

TouchScreen::TouchScreen(uint8_t xp, uint8_t yp, uint8_t xm, uint8_t ym) {
  _yp = yp;
  _xm = xm;
  _ym = ym;
  _xp = xp;
  _rxplate = 0;
	#if defined(__arm__)
		//No direct port manipulation on DUE and ARM
	#else
		//Added these to here for speed up the point detection
		xp_port = digitalPinToPort(_xp);
		yp_port = digitalPinToPort(_yp);
		xm_port = digitalPinToPort(_xm);
		ym_port = digitalPinToPort(_ym);

		xp_pin = digitalPinToBitMask(_xp);
		yp_pin = digitalPinToBitMask(_yp);
		xm_pin = digitalPinToBitMask(_xm);
		ym_pin = digitalPinToBitMask(_ym);
	#endif
  
  pressureThreshhold = 10;
}


TouchScreen::TouchScreen(uint8_t xp, uint8_t yp, uint8_t xm, uint8_t ym,
			 uint16_t rxplate) {
  _yp = yp;
  _xm = xm;
  _ym = ym;
  _xp = xp;
  _rxplate = rxplate;
  
	#if defined(__arm__)
		//No direct port manipulation on DUE and ARM
	#else
  
		//Added these to here for speed up the point detection
		xp_port = digitalPinToPort(_xp);
		yp_port = digitalPinToPort(_yp);
		xm_port = digitalPinToPort(_xm);
		ym_port = digitalPinToPort(_ym);

		xp_pin = digitalPinToBitMask(_xp);
		yp_pin = digitalPinToBitMask(_yp);
		xm_pin = digitalPinToBitMask(_xm);
		ym_pin = digitalPinToBitMask(_ym);
	#endif
  pressureThreshhold = 10;
}

uint16_t TouchScreen::readTouchX(void) {
   //Added int to save result
   uint16_t result;
   #if defined(__arm__)
		pinMode(_yp, INPUT);
		pinMode(_ym, INPUT);
		//digitalWrite(_yp, LOW);
		//digitalWrite(_ym, LOW);
		 pinMode(_xp, OUTPUT);
		 digitalWrite(_xp, HIGH);
		 pinMode(_xm, OUTPUT);
		 digitalWrite(_xm, LOW);
		 digitalWrite(_xm, LOW);
	#else 
	
		pinMode(_yp, INPUT);
		pinMode(_ym, INPUT);	
		*portOutputRegister(yp_port) &= ~yp_pin;
		*portOutputRegister(ym_port) &= ~ym_pin;  
		pinMode(_xp, OUTPUT);
		*portOutputRegister(xp_port) |= xp_pin;
		pinMode(_xm, OUTPUT);
		*portOutputRegister(xm_port) &= ~xm_pin;
	#endif
   
   result = 1023-analogRead(_yp);
   
   //Clean pins for LCD fix
   cleanPins();
   
   return result;
}


uint16_t TouchScreen::readTouchY(void) {
	//Added int to save result
   uint16_t result;
   
	#if defined(__arm__)
		pinMode(_xp, INPUT);
		pinMode(_xm, INPUT);
		digitalWrite(_xp, LOW);
		digitalWrite(_xm, LOW);
		pinMode(_yp, OUTPUT);
		digitalWrite(_yp, HIGH);
		pinMode(_ym, OUTPUT);
		digitalWrite(_ym, LOW);
	#else
		pinMode(_xp, INPUT);
		pinMode(_xm, INPUT);
		*portOutputRegister(xp_port) &= ~xp_pin;
		*portOutputRegister(xm_port) &= ~xm_pin;
		pinMode(_yp, OUTPUT);
		*portOutputRegister(yp_port) |= yp_pin;
		pinMode(_ym, OUTPUT);
		*portOutputRegister(ym_port) &= ~ym_pin;
	#endif
	
   result = 1023-analogRead(_xm);
   
   //Clean pins for LCD fix
   cleanPins();
   
   return result;
}


uint16_t TouchScreen::pressure(void) {
  
  // Force LCD display pins for touchscreen operation
  #if defined(__arm__)
	// Set X+ to ground
	pinMode(_xp, OUTPUT);
	digitalWrite(_xp, LOW);
	// Set Y- to VCC
	pinMode(_ym, OUTPUT);
	digitalWrite(_ym, HIGH); 
  
	// Hi-Z X- and Y+
	digitalWrite(_xm, LOW);
	digitalWrite(_yp, LOW);
  #else
	// Set X+ to ground
	pinMode(_xp, OUTPUT);
	*portOutputRegister(xp_port) &= ~xp_pin;
	
	// Set Y- to VCC
	pinMode(_ym, OUTPUT);
	*portOutputRegister(ym_port) |= ym_pin;
	 
	// Hi-Z X- and Y+
	*portOutputRegister(xm_port) &= ~xm_pin;
	*portOutputRegister(yp_port) &= ~yp_pin;
	
  #endif
  
  //Read settings
  pinMode(_xm, INPUT);
  pinMode(_yp, INPUT);
  
  int16_t z1 = analogRead(_xm); 
  int16_t z2 = analogRead(_yp);
  
  if (_rxplate != 0) {
    // now read the x 
    float rtouch;
    rtouch = z2;
    rtouch /= z1;
    rtouch -= 1;
    rtouch *= readTouchX();
    rtouch *= _rxplate;
    rtouch /= 1024;
	
	//Mo need to clear pins here 
	//because readRouchX() does it already
	
    
    return rtouch;
  } else {
	// Clean pins for LCD fix
	cleanPins();
    return (1023-(z2-z1));
  }
}

bool TouchScreen::isTouching(void) {
	//read current pressure level
	uint16_t touch = pressure();
	
	//No need to clear pins, because pressure does it already
	
	//Minimum and maximum that we define as good touch
	if (touch > 20 && touch < 980) {
		return true;
	}
	else return false;
}

//Pin clearing function. This clears used pins for better LCD support on same pins.
void TouchScreen::cleanPins(void) {
	#if defined(__arm__)
		pinMode(_xm, OUTPUT);
		digitalWrite(_xm, LOW);
		pinMode(_yp, OUTPUT);
		digitalWrite(_yp, HIGH);
		pinMode(_ym, OUTPUT);
		digitalWrite(_ym, LOW);
		pinMode(_xp, OUTPUT);
		digitalWrite(_xp, HIGH);
	#else
		pinMode(_xm, OUTPUT);
		*portOutputRegister(xm_port) &= ~xm_pin;
		pinMode(_yp, OUTPUT);
		*portOutputRegister(yp_port) |= yp_pin;
		pinMode(_ym, OUTPUT);
		*portOutputRegister(ym_port) &= ~ym_pin
		pinMode(_xp, OUTPUT);
		*portOutputRegister(xp_port) |= xp_pin;
	#endif
}