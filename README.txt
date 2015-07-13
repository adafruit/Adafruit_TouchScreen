This is the 4-wire resistive touch screen firmware for Arduino. Works with all Arduinos and the Mega

I have comoleted the the library in the sense of now the library is pinsafe with LCD. No need to pull LCD pins anymore in sketches.

I also made in UNO and MEGA the library fully port writes so touch screen would be faster.

I removed porthandling initializings from getPoint function to seepd up the point fetching.

I added cleanPins function which is driven on pin affecting functions.

Added bool isTouching function for looping pressure where needed.
