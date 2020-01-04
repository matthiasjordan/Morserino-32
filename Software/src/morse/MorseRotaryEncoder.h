#ifndef MORSEROTARYENCODER_H_
#define MORSEROTARYENCODER_H_

#include <Arduino.h>

#define LATCHSTATE 3

namespace MorseRotaryEncoder
{

    // things for reading the encoder
    extern portMUX_TYPE mux;
    extern volatile int8_t _oldState;

    // positions: [3] 1 0 2 [3] 1 0 2 [3]
    // [3] is the positions where my rotary switch detends
    // ==> right, count up
    // <== left,  count down

    extern volatile int8_t encoderPos;
    extern volatile uint64_t IRTime;   // interrupt time
    extern const int encoderWaitTime;         // how long to wait for next reading from encoder in microseconds
    extern volatile uint8_t stateRegister;

    void setup();
    void IRAM_ATTR isr();
    int IRAM_ATTR checkEncoder();
}

#endif /* MORSEROTARYENCODER_H_ */
