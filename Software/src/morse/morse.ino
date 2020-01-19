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

/*****************************************************************************************************************************
 *  code by others used in this sketch, apart from the ESP32 libraries distributed by Heltec
 *  (see: https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series)
 *
 *  ClickButton library -> https://code.google.com/p/clickbutton/ by Ragnar Aronsen
 * 
 * 
 *  For volume control of NF output: I used a similar principle as  Connor Nishijima, see 
 *                                   https://hackaday.io/project/11957-10-bit-component-less-volume-control-for-arduino
 *                                   but actually using two PWM outputs, connected with an AND gate
 *  Routines for morse decoder - to a certain degree based on code by Hjalmar Skovholm Hansen OZ1JHM
 *                                   - see http://skovholm.com/cwdecoder
 ****************************************************************************************************************************/

///// include of the various libraries and include files being used
#include <Wire.h>          // Only needed for Arduino 1.6.5 and earlier
#include "ClickButton.h"   // button control library
#include <SPI.h>           // library for SPI interface
#include <LoRa.h>          // library for LoRa transceiver
#include <WiFi.h>          // basic WiFi functionality
#include <WebServer.h>     // simple web sever
#include <ESPmDNS.h>       // DNS functionality
#include <WiFiClient.h>    //WiFi clinet library
#include <Update.h>        // update "over the air" (OTA) functionality
#include "FS.h"
#include "SPIFFS.h"

#include "morsedefs.h"
#include "MorseDisplay.h"
#include "wklfonts.h"      // monospaced fonts in size 12 (regular and bold) for smaller text and 15 for larger text (regular and bold), called :
// DialogInput_plain_12, DialogInput_bold_12 & DialogInput_plain_15, DialogInput_bold_15
// these fonts were created with this tool: http://oleddisplay.squix.ch/#/home
#include "abbrev.h"        // common CW abbreviations
#include "english_words.h" // common English words
#include "MorsePreferences.h"
#include "MorsePreferencesMenu.h"
#include "MorseRotaryEncoder.h"
#include "MorseSound.h"
#include "koch.h"
#include "MorseLoRa.h"
#include "MorseSystem.h"
#include "MorseMachine.h"
#include "MorseUI.h"
#include "MorseGenerator.h"
#include "MorsePlayerFile.h"
#include "MorseKeyer.h"
#include "decoder.h"
#include "MorseMenu.h"
#include "MorseEchoTrainer.h"

////////////////////////////////////////////////////////////////////
// encoder subroutines
/// interrupt service routine - needs to be positioned BEFORE all other functions, including setup() and loop()
/// interrupt service routine

void IRAM_ATTR isr()
{                    // Interrupt service routine is executed when a HIGH to LOW transition is detected on CLK
    MorseRotaryEncoder::isr();
}

int IRAM_ATTR checkEncoder()
{
    return MorseRotaryEncoder::checkEncoder();
}

////////////////////////   S E T U P /////////////////////////////

void setup()
{

    Serial.begin(115200);
    delay(200); // give me time to bring up serial monitor

    // enable Vext
#if BOARDVERSION == 3
    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, LOW);
#endif

    // set up the encoder - we need external pull-ups as the pins used do not have built-in pull-ups!
    pinMode(PinCLK, INPUT_PULLUP);
    pinMode(PinDT, INPUT_PULLUP);
    pinMode(keyerPin, OUTPUT);        // we can use the built-in LED to show when the transmitter is being keyed
    pinMode(leftPin, INPUT);          // external keyer left paddle
    pinMode(rightPin, INPUT);         // external keyer right paddle

    /// enable deep sleep
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, (esp_sleep_ext1_wakeup_mode_t) 0); //1 = High, 0 = Low
    analogSetAttenuation(ADC_0db);

// we MUST reset the OLED RST pin for 50 ms! for the old board only, but as it does not hurt we do it anyway
//#if BOARDVERSION == 2
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, LOW);     // set GPIO16 low to reset OLED
    delay(50);
    digitalWrite(OLED_RST, HIGH);    // while OLED is running, must set GPIO16 in high
//# endif

    MorseDisplay::init();
    MorseSound::setup();

    //call ISR when any high/low changed seen
    //on any of the enoder pins
    attachInterrupt(digitalPinToInterrupt(PinDT), isr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PinCLK), isr, CHANGE);

    MorseRotaryEncoder::setup();

    // set up for encoder button
    pinMode(modeButtonPin, INPUT);
    pinMode(volButtonPin, INPUT_PULLUP);               // external pullup for all GPIOS > 32 with ESP32-LORA
                                                       // wake up also works without external pullup! Interesting!

    MorseUI::setup();

    // read preferences from non-volatile storage
    // if version cannot be read, we have a new ESP32 and need to write the preferences first
    MorsePreferences::readPreferences("morserino");

    Koch::setup();
    MorseLoRa::setup();
    MorsePlayerFile::setup();
    MorseDisplay::displayStartUp();

    MorseMenu::setup();
    MorseMenu::menu_();
} /////////// END setup()

///////////////////////// THE MAIN LOOP - do this OFTEN! /////////////////////////////////

void loop()
{
    int t;

    MorseKeyer::checkPaddles();
    switch (MorseMachine::getMode())
    {
        case MorseMachine::morseKeyer: {
            if (MorseKeyer::doPaddleIambic())
            {
                return;                                                        // we are busy keying and so need a very tight loop !
            }
            break;
        }
        case MorseMachine::loraTrx: {
            if (MorseKeyer::doPaddleIambic())
            {
                return;                                                        // we are busy keying and so need a very tight loop !
            }
            MorseGenerator::generateCW();
            break;
        }
        case MorseMachine::morseTrx: {
            if (MorseKeyer::doPaddleIambic())
            {
                return;                                                        // we are busy keying and so need a very tight loop !
            }
            Decoder::doDecodeShow();
            break;
        }
        case MorseMachine::morseGenerator: {
            MorseGenerator::loop();
            break;
        }
        case MorseMachine::echoTrainer:
        {
            if (MorseEchoTrainer::loop())
            {
                return;
            }
            break;
        }
        case MorseMachine::morseDecoder: {
            Decoder::doDecodeShow();
            break;
        }
        default:
            break;

    } // end switch and code depending on state of metaMorserino

    // if we have time check for button presses

    MorseUI::modeButton.Update();
    MorseUI::volButton.Update();

    switch (MorseUI::volButton.clicks)
    {
        case 1:
            if (MorseMachine::isEncoderMode(MorseMachine::scrollMode))
            {
                if (MorseMachine::isMode(MorseMachine::morseDecoder))
                {
                    MorseMachine::encoderState = MorseMachine::speedSettingMode;
                }
                else
                {
                    MorseMachine::encoderState = MorseMachine::volumeSettingMode;
                }
                MorseDisplay::relPos = MorseDisplay::maxPos;
                MorseDisplay::refreshScrollArea((MorseDisplay::bottomLine + 1 + MorseDisplay::relPos) % NoOfLines);
                MorseDisplay::displayScrollBar(false);
            }
            else if (MorseMachine::encoderState == MorseMachine::volumeSettingMode && !MorseMachine::isMode(MorseMachine::morseDecoder))
            {          //  single click toggles encoder between speed and volume
                MorseMachine::encoderState = MorseMachine::speedSettingMode;
                MorsePreferences::writeVolume();
                MorseDisplay::displayCWspeed();
                MorseDisplay::displayVolume();
            }
            else
            {
                MorseMachine::encoderState = MorseMachine::volumeSettingMode;
                MorseDisplay::displayCWspeed();
                MorseDisplay::displayVolume();
            }
            break;
        case -1:
            if (MorseMachine::encoderState == MorseMachine::scrollMode)
            {
                MorseMachine::encoderState =
                        (MorseMachine::isMode(MorseMachine::morseDecoder) ? MorseMachine::volumeSettingMode : MorseMachine::speedSettingMode);
                MorseDisplay::relPos = MorseDisplay::maxPos;
                MorseDisplay::refreshScrollArea((MorseDisplay::bottomLine + 1 + MorseDisplay::relPos) % NoOfLines);
                MorseDisplay::displayScrollBar(false);
            }
            else
            {
                MorseMachine::encoderState = MorseMachine::scrollMode;
                MorseDisplay::displayScrollBar(true);
            }
            break;
    }

    switch (MorseUI::modeButton.clicks)
    {                                // actions based on encoder button
        case -1:
            // long click exits current mode and goes to top menu
            MorseMenu::menu_();
            return;
        case 1:
            if (MorseMachine::isMode(MorseMachine::morseGenerator) || MorseMachine::isMode(MorseMachine::echoTrainer))
            {                                       //  start/stop in trainer modi, in others does nothing currently
                MorseEchoTrainer::active = !MorseEchoTrainer::active;
                if (!MorseEchoTrainer::active)
                {
                    MorseGenerator::keyOut(false, true, 0, 0);
                    MorseDisplay::printOnStatusLine(true, 0, "Continue w/ Paddle");
                }
                else
                {
                    MorseMenu::cleanStartSettings();
                }

            }
            break;
        case 2:
            MorsePreferencesMenu::setupPreferences(MorsePreferences::prefs.menuPtr); // double click shows the preferences menu (true would select a specific option only)
            MorseDisplay::clear();                                  // restore display
            MorseDisplay::displayTopLine();
            if (MorseMachine::isMode(MorseMachine::morseGenerator) || MorseMachine::isMode(MorseMachine::echoTrainer))
                MorseGenerator::stopFlag = true;                                  // we stop what we had been doing
            else
                MorseGenerator::stopFlag = false;
            break;
        default:
            break;
    }

    // and we have time to check the encoder
    if ((t = checkEncoder()))
    {
        MorseUI::click();
        switch (MorseMachine::encoderState)
        {
            case MorseMachine::speedSettingMode:
                MorseEchoTrainer::changeSpeed(t);
                break;
            case MorseMachine::volumeSettingMode:
                MorsePreferences::prefs.sidetoneVolume += (t * 10) + 11;
                MorsePreferences::prefs.sidetoneVolume = constrain(MorsePreferences::prefs.sidetoneVolume, 11, 111) - 11;
                //Serial.println(MorsePreferences::prefs.sidetoneVolume);
                MorseDisplay::displayVolume();
                break;
            case MorseMachine::scrollMode:
                if (t == 1 && MorseDisplay::relPos < MorseDisplay::maxPos)
                {        // up = scroll towards bottom
                    ++MorseDisplay::relPos;
                    MorseDisplay::refreshScrollArea((MorseDisplay::bottomLine + 1 + MorseDisplay::relPos) % NoOfLines);
                }
                else if (t == -1 && MorseDisplay::relPos > 0)
                {
                    --MorseDisplay::relPos;
                    MorseDisplay::refreshScrollArea((MorseDisplay::bottomLine + 1 + MorseDisplay::relPos) % NoOfLines);
                }
                //encoderPos = 0;
                //portEXIT_CRITICAL(&mux);
                MorseDisplay::displayScrollBar(true);
                break;
        }
    } // encoder 
    MorseSystem::checkShutDown(false);         // check for time out

}     /////////////////////// end of loop() /////////

