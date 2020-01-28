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

#include "MorseModeDecoder.h"
#include "decoder.h"
#include "MorseMachine.h"
#include "MorseKeyer.h"
#include "MorseGenerator.h"
#include "MorseDisplay.h"

MorseModeDecoder morseModeDecoder;


boolean MorseModeDecoder::menuExec(String mode)
{
    MorsePreferences::currentOptions = MorsePreferences::decoderOptions;               // list of available options in lora trx mode
    MorseMachine::morseState = MorseMachine::morseDecoder;
    MorseMachine::encoderState = MorseMachine::volumeSettingMode;
    MorseKeyer::keyTx = false;
    MorseDisplay::clear();
    MorseDisplay::printOnScroll(1, REGULAR, 0, "Start Decoder");

    Decoder::startDecoder();
    return true;
}


void MorseModeDecoder::startDecoder()
{
    Decoder::onCharacter = [](String s){};
    Decoder::speedChanged = true;
    delay(650);
    MorseDisplay::clear();
    MorseDisplay::displayTopLine();
    MorseDisplay::drawInputStatus(false);
    MorseDisplay::printToScroll(REGULAR, "");      // clear the buffer

    MorseDisplay::displayCWspeed();
    MorseDisplay::displayVolume();

    MorseKeyer::setup();

    /// set up variables for Goertzel Morse Decoder
    Decoder::setupGoertzel();
    Decoder::filteredState = Decoder::filteredStateBefore = false;
    Decoder::decoderState = Decoder::LOW_;
    Decoder::ditAvg = 60;
    Decoder::dahAvg = 180;
}


boolean MorseModeDecoder::loop()
{
    Decoder::doDecodeShow();
    return false;
}

boolean MorseModeDecoder::togglePause() {
    return false;
}

void MorseModeDecoder::onPreferencesChanged() {
    // nothing to do
}
