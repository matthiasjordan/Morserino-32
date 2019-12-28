#ifndef MORSEROTARYENCODER_H_
#define MORSEROTARYENCODER_H_

#include <Arduino.h>

namespace MorseRotaryEncoder {

    // things for reading the encoder
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;


    volatile int8_t _oldState;

    #define LATCHSTATE 3

    volatile int8_t encoderPos = 0;
    volatile uint64_t IRTime = 0;   // interrupt time
    const int encoderWaitTime = 100 ;         // how long to wait for next reading from encoder in microseconds
    volatile uint8_t stateRegister = 0;


    void setup();
    void IRAM_ATTR isr();
    int IRAM_ATTR checkEncoder() ;
}



#endif /* MORSEROTARYENCODER_H_ */
