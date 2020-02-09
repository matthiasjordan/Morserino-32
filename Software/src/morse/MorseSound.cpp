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

#include "MorseSound.h"
#include "MorsePreferences.h"

using namespace MorseSound;

/////////////////////// parameters for LF tone generation and  HF (= vol ctrl) PWM
int toneFreq = 500;
int toneChannel = 2;      // this PWM channel is used for LF generation, duty cycle is 0 (silent) or 50% (tone)
int lineOutChannel = 3;   // this PWM channel is used for line-out LF generation, duty cycle is 0 (silent) or 50% (tone)
int volChannel = 8;       // this PWM channel is used for HF generation, duty cycle between 1% (almost silent) and 100% (loud)
int pwmResolution = 10;
unsigned int volFreq = 32000; // this is the HF frequency we are using

const int dutyCycleFiftyPercent = 512;
const int dutyCycleTwentyPercent = 25;
const int dutyCycleZero = 0;

//// functions for generating a tone....

void MorseSound::setup()
{
    // set up PWMs for tone generation
    ledcSetup(toneChannel, toneFreq, pwmResolution);
    ledcAttachPin(LF_Pin, toneChannel);

    ledcSetup(lineOutChannel, toneFreq, pwmResolution);
    ledcAttachPin(lineOutPin, lineOutChannel);                                    ////// change this for real version - no line out currntly

    ledcSetup(volChannel, volFreq, pwmResolution);
    ledcAttachPin(HF_Pin, volChannel);

    ledcWrite(toneChannel, 0);
    ledcWrite(lineOutChannel, 0);
}

void MorseSound::pwmTone(unsigned int frequency, unsigned int volume, boolean lineOut)
{ // frequency in Hertz, volume in range 0 - 100; we use 10 bit resolution
    const uint16_t vol[] =
        {0, 1, 2, 3, 16, 150, 380, 580, 700, 880, 1023};
    int i = uconstrain(volume / 10, 10);
    //MORSELOGLN(vol[i]);
    //MORSELOGLN(frequency);
    if (lineOut)
    {
        ledcWriteTone(lineOutChannel, (double) frequency);
        ledcWrite(lineOutChannel, dutyCycleFiftyPercent);
    }

    ledcWrite(volChannel, volFreq);
    ledcWrite(volChannel, vol[i]);
    ledcWriteTone(toneChannel, frequency);

    if (i == 0)
        ledcWrite(toneChannel, dutyCycleZero);
    else if (i > 3)
        ledcWrite(toneChannel, dutyCycleFiftyPercent);
    else
        ledcWrite(toneChannel, i * i * i + 4 + 2 * i);          /// an ugly hack to allow for lower volumes on headphones

}

void MorseSound::pwmNoTone()
{      // stop playing a tone by changing duty cycle of the tone to 0
    ledcWrite(toneChannel, dutyCycleTwentyPercent);
    ledcWrite(lineOutChannel, dutyCycleTwentyPercent);
    delayMicroseconds(125);
    ledcWrite(toneChannel, dutyCycleZero);
    ledcWrite(lineOutChannel, dutyCycleZero);

}

void MorseSound::pwmClick(unsigned int volume)
{                        /// generate a click on the speaker
    if (!MorsePreferences::prefs.encoderClicks)
        return;
    pwmTone(250, volume, false);
    delay(6);
    pwmTone(280, volume, false);
    delay(5);
    pwmNoTone();
}


void MorseSound::soundSignalOK() {
    MorseSound::pwmTone(440, MorsePreferences::prefs.sidetoneVolume, false);
    delay(97);
    MorseSound::pwmNoTone();
    MorseSound::pwmTone(587, MorsePreferences::prefs.sidetoneVolume, false);
    delay(193);
    MorseSound::pwmNoTone();
}


void MorseSound::soundSignalERR() {
    MorseSound::pwmTone(311, MorsePreferences::prefs.sidetoneVolume, false);
    delay(193);
    MorseSound::pwmNoTone();
}
