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

#include "MorseMachine.h"

using namespace MorseMachine;

encoderMode MorseMachine::encoderState = speedSettingMode;    // we start with adjusting the speed

morserinoMode MorseMachine::morseState = morseKeyer;

morserinoMode MorseMachine::getMode()
{
    return morseState;
}

boolean MorseMachine::isMode(morserinoMode mode)
{
    return morseState == mode;
}

boolean MorseMachine::isEncoderMode(encoderMode mode)
{
    return encoderState == mode;
}

