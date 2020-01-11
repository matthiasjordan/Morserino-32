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
#include "MorsePreferences.h"
#include "MorseLoRa.h"
#include "MorseKeyer.h"
#include "MorseSound.h"
#include "MorseEchoTrainer.h"
#include "decoder.h"
#include "koch.h"
#include "MorsePlayerFile.h"
#include "english_words.h"
#include "abbrev.h"

using namespace MorseGenerator;

const String CWchars = "abcdefghijklmnopqrstuvwxyz0123456789.,:-/=?@+SANKVäöüH";
//                      0....5....1....5....2....5....3....5....4....5....5...
// we use substrings as char pool for trainer mode
// SANK will be replaced by <as>, <ka>, <kn> and <sk>, H = ch
// a = CWchars.substring(0,26); 9 = CWchars.substring(26,36); ? = CWchars.substring(36,45); <> = CWchars.substring(44,49);
// a9 = CWchars.substring(0,36); 9? = CWchars.substring(26,45); ?<> = CWchars.substring(36,50);
// a9? = CWchars.substring(0,45); 9?<> = CWchars.substring(26,50);
// a9?<> = CWchars;

//byte NoE = 0;             // Number of Elements
// byte nextElement[8];      // the list of elements; 0 = dit, 1 = dah

// for each character:
// byte length// byte morse encoding as binary value, beginning with most significant bit

const byte pool[][2] = {
// letters
        {B01000000, 2},  // a    0
        {B10000000, 4},  // b
        {B10100000, 4},  // c
        {B10000000, 3},  // d
        {B00000000, 1},  // e
        {B00100000, 4},  // f
        {B11000000, 3},  // g
        {B00000000, 4},  // h
        {B00000000, 2},  // i
        {B01110000, 4},  // j
        {B10100000, 3},  // k
        {B01000000, 4},  // l
        {B11000000, 2},  // m
        {B10000000, 2},  // n
        {B11100000, 3},  // o
        {B01100000, 4},  // p
        {B11010000, 4},  // q
        {B01000000, 3},  // r
        {B00000000, 3},  // s
        {B10000000, 1},  // t
        {B00100000, 3},  // u
        {B00010000, 4},  // v
        {B01100000, 3},  // w
        {B10010000, 4},  // x
        {B10110000, 4},  // y
        {B11000000, 4},  // z  25
// numbers
        {B11111000, 5},  // 0  26
        {B01111000, 5},  // 1
        {B00111000, 5},  // 2
        {B00011000, 5},  // 3
        {B00001000, 5},  // 4
        {B00000000, 5},  // 5
        {B10000000, 5},  // 6
        {B11000000, 5},  // 7
        {B11100000, 5},  // 8
        {B11110000, 5},  // 9  35
// interpunct   . , : - / = ? @ +    010101 110011 111000 100001 10010 10001 001100 011010 01010
        {B01010100, 6},  // .  36
        {B11001100, 6},  // ,  37
        {B11100000, 6},  // :  38
        {B10000100, 6},  // -  39
        {B10010000, 5},  // /  40
        {B10001000, 5},  // =  41
        {B00110000, 6},  // ?  42
        {B01101000, 6},  // @  43
        {B01010000, 5},  // +  44    (at the same time <ar> !)
// Pro signs  <>  <as> <ka> <kn> <sk>
        {B01000000, 5},  // <as> 45 S
        {B10101000, 5},  // <ka> 46 A
        {B10110000, 5},  // <kn> 47 N
        {B00010100, 6},   // <sk> 48    K
        {B00010000, 5},  // <ve> 49 E
// German characters
        {B01010000, 4},  // ä    50
        {B11100000, 4},  // ö    51
        {B00110000, 4},  // ü    52
        {B11110000, 4}   // ch   53  H
};

boolean generateStartSequence = true;                        // to indicate that we send vvvA next
String MorseGenerator::CWword = "";

AutoStopModes MorseGenerator::autoStop = off;
boolean MorseGenerator::effectiveAutoStop = false;                 // If to stop after each word in generator modes

unsigned char MorseGenerator::generatorState; // should be MORSE_TYPE instead of uns char
unsigned long MorseGenerator::genTimer;                         // timer used for generating morse code in trainer mode

String MorseGenerator::clearText = "";
int MorseGenerator::repeats = 0;
uint8_t MorseGenerator::wordCounter = 0;                          // for maxSequence

int rxDitLength = 0;                    // set new value for length of dits and dahs and other timings
int rxDahLength = 0;
int rxInterCharacterSpace = 0;
int rxInterWordSpace = 0;

GEN_TYPE MorseGenerator::generatorMode = RANDOMS;          // trainer: what symbol (groups) are we going to send?            0 -  5
uint8_t MorseGenerator::effectiveTrainerDisplay = MorsePreferences::prefs.trainerDisplay;
boolean MorseGenerator::stopFlag = false;                         // for maxSequence
boolean MorseGenerator::firstTime = true;                         /// for word doubler mode

MorseGenerator::Config generatorConfig;

namespace internal
{
    String fetchNewWord();
    String fetchRandomWord();
    void dispGeneratedChar();
    void setStart();

    String getRandomChars(int maxLength, int option);
    String getRandomCall(int maxLength);
    String getRandomWord(int maxLength);
    String getRandomAbbrev(int maxLength);
    String generateCWword(String symbols);
    String getRandomCWChars(int option, int maxLength);

    unsigned long getCharTiming(MorseGenerator::Config *generatorConfig, char c);
    unsigned long getIntercharSpace(MorseGenerator::Config *generatorConfig);
    unsigned long getInterwordSpace(MorseGenerator::Config *generatorConfig);
    unsigned long getInterelementSpace(MorseGenerator::Config *generatorConfig);

}

void MorseGenerator::setup()
{
    MorseKeyer::setup();
}

boolean MorseGenerator::menuExec(String mode)
{
    if (mode == "a")
    {
//        MorsePreferences::currentOptions = MorsePreferences::generatorOptions;
    }
    else if (mode == "player")
    {
//        MorsePreferences::currentOptions = MorsePreferences::playerOptions;                  // list of available options in player mode
        MorsePlayerFile::openAndSkip();
    }
    MorseGenerator::startTrainer();
    return true;
}

void MorseGenerator::setNextWordvvvA()
{
    generateStartSequence = true;
}

void MorseGenerator::setSendCWToLoRa(boolean mode)
{
    generatorConfig.sendCWToLoRa = mode;
}

void internal::setStart()
{
    MorseGenerator::setNextWordvvvA();
    CWword = "";
    clearText = "";
}

void MorseGenerator::startTrainer()
{
    internal::setStart();
    MorseGenerator::firstTime = true;
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
    MorseKeyer::keyTx = true;
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
                if (generatorConfig.printSpaceAfterWord)
                {
                    MorseDisplay::printToScroll(REGULAR, " ");    /// in any case, add a blank after the word on the display
                }

                String newWord = internal::fetchNewWord();
                MorseGenerator::clearText = newWord;
                MorseGenerator::CWword = internal::generateCWword(newWord);
                MorseEchoTrainer::echoTrainerWord = newWord;

                if (CWword.length() == 0)
                {
                    // we really should have something here - unless in trx mode; in this case return
                    return;
                }

                if (generatorConfig.printLFAfterWord)
                {
                    MorseDisplay::printToScroll(REGULAR, "\n");
                }
            }

            // retrieve next element from CWword; if 0, we were at end of character
            char c = CWword[0];
            CWword.remove(0, 1);

            if (c == '0' || !CWword.length())
            {                      // a character just had been finished
                if (generatorConfig.sendCWToLoRa)
                {
                    MorseLoRa::cwForLora(0);
                }
            }

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

            if (generatorConfig.key && !stopFlag)         // we finished maxSequence and so do start output (otherwise we get a short click)
            {
                keyOut(true, (!MorseMachine::isMode(MorseMachine::loraTrx)), MorseSound::notes[MorsePreferences::prefs.sidetoneFreq],
                        MorsePreferences::prefs.sidetoneVolume);
            }
            generatorState = KEY_DOWN;                              // next state = key down = dit or dah

            break;
        }
        case KEY_DOWN:
        {
            //// otherwise we continue here; stop keying,  and determine the length of the following pause: inter Element, interCharacter or InterWord?

            if (generatorConfig.key)
            {
                keyOut(false, (!MorseMachine::isMode(MorseMachine::loraTrx)), 0, 0);
            }

            if (CWword.length() == 0)
            {
                // we just ended the the word

                if (MorseMachine::isMode(MorseMachine::morseGenerator))
                    autoStop = effectiveAutoStop ? stop1 : off;

                internal::dispGeneratedChar();

                if (MorseMachine::isMode(MorseMachine::echoTrainer))
                {
                    switch (MorseEchoTrainer::getState())
                    {
                        case MorseEchoTrainer::START_ECHO:
                            MorseEchoTrainer::setState(MorseEchoTrainer::SEND_WORD);
                            genTimer = millis() + MorseKeyer::interCharacterSpace
                                    + (MorsePreferences::prefs.promptPause * MorseKeyer::interWordSpace);
                            break;
                        case MorseEchoTrainer::REPEAT_WORD:
                            // fall through
                        case MorseEchoTrainer::SEND_WORD:
                            if (MorseEchoTrainer::echoStop)
                                break;
                            else
                            {
                                MorseEchoTrainer::setState(MorseEchoTrainer::GET_ANSWER);
                                if (MorsePreferences::prefs.echoDisplay != CODE_ONLY)
                                {
                                    MorseDisplay::printToScroll(REGULAR, " ");
                                    MorseDisplay::printToScroll(INVERSE_REGULAR, ">");    /// add a blank after the word on the display
                                }
                                ++repeats;
                                genTimer = millis() + MorsePreferences::prefs.responsePause * MorseKeyer::interWordSpace;
                            }
                        default:
                            break;
                    }
                }
                else
                {
                    genTimer = millis() + internal::getInterwordSpace(&generatorConfig);
                    if (MorseMachine::isMode(MorseMachine::morseGenerator) && MorsePreferences::prefs.loraTrainerMode == 1)
                    {                                   // in generator mode and we want to send with LoRa
                        MorseLoRa::cwForLora(0);
                        MorseLoRa::cwForLora(3);                           // as we have just finished a word
                        //Serial.println("cwForLora(3);");
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
            {                                                                                         // we are in the middle of a character
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
            Serial.println("This should not be reached (getCharTiming)");
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
            Serial.println("This should not be reached (getCharTiming)");
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
            Serial.println("This should not be reached (getCharTiming)");
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
            Serial.println("This should not be reached (getCharTiming)");
        }
    }
    return delta;
}

String internal::fetchRandomWord()
{
    String word = "";

    switch (MorseGenerator::generatorMode)
    {
        case RANDOMS:
        {
            word = internal::getRandomChars(MorsePreferences::prefs.randomLength, MorsePreferences::prefs.randomOption);
            break;
        }
        case CALLS:
        {
            word = internal::getRandomCall(MorsePreferences::prefs.callLength);
            break;
        }
        case ABBREVS:
        {
            word = internal::getRandomAbbrev(MorsePreferences::prefs.abbrevLength);
            break;
        }
        case WORDS:
        {
            word = internal::getRandomWord(MorsePreferences::prefs.wordLength);
            break;
        }
        case KOCH_LEARN:
        {
            word = Koch::getChar(MorsePreferences::prefs.kochFilter);
            break;
        }
        case MIXED:
        {
            switch (random(4))
            {
                case 0:
                    word = internal::getRandomWord(MorsePreferences::prefs.wordLength);
                    break;
                case 1:
                    word = internal::getRandomAbbrev(MorsePreferences::prefs.abbrevLength);
                    break;
                case 2:
                    word = internal::getRandomCall(MorsePreferences::prefs.callLength);
                    break;
                case 3:
                    word = internal::getRandomChars(1, OPT_PUNCTPRO); // just a single pro-sign or interpunct
                    break;
            }
            break;
        }
        case KOCH_MIXED:
        {
            switch (random(3))
            {
                case 0:
                    word = internal::getRandomWord(MorsePreferences::prefs.wordLength);
                    break;
                case 1:
                    word = internal::getRandomAbbrev(MorsePreferences::prefs.abbrevLength);
                    break;
                case 2:
                    word = internal::getRandomChars(MorsePreferences::prefs.randomLength, OPT_KOCH); // Koch option!
                    break;
            }
            break;
        }
        case PLAYER:
        {
            if (MorsePreferences::prefs.randomFile)
            {
                MorsePlayerFile::skipWords(random(MorsePreferences::prefs.randomFile + 1));
            }
            word = MorsePlayerFile::getWord();
            break;
        }
        case NA:
        {
            break;
        }
    } // end switch (generatorMode)
    return word;
}

String fetchNewWordFromLoRa()
{
    String word;
    int rssi, rxWpm;
    uint8_t header = MorseLoRa::decodePacket(&rssi, &rxWpm, &MorseGenerator::CWword);
    //Serial.println("Header: " + (String) header);
    //Serial.println("CWword: " + (String) CWword);
    //Serial.println("Speed: " + (String) rxWpm);
    if ((header >> 6) != 1)
    {
        // invalid protocol version
        word = "";
    }
    if ((rxWpm < 5) || (rxWpm > 60))
    {
        // invalid speed
        word = "";
    }
    word = Decoder::CWwordToClearText(MorseGenerator::CWword);
    //Serial.println("clearText: " + (String) clearText);
    //Serial.println("RX Speed: " + (String)rxWpm);
    //Serial.println("RSSI: " + (String)rssi);
    rxDitLength = 1200 / rxWpm; // set new value for length of dits and dahs and other timings
    rxDahLength = 3 * rxDitLength; // calculate the other timing values
    rxInterCharacterSpace = 3 * rxDitLength;
    rxInterWordSpace = 7 * rxDitLength;
    MorseDisplay::vprintOnStatusLine(true, 4, "%2ir", rxWpm);
    MorseDisplay::printOnStatusLine(true, 9, "s");
    MorseDisplay::updateSMeter(rssi); // indicate signal strength of new packet
    return word;
}

/**
 * we check the rxBuffer and see if we received something
 */
String fetchNewWord_LoRa()
{
    // we check the rxBuffer and see if we received something
    MorseDisplay::updateSMeter(0); // at end of word we set S-meter to 0 until we receive something again
    //Serial.print("end of word - S0? ");
    generateStartSequence = false;
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

String randomGenerate()
{
    MorseGenerator::repeats = 0;
    String result = "";
    if ((MorsePreferences::prefs.maxSequence != 0) && (generatorMode != KOCH_LEARN))
    {
        if (MorseMachine::isMode(MorseMachine::echoTrainer) || ((MorseMachine::isMode(MorseMachine::morseGenerator)) && !effectiveAutoStop))
        {
            ++MorseGenerator::wordCounter;
            int limit = 1 + MorsePreferences::prefs.maxSequence;
            if (MorseGenerator::wordCounter == limit)
            {
                result = "+";
                MorseEchoTrainer::echoStop = true;
                if (MorseEchoTrainer::isState(MorseEchoTrainer::REPEAT_WORD))
                {
                    MorseEchoTrainer::setState(MorseEchoTrainer::SEND_WORD);
                }
            }
            else if (MorseGenerator::wordCounter == (limit + 1))
            {
                MorseGenerator::stopFlag = true;
                MorseEchoTrainer::echoStop = false;
                MorseGenerator::wordCounter = 1;
            }
        }
    }
    if (result != "+")
    {
        result = internal::fetchRandomWord();
    }
    MorseGenerator::firstTime = false;
    return result;
}

String internal::fetchNewWord()
{
    String result = "";
//Serial.println("startFirst: " + String((startFirst ? "true" : "false")));
//Serial.println("firstTime: " + String((firstTime ? "true" : "false")));
    if (MorseMachine::isMode(MorseMachine::loraTrx))
    {
        result = fetchNewWord_LoRa();
    } // end if loraTrx
    else
    {
        if (MorseGenerator::generatorMode == KOCH_LEARN)
        {
            generateStartSequence = false;
            MorseEchoTrainer::setState(MorseEchoTrainer::SEND_WORD);
        }

        if (generateStartSequence == true)
        {                                 /// do the initial sequence in trainer mode, too
            result = "vvvA";
            generateStartSequence = false;
        }
        else if (MorseMachine::isMode(MorseMachine::morseGenerator) && MorsePreferences::prefs.wordDoubler == true
                && MorseGenerator::firstTime == false)
        {
            result = MorseEchoTrainer::echoTrainerWord;
            MorseGenerator::firstTime = true;
        }
        else if (MorseMachine::isMode(MorseMachine::echoTrainer))
        {
            Decoder::interWordTimerOff();
            //Serial.println("echoTrainerState: " + String(echoTrainerState));
            switch (MorseEchoTrainer::getState())
            {
                case MorseEchoTrainer::REPEAT_WORD:
                {
                    if (MorsePreferences::prefs.echoRepeats == 7 || MorseGenerator::repeats <= MorsePreferences::prefs.echoRepeats)
                        result = MorseEchoTrainer::echoTrainerWord;
                    else
                    {
                        result = MorseEchoTrainer::echoTrainerWord;
                        if (MorseGenerator::generatorMode != KOCH_LEARN)
                        {
                            MorseDisplay::printToScroll(INVERSE_REGULAR, MorseDisplay::cleanUpProSigns(result)); //// clean up first!
                            MorseDisplay::printToScroll(REGULAR, " ");
                        }
                        result = randomGenerate();
                    }
                    break;
                }
                case MorseEchoTrainer::SEND_WORD:
                {
                    result = randomGenerate();
                    break;
                }
                default:
                    break;
            }   /// end special cases for echo Trainer
        }
        else
        {
            result = randomGenerate();
        }       /// end if else - we either already had something in trainer mode, or we got a new word

    } /// else (= not in loraTrx mode)
    return result;
} // end of fetchNewWord()

void MorseGenerator::keyOut(boolean on, boolean fromHere, int f, int volume)
{
    //// generate a side-tone with frequency f when on==true, or turn it off
    //// differentiate external (decoder, sometimes cw_generate) and internal (keyer, sometimes Cw-generate) side tones
    //// key transmitter (and line-out audio if we are in a suitable mode)

    static boolean intTone = false;
    static boolean extTone = false;

    static int intPitch, extPitch;

// Serial.println("keyOut: " + String(on) + String(fromHere));
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
            digitalWrite(keyerPin, LOW);      // stop keying Tx
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
/////// CWchars = "abcdefghijklmnopqrstuvwxyz0123456789.,:-/=?@+SANKVäöüH";

String internal::generateCWword(String symbols)
{
    int pointer;
    byte bitMask, NoE;
    //byte nextElement[8];      // the list of elements; 0 = dit, 1 = dah
    String result = "";

    int l = symbols.length();

    for (int i = 0; i < l; ++i)
    {
        char c = symbols.charAt(i);                                 // next char in string
        pointer = CWchars.indexOf(c);                             // at which position is the character in CWchars?
        if (pointer == -1)
        {
            return "111111110"; // <err>
        }
        NoE = pool[pointer][1];                                     // how many elements in this morse code symbol?
        bitMask = pool[pointer][0];                                 // bitMask indicates which of the elements are dots and which are dashes
        for (int j = 0; j < NoE; ++j)
        {
            result += (bitMask & B10000000 ? "2" : "1");     // get MSB and store it in string - 2 is dah, 1 is dit, 0 = inter-element space
            bitMask = bitMask << 1;                               // shift bitmask 1 bit to the left
        } /// now we are at the end of one character, therefore we add enough space for inter-character
        result += "0";
    }     /// now we are at the end of the word, therefore we remove the final 0!
    result.remove(result.length() - 1);
    return result;
}

/// when generating CW, we display the character (under certain circumstances)
/// add code to display in echo mode when parameter is so set
/// MorsePreferences::prefs.echoDisplay 1 = CODE_ONLY 2 = DISP_ONLY 3 = CODE_AND_DISP

void internal::dispGeneratedChar()
{
    static String charString;
    charString.reserve(10);

    if (MorseGenerator::generatorMode == KOCH_LEARN || MorseMachine::isMode(MorseMachine::loraTrx)
            || (MorseMachine::isMode(MorseMachine::morseGenerator) && MorseGenerator::effectiveTrainerDisplay == DISPLAY_BY_CHAR)
            || (MorseMachine::isMode(MorseMachine::echoTrainer) && MorsePreferences::prefs.echoDisplay != CODE_ONLY))
    //&& echoTrainerState != SEND_WORD
    //&& echoTrainerState != REPEAT_WORD))

    {       /// we need to output the character on the display now
        charString = MorseGenerator::clearText.charAt(0);                   /// store first char of clearText in charString
        MorseGenerator::clearText.remove(0, 1);                              /// and remove it from clearText
        if (MorseGenerator::generatorMode == KOCH_LEARN)
        {
            MorseDisplay::printToScroll(REGULAR, "");                      // clear the buffer first
        }
        MorseDisplay::printToScroll(MorseMachine::isMode(MorseMachine::loraTrx) ? BOLD : REGULAR,
                MorseDisplay::cleanUpProSigns(charString));
        if (MorseGenerator::generatorMode == KOCH_LEARN)
            MorseDisplay::printToScroll(REGULAR, " ");                      // output a space
    }   //// end display_by_char

    MorsePreferences::fireCharSeen(true);
}

String internal::getRandomCWChars(int option, int maxLength)
{
    int s = 0, e = 50;
    switch (option)
    {
        case OPT_NUM:
        case OPT_NUMPUNCT:
        case OPT_NUMPUNCTPRO:
            s = 26;
            break;
        case OPT_PUNCT:
        case OPT_PUNCTPRO:
            s = 36;
            break;
        case OPT_PRO:
            s = 44;
            break;
        default:
            s = 0;
            break;
    }
    switch (option)
    {
        case OPT_ALPHA:
            e = 26;
            break;
        case OPT_ALNUM:
        case OPT_NUM:
            e = 36;
            break;
        case OPT_ALNUMPUNCT:
        case OPT_NUMPUNCT:
        case OPT_PUNCT:
            e = 45;
            break;
        default:
            e = 50;
            break;
    }
    String result = "";
    for (int i = 0; i < maxLength; ++i)
    {
        result += CWchars.charAt(random(s, e));
    }

    return result;
}

// we use substrings as char pool for trainer mode
// SANK will be replaced by <as>, <ka>, <kn> and <sk>
// Options:
//    0: a9?<> = CWchars; all of them; same as Koch 45+
//    1: a = CWchars.substring(0,26);
//    2: 9 = CWchars.substring(26,36);
//    3: ? = CWchars.substring(36,45);
//    4: <> = CWchars.substring(44,50);
//    5: a9 = CWchars.substring(0,36);
//    6: 9? = CWchars.substring(26,45);
//    7: ?<> = CWchars.substring(36,50);
//    8: a9? = CWchars.substring(0,45);
//    9: 9?<> = CWchars.substring(26,50);

//  {OPT_ALL, OPT_ALPHA, OPT_NUM, OPT_PUNCT, OPT_PRO, OPT_ALNUM, OPT_NUMPUNCT, OPT_PUNCTPRO, OPT_ALNUMPUNCT, OPT_NUMPUNCTPRO}

String internal::getRandomChars(int maxLength, int option)
{             /// random char string, eg. group of 5, 9 differing character pools; maxLength = 1-6
    String result;

    if (maxLength > 6)
    {                                        // we use a random length!
        maxLength = random(2, maxLength - 3);                     // maxLength is max 10, so random upper limit is 7, means max 6 chars...
    }

    if (Koch::isKochActive())
    {
        result = Koch::getRandomChars(maxLength);
    }
    else
    {
        result = internal::getRandomCWChars(option, maxLength);
    }
    return result;
}

String internal::getRandomCall(int maxLength)
{            // random call-sign like pattern, maxLength = 3 - 6, 0 returns any length
    const byte prefixType[] = {1, 0, 1, 2, 3, 1};         // 0 = a, 1 = aa, 2 = a9, 3 = 9a
    byte prefix;
    String call = "";
    unsigned int l = 0;
    //int s, e;

    if (maxLength == 1 || maxLength == 2)
        maxLength = 3;
    if (maxLength > 6)
        maxLength = 6;

    if (maxLength == 3)
        prefix = 0;
    else
        prefix = prefixType[random(0, 6)];           // what type of prefix?
    switch (prefix)
    {
        case 1:
            call += CWchars.charAt(random(0, 26));
            ++l;
        case 0:
            call += CWchars.charAt(random(0, 26));
            ++l;
            break;
        case 2:
            call += CWchars.charAt(random(0, 26));
            call += CWchars.charAt(random(26, 36));
            l = 2;
            break;
        case 3:
            call += CWchars.charAt(random(26, 36));
            call += CWchars.charAt(random(0, 26));
            l = 2;
            break;
    } // we have a prefix by now; l is its length
      // now generate a number
    call += CWchars.charAt(random(26, 36));
    ++l;
    // generate a suffix, 1 2 or 3 chars long - we re-use prefix for the suffix length
    if (maxLength == 3)
        prefix = 1;
    else if (maxLength == 0)
    {
        prefix = random(1, 4);
        prefix = (prefix == 2 ? prefix : random(1, 4)); // increase the likelihood for suffixes of length 2
    }
    else
    {
        //prefix = random(1,_min(maxLength-l+1, 4));     // suffix not longer than 3 chars!
        prefix = _min(maxLength - l, 3);                 // we try to always give the desired length, but never more than 3 suffix chars
    }
    while (prefix--)
    {
        call += CWchars.charAt(random(0, 26));
        ++l;
    } // now we have the suffix as well
      // are we /p or /m? - we do this only in rare cases - 1 out of 9, and only when maxLength = 0, or maxLength-l >= 2
    if (maxLength == 0) //|| maxLength - l >= 2)
        if (!random(0, 8))
        {
            call += "/";
            call += (random(0, 2) ? "m" : "p");
        }
    // we have a complete call sign!
    return call;
}

String internal::getRandomWord(int maxLength)
{        //// give me a random English word, max maxLength chars long (1-5) - 0 returns any length
    if (maxLength > 5)
        maxLength = 0;
    if (Koch::isKochActive())
        return Koch::getRandomWord();
    else
        return EnglishWords::getRandomWord(maxLength);
}

String internal::getRandomAbbrev(int maxLength)
{        //// give me a random CW abbreviation , max maxLength chars long (1-5) - 0 returns any length
    if (maxLength > 5)
        maxLength = 0;
    if (Koch::isKochActive())
        return Koch::getRandomAbbrev();
    else
        return Abbrev::getRandomAbbrev(maxLength);
}

void MorseGenerator::setupHeadCopying()
{
    effectiveAutoStop = true;
    effectiveTrainerDisplay = DISPLAY_BY_CHAR;
}
