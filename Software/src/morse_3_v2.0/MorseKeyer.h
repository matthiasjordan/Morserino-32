#ifndef MORSEKEYER_H_
#define MORSEKEYER_H_

#include <Arduino.h>

// defines for keyer modi
//

#define    IAMBICA      1          // Curtis Mode A
#define    IAMBICB      2          // Curtis Mode B (with enhanced Curtis timing, set as parameter
#define    ULTIMATIC    3          // Ultimatic mode
#define    NONSQUEEZE   4          // Non-squeeze mode of dual-lever paddles - simulate a single-lever paddle

namespace MorseKeyer {

    ///////////////////////////////////////////////////////////////////////////////
    //
    //  Iambic Keyer State Machine Defines

    enum KSTYPE {IDLE_STATE, DIT, DAH, KEY_START, KEYED, INTER_ELEMENT };


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
KSTYPE keyerState;
uint8_t sensor;                 // what we read from checking the touch sensors
boolean leftKey, rightKey;
unsigned int interCharacterSpace, interWordSpace;   // need to be properly initialised!
unsigned int effWpm;                                // calculated effective speed in WpM

boolean keyTx = false;             // when state is set by manual key or touch paddle, then true!
                                   // we use this to decide if Tx should be keyed or not
void setup();
void updateTimings();
void keyTransmitter();
boolean doPaddleIambic(boolean dit, boolean dah);
boolean checkPaddles();
void clearPaddleLatches ();


}

#endif /* MORSEKEYER_H_ */
