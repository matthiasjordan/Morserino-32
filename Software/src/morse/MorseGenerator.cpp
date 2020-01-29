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

#include "MorseGenerator.h"
#include "MorseDisplay.h"
#include "MorseMachine.h"
#include "MorseLoRa.h"
#include "MorseKeyer.h"
#include "MorseSound.h"
#include "decoder.h"
#include "MorseMenu.h"
#include "MorseModeEchoTrainer.h"

using namespace MorseGenerator;

unsigned char MorseGenerator::generatorState; // should be MORSE_TYPE instead of uns char
unsigned long MorseGenerator::genTimer;                         // timer used for generating morse code in trainer mode

String MorseGenerator::CWword = "";
String MorseGenerator::clearText = "";
uint8_t MorseGenerator::wordCounter = 0;                          // for maxSequence

int rxDitLength = 0;                    // set new value for length of dits and dahs and other timings
int rxDahLength = 0;
int rxInterCharacterSpace = 0;
int rxInterWordSpace = 0;

boolean MorseGenerator::stopFlag = false;                         // for maxSequence

namespace MorseGenerator
{
    boolean active;
}

MorseGenerator::Config generatorConfig;

namespace internal
{
    String fetchNewWord();
    void dispGeneratedChar();

    void setStart2();

    String textToCWword(String symbols);

    unsigned long getCharTiming(MorseGenerator::Config *generatorConfig, char c);
    unsigned long getIntercharSpace(MorseGenerator::Config *generatorConfig);
    unsigned long getInterwordSpace(MorseGenerator::Config *generatorConfig);
    unsigned long getInterelementSpace(MorseGenerator::Config *generatorConfig);

}

void MorseGenerator::setup()
{
    MorseKeyer::setup();
}

Config* MorseGenerator::getConfig()
{
    return &generatorConfig;
}

void MorseGenerator::setStart()
{
    MORSELOGLN("MG:sS() 1");
    generatorConfig.key = true;
    generatorConfig.printDitDah = false;
    generatorConfig.wordEndMethod = spaceAndFlush;
    generatorConfig.printSpaceAfterChar = false;
    generatorConfig.timing = Timing::tx;
    generatorConfig.clearBufferBeforPrintChar = false;
    generatorConfig.printCharStyle = REGULAR;
    generatorConfig.printChar = true;
    generatorConfig.onFetchNewWord = &voidFunction;
    generatorConfig.onGeneratorWordEnd = &uLongFunctionMinus1;
    generatorConfig.onLastWord = &voidFunction;

    MorseGenerator::handleEffectiveTrainerDisplay(MorsePreferences::prefs.trainerDisplay);

    internal::setStart2();
}

void internal::setStart2()
{
    CWword = "";
    clearText = "";
    genTimer = millis() - 1;  // we will be at end of KEY_DOWN when called the first time, so we can fetch a new word etc...
    wordCounter = 0;                             // reset word counter for maxSequence
}

void MorseGenerator::startTrainer()
{
    MorseGenerator::setStart();
    MorseMachine::morseState = MorseMachine::morseGenerator;
    MorseGenerator::setup();
    MorseDisplay::clear();
    MorseDisplay::printOnScroll(0, REGULAR, 0, "Generator     ");
    MorseDisplay::printOnScroll(1, REGULAR, 0, "Start/Stop:   ");
    MorseDisplay::printOnScroll(2, REGULAR, 0, "Paddle | BLACK");
    delay(1250);
    MorseDisplay::clear();
    MorseDisplay::displayTopLine();
    MorseDisplay::clearScroll();      // clear the buffer
}

void MorseGenerator::handleEffectiveTrainerDisplay(uint8_t mode)
{
    MorseGenerator::Config *generatorConfig = MorseGenerator::getConfig();
    switch (mode)
    {
        case DISPLAY_BY_CHAR:
        {
            generatorConfig->printChar = true;
            generatorConfig->wordEndMethod = MorseGenerator::space;
            MorseDisplay::getConfig()->autoFlush = true;
            break;
        }
        case DISPLAY_BY_WORD:
        {
            generatorConfig->printChar = true;
            generatorConfig->wordEndMethod = MorseGenerator::spaceAndFlush;
            MorseDisplay::getConfig()->autoFlush = false;
            break;
        }
        case NO_DISPLAY:
        {
            generatorConfig->printChar = false;
            generatorConfig->wordEndMethod = MorseGenerator::space;
            MorseDisplay::getConfig()->autoFlush = false;
            break;
        }
    }
}

void MorseGenerator::generateCW()
{          // this is called from loop() (frequently!)  and generates CW

    if (millis() < genTimer)
    {
        // if not at end of key up or down we need to wait, so we just return to loop()
        return;
    }

    switch (generatorState)
    {                                             // CW generator state machine - key is up or down
        case KEY_UP:
        {
            // here we continue if the pause has been long enough

            if (CWword.length() == 0)
            {                                               // fetch a new word if we have an empty word

                String newWord = "";

                uint8_t max = MorsePreferences::prefs.maxSequence;
                if (max && MorseGenerator::wordCounter == (max - 1))
                {
                    // last word;
                    MorseText::setNextWordIsEndSequence();
                    generatorConfig.onLastWord();
                }
                else if (max && MorseGenerator::wordCounter >= max)
                {
                    // stop
                    MorseGenerator::stopFlag = true;
                    MorseGenerator::wordCounter = 0;
//                    MorseEchoTrainer::echoStop = false;
                }

                if (!MorseGenerator::stopFlag)
                {
                    newWord = internal::fetchNewWord();

                    MorseGenerator::clearText = newWord;
                    MorseGenerator::CWword = internal::textToCWword(newWord);
                }

                if (clearText == "")
                {
                    // we really should have something here - unless in trx mode; in this case return
                    return;
                }

                if (wordCounter != 0)
                {
                    switch (generatorConfig.wordEndMethod)
                    {
                        case LF:
                        {
                            MorseDisplay::printToScroll(REGULAR, "\n");
                            break;
                        }
                        case flush:
                        {
                            MorseDisplay::flushScroll();
                            break;
                        }
                        case spaceAndFlush:
                        {
                            MorseDisplay::printToScroll(REGULAR, " ");    /// in any case, add a blank after the word on the display
                            MorseDisplay::flushScroll();
                            break;
                        }
                        case space:
                        {
                            MorseDisplay::printToScroll(REGULAR, " ");    /// in any case, add a blank after the word on the display
                            break;
                        }
                        case nothing:
                        {
                            break;
                        }
                    }
                }
                MorseGenerator::wordCounter += 1;

            }

            // retrieve next element from CWword; if 0, we were at end of character
            char c = CWword[0];
            CWword.remove(0, 1);

            if ((c == '0'))
            {                      // a character just had been finished
                if (generatorConfig.sendCWToLoRa)
                {
                    MorseLoRa::cwForLora(0);
                }
            }
            else
            {
                genTimer = millis() + internal::getCharTiming(&generatorConfig, c);

                if (generatorConfig.sendCWToLoRa)
                {
                    // send the element to LoRa
                    c == '1' ? MorseLoRa::cwForLora(1) : MorseLoRa::cwForLora(2);
                }

                /// if Koch learn character we show dit or dah
                if (generatorConfig.printDitDah)
                {
                    MorseDisplay::printToScroll(REGULAR, c == '1' ? "." : "-");
                }

                if (generatorConfig.key && !stopFlag) // we finished maxSequence and so do start output (otherwise we get a short click)
                {
                    keyOut(true, (!MorseMachine::isMode(MorseMachine::loraTrx)), MorseSound::notes[MorsePreferences::prefs.sidetoneFreq],
                            MorsePreferences::prefs.sidetoneVolume);
                }
                generatorState = KEY_DOWN;                              // next state = key down = dit or dah
            }
            break;
        }
        case KEY_DOWN:
        {
            //// otherwise we continue here; stop keying,  and determine the length of the following pause: inter Element, interCharacter or InterWord?

            if (generatorConfig.key)
            {
                keyOut(false, (!MorseMachine::isMode(MorseMachine::loraTrx)), 0, 0);
            }

            if (CWword.length() == 1)
            {
                // we just ended the the word

                internal::dispGeneratedChar();

                unsigned long delta = generatorConfig.onGeneratorWordEnd();

                if (delta != -1)
                {
                    genTimer = millis() + delta;
                }
                else
                {
                    genTimer = millis() + internal::getInterwordSpace(&generatorConfig);
                    if (generatorConfig.sendCWToLoRa)
                    {                                   // in generator mode and we want to send with LoRa
                        MorseLoRa::cwForLora(0);
                        MorseLoRa::cwForLora(3);                           // as we have just finished a word
                        MorseLoRa::sendWithLora();                         // finalise the string and send it to LoRA
                        delay(MorseKeyer::interCharacterSpace + MorseKeyer::ditLength); // we need a slightly longer pause otherwise the receiving end might fall too far behind...
                    }
                }
            }
            else if (CWword[0] == '0')
            {
                // we are at end of character
                internal::dispGeneratedChar();
                genTimer = millis() + internal::getIntercharSpace(&generatorConfig);
            }
            else
            {                                                                                     // we are in the middle of a character
                genTimer = millis() + internal::getInterelementSpace(&generatorConfig);
            }
            generatorState = KEY_UP;                               // next state = key up = pause
            break;
        }
    }   /// end switch (generatorState)
}

unsigned long internal::getCharTiming(MorseGenerator::Config *generatorConfig, char c)
{
    long delta;
    switch (generatorConfig->timing)
    {
        case quick:
        {
            delta = 2;      // very short timing
            break;
        }
        case tx:
        {
            delta = (c == '1' ? MorseKeyer::ditLength : MorseKeyer::dahLength);
            break;
        }
        case rx:
        {
            delta = (c == '1' ? rxDitLength : rxDahLength);
            break;
        }
        default:
        {
            delta = 0;
            MORSELOGLN("This should not be reached (getCharTiming)");
        }
    }
    return delta;
}

unsigned long internal::getIntercharSpace(MorseGenerator::Config *generatorConfig)
{
    long delta;
    switch (generatorConfig->timing)
    {
        case quick:
        {
            delta = 1;      // very short timing
            break;
        }
        case tx:
        {
            delta = MorseKeyer::interCharacterSpace;
            break;
        }
        case rx:
        {
            delta = rxInterCharacterSpace;
            break;
        }
        default:
        {
            delta = 0;
            MORSELOGLN("This should not be reached (getCharTiming)");
        }
    }
    return delta;
}

unsigned long internal::getInterwordSpace(MorseGenerator::Config *generatorConfig)
{
    long delta;
    switch (generatorConfig->timing)
    {
        case quick:
        {
            delta = 2;      // very short timing
            break;
        }
        case tx:
        {
            delta = MorseKeyer::interWordSpace;
            break;
        }
        case rx:
        {
            delta = rxInterWordSpace;
            break;
        }
        default:
        {
            delta = 0;
            MORSELOGLN("This should not be reached (getCharTiming)");
        }
    }
    return delta;
}

unsigned long internal::getInterelementSpace(MorseGenerator::Config *generatorConfig)
{
    long delta;
    switch (generatorConfig->timing)
    {
        case quick:
        {
            delta = 2;      // very short timing
            break;
        }
        case tx:
        {
            delta = MorseKeyer::ditLength;
            break;
        }
        case rx:
        {
            delta = rxDitLength;
            break;
        }
        default:
        {
            delta = 0;
            MORSELOGLN("This should not be reached (getCharTiming)");
        }
    }
    return delta;
}

String fetchNewWordFromLoRa()
{
    MorseLoRa::Packet packet = MorseLoRa::decodePacket();
    if (!packet.valid)
    {
        return "";
    }

    rxDitLength = 1200 / packet.rxWpm; // set new value for length of dits and dahs and other timings
    rxDahLength = 3 * rxDitLength; // calculate the other timing values
    rxInterCharacterSpace = 3 * rxDitLength;
    rxInterWordSpace = 7 * rxDitLength;
    MorseDisplay::vprintOnStatusLine(true, 4, "%2ir", packet.rxWpm);
    MorseDisplay::printOnStatusLine(true, 9, "s");
    MorseDisplay::updateSMeter(packet.rssi); // indicate signal strength of new packet

    String word = Decoder::CWwordToClearText(packet.payload);
    return word;
}

/**
 * we check the rxBuffer and see if we received something
 */
String fetchNewWord_LoRa()
{
    // we check the rxBuffer and see if we received something
    MorseDisplay::updateSMeter(0); // at end of word we set S-meter to 0 until we receive something again
    ////// from here: retrieve next CWword from buffer!
    String word;
    if (MorseLoRa::loRaBuReady())
    {
        word = fetchNewWordFromLoRa();
    }
    else
    {
        // we did not receive anything
        word = "";
    }
    return word;
}

String internal::fetchNewWord()
{
    String result = "";

    if (MorseMachine::isMode(MorseMachine::loraTrx))
    {
        result = fetchNewWord_LoRa();
    } // end if loraTrx
    else
    {
        generatorConfig.onFetchNewWord();

        // TODO: move into KochLearn.onFetchNewWord()
        if (MorseMenu::isCurrentMenuItem(MorseMenu::_kochLearn))
        {
            morseModeEchoTrainer.setState(MorseModeEchoTrainer::SEND_WORD);
        }

        result = MorseText::generateWord();
    }
    return result;
}

void MorseGenerator::keyOut(boolean on, boolean fromHere, int f, int volume)
{
    //// generate a side-tone with frequency f when on==true, or turn it off
    //// differentiate external (decoder, sometimes cw_generate) and internal (keyer, sometimes Cw-generate) side tones
    //// key transmitter (and line-out audio if we are in a suitable mode)

    static boolean intTone = false;
    static boolean extTone = false;

    static int intPitch, extPitch;

    if (on)
    {
        if (fromHere)
        {
            intPitch = f;
            intTone = true;
            MorseSound::pwmTone(intPitch, volume, true);
            MorseKeyer::keyTransmitter();
        }
        else
        {                    // not from here
            extTone = true;
            extPitch = f;
            if (!intTone)
            {
                MorseSound::pwmTone(extPitch, volume, false);
            }
        }
    }
    else
    {                      // key off
        if (fromHere)
        {
            intTone = false;
            if (extTone)
            {
                MorseSound::pwmTone(extPitch, volume, false);
            }
            else
            {
                MorseSound::pwmNoTone();
            }
            MorseKeyer::unkeyTransmitter();
        }
        else
        {                 // not from here
            extTone = false;
            if (!intTone)
            {
                MorseSound::pwmNoTone();
            }
        }
    }   // end key off
}

/////// generate CW representations from its input string

String internal::textToCWword(String symbols)
{
    int pointer;
    String result = "";

    int l = symbols.length();

    for (int i = 0; i < l; ++i)
    {
        if (i != 0)
        {
            result += "0";
        }
        char c = symbols.charAt(i);                                 // next char in string
        pointer = MorseText::findChar(c);                                 // at which position is the character in CWchars?
        if (pointer == -1)
        {
            result = "11111111"; // <err>
            break;
        }
        result += MorseText::morseChars[pointer].code;
    }
    result += "0";
    return result;
}

/// when generating CW, we display the character (under certain circumstances)
/// add code to display in echo mode when parameter is so set
/// MorsePreferences::prefs.echoDisplay 1 = CODE_ONLY 2 = DISP_ONLY 3 = CODE_AND_DISP

void internal::dispGeneratedChar()
{
    String charString = String(clearText.charAt(0));
    clearText.remove(0, 1);

//    charString.reserve(10);

    if (generatorConfig.printChar)
    {       /// we need to output the character on the display now
        if (generatorConfig.clearBufferBeforPrintChar)
        {
            MorseDisplay::printToScroll(REGULAR, "");                      // clear the buffer first
        }
        MorseDisplay::printToScroll(generatorConfig.printCharStyle, MorseText::internalToProSigns(charString));
        if (generatorConfig.printSpaceAfterChar)
        {
            MorseDisplay::printToScroll(REGULAR, " ");                      // output a space
        }
    }
    else
    {
        MORSELOGLN("Generator: dispGenChar no printChar - would have been " + charString);
    }

    MorsePreferences::fireCharSeen(true);
}

