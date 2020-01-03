#ifndef MORSEROTARYENCODER_H_
#define MORSEROTARYENCODER_H_

#include <Arduino.h>

namespace MorseRotaryEncoder
{

    // things for reading the encoder
    extern portMUX_TYPE mux;

    extern volatile int8_t _oldState;

#define LATCHSTATE 3

    extern volatile int8_t encoderPos;
    extern volatile uint64_t IRTime;   // interrupt time
    extern const int encoderWaitTime;         // how long to wait for next reading from encoder in microseconds
    extern volatile uint8_t stateRegister;

    void setup();
    void IRAM_ATTR isr();
    int IRAM_ATTR checkEncoder();
}

#endif /* MORSEROTARYENCODER_H_ */
