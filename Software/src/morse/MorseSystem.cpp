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

#include "MorseSystem.h"
#include "MorseDisplay.h"
#include "MorsePreferences.h"
#include <WiFi.h>          // basic WiFi functionality
#include <LoRa.h>          // library for LoRa transceiver

using namespace MorseSystem;

boolean MorseSystem::menuExec(String mode)
{
    if (mode == "sleep")
    {
        MorseSystem::checkShutDown(true);
    }

    return false;
}

//// measure battery voltage in mV
int16_t MorseSystem::batteryVoltage()
{
    int32_t c;

#if BOARDVERSION == 3
    WiFi.mode(WIFI_MODE_NULL);      // make sure WiFi is not running, as it uses the same ADC as battery measurement!
    const float XS = 1.95;      //The returned reading is multiplied by this XS to get the battery voltage.

    analogSetClockDiv(128);      //  this value was found by experimenting - no clue what it really does :-(
    analogSetPinAttenuation(batteryPin, ADC_11db);
    //delay(75);
    c = 0;
    for (int i = 0; i < 2048; ++i)
        c += analogRead(batteryPin);

    c = (int) c * XS;
    c = c / 2048;
    //printOnScroll(1, REGULAR, 1, String(c));
    analogSetClockDiv(1);      // 5ms

#elif BOARDVERSION == 2     // probably buggy - but BOARDVERSION 2 is not supported anymore, was prototype only
    adcAttachPin(batteryPin);

    const float XS = 1.255;
    c = analogRead(batteryPin);
    delay(100);
    c = analogRead(batteryPin);
    c = (int) c*XS;

#endif

    return c;
}

volatile uint64_t TOTcounter;                       // holds millis for Time-Out Timer

void MorseSystem::resetTOT()
{       //// reset the Time Out Timer - we do this whenever there is a screen update
    TOTcounter = millis();
}

void MorseSystem::checkShutDown(boolean enforce)
{       /// if enforce == true, we shut donw even if there was no time-out
    // unsigend long timeOut = ((morseState == loraTrx) || (morseState == morseTrx)) ? 450000 : 300000;  /// 7,5 or 5 minutes
    unsigned long timeOut;
    switch (MorsePreferences::prefs.timeOut)
    {
        case 4:
            timeOut = ULONG_MAX;
            break;
        default:
            timeOut = 300000UL * MorsePreferences::prefs.timeOut;
            break;
    }

    if ((millis() - TOTcounter) > timeOut || enforce == true)
    {
        MorseDisplay::clear();
        MorseDisplay::printOnScroll(1, INVERSE_BOLD, 0, "Power OFF...");
        MorseDisplay::printOnScroll(2, REGULAR, 0, "RED to turn ON");
        MorseDisplay::displayDisplay();
        delay(1500);
        shutMeDown();
    }
}

void MorseSystem::shutMeDown()
{
    MorseDisplay::sleep();                //OLED sleep
    LoRa.sleep();                   //LORA sleep
    delay(50);
#if BOARDVERSION == 3
    digitalWrite(Vext, HIGH);
#endif
    delay(50);
    esp_deep_sleep_start();         // go to deep sleep
}

