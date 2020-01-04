/******************************************************************************************************************************
 *  morse_3 Software for the Morserino-32 multi-functional Morse code machine, based on the Heltec WiFi LORA (ESP32) module ***
 *  Copyright (C) 2018  Willi Kraml, OE1WKL                                                                                 ***
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 *  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with this program.
 *  If not, see <https://www.gnu.org/licenses/>.
 *****************************************************************************************************************************/

#include <Arduino.h>
#include "morsedefs.h"
#include "MorseRotaryEncoder.h"

using namespace MorseRotaryEncoder;

portMUX_TYPE MorseRotaryEncoder::mux = portMUX_INITIALIZER_UNLOCKED;

volatile int8_t MorseRotaryEncoder::_oldState;
volatile int8_t MorseRotaryEncoder::encoderPos = 0;
volatile uint64_t IRTime = 0;   // interrupt time
const int encoderWaitTime = 100;         // how long to wait for next reading from encoder in microseconds
volatile uint8_t MorseRotaryEncoder::stateRegister = 0;

void MorseRotaryEncoder::setup()
{
    encoderPos = 0;           /// this is the encoder position
}

void MorseRotaryEncoder::isr()
{                    // Interrupt service routine is executed when a HIGH to LOW transition is detected on CLK
//if (micros()  > (IRTime + 1000) ) {
    portENTER_CRITICAL_ISR(&mux);

    int sig2 = digitalRead(PinDT); //MSB = most significant bit
    int sig1 = digitalRead(PinCLK); //LSB = least significant bit
    delayMicroseconds(125);                 // this seems to improve the responsiveness of the encoder and avoid any bouncing

    int8_t thisState = sig1 | (sig2 << 1);
    if (_oldState != thisState)
    {
        stateRegister = (stateRegister << 2) | thisState;
        if (thisState == LATCHSTATE)
        {

            if (stateRegister == 135)
                encoderPos = 1;
            else if (stateRegister == 75)
                encoderPos = -1;
            else
                encoderPos = 0;
        }
        _oldState = thisState;
    }
    portEXIT_CRITICAL_ISR(&mux);
}

int MorseRotaryEncoder::checkEncoder()
{
    int t;

    portENTER_CRITICAL(&mux);

    t = encoderPos;
    if (encoderPos)
    {
        encoderPos = 0;
        portEXIT_CRITICAL(&mux);
        return t;
    }
    else
    {
        portEXIT_CRITICAL(&mux);
        return 0;
    }
}

