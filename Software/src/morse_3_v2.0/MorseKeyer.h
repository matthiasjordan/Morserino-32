#ifndef MORSEKEYER_H_
#define MORSEKEYER_H_

#include <Arduino.h>

namespace MorseKeyer {


//  keyerControl bit definitions

#define     DIT_L      0x01     // Dit latch
#define     DAH_L      0x02     // Dah latch
#define     DIT_LAST   0x04     // Dit was last processed element

//  Global Keyer Variables
//
unsigned char keyerControl = 0; // this holds the latches for the paddles and the DIT_LAST latch, see above


boolean DIT_FIRST = false; // first latched was dit?
unsigned int ditLength ;        // dit length in milliseconds - 100ms = 60bpm = 12 wpm
unsigned int dahLength ;        // dahs are 3 dits long
unsigned char keyerState;
uint8_t sensor;                 // what we read from checking the touch sensors
boolean leftKey, rightKey;
unsigned int interCharacterSpace, interWordSpace;   // need to be properly initialised!
unsigned int effWpm;                                // calculated effective speed in WpM

void updateTimings();
void keyTransmitter();


}

#endif /* MORSEKEYER_H_ */
