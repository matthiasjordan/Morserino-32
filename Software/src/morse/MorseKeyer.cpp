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

#include "MorseKeyer.h"
#include "MorseMachine.h"
#include "MorseGenerator.h"
#include "decoder.h"
#include "MorseLoRaCW.h"
#include "MorseDisplay.h"
#include "MorseSound.h"

using namespace MorseKeyer;

namespace internal
{

    void updatePaddleLatch(boolean dit, boolean dah);
    void setDITstate();
    void setDAHstate();
    boolean doPaddleIambic(boolean dit, boolean dah);
    uint8_t readSensors(int left, int right);
    void initSensors();
}

unsigned char MorseKeyer::keyerControl = 0; // this holds the latches for the paddles and the DIT_LAST latch, see above
KSTYPE MorseKeyer::keyerState;

uint8_t MorseKeyer::sensor;                 // what we read from checking the touch sensors
boolean MorseKeyer::leftKey, MorseKeyer::rightKey;

boolean MorseKeyer::DIT_FIRST = false; // first latched was dit?
unsigned int MorseKeyer::ditLength;        // dit length in milliseconds - 100ms = 60bpm = 12 wpm
unsigned int MorseKeyer::dahLength;        // dahs are 3 dits long
boolean MorseKeyer::keyTx = false;
unsigned int MorseKeyer::interCharacterSpace;
unsigned int MorseKeyer::interWordSpace;   // need to be properly initialised!
unsigned int MorseKeyer::effWpm;                                // calculated effective speed in WpM

unsigned int lUntouched = 0;                        // sensor values (in untouched state) will be stored here
unsigned int rUntouched = 0;

void (*MorseKeyer::onWordEnd)();
void (*MorseKeyer::onCharacter)(String keyed);
void (*MorseKeyer::onWordEndDitDah)();
void (*MorseKeyer::onWordEndNDitDah)();

void MorseKeyer::setup()
{
    // to calibrate sensors, we record the values in untouched state
    internal::initSensors();
    MorseKeyer::updateTimings();
    onCharacter = [](String s)
    {
        MorseDisplay::printToScroll(FONT_OUTGOING, s);
    };
    onWordEnd = []() {
        MorseDisplay::printToScroll(FONT_OUTGOING, " ");
    };
    onWordEndDitDah = &voidFunction;
    onWordEndNDitDah = &voidFunction;
    clearPaddleLatches();
}

void MorseKeyer::updateTimings()
{
    ditLength = 1200 / MorsePreferences::prefs.wpm;                    // set new value for length of dits and dahs and other timings
    dahLength = 3 * ditLength;
    interCharacterSpace = MorsePreferences::prefs.interCharSpace * ditLength;
    //interWordSpace = _max(p_interWordSpace * ditLength, (p_interCharSpace+6)*ditLength);
    interWordSpace = _max(MorsePreferences::prefs.interWordSpace, MorsePreferences::prefs.interCharSpace+4) * ditLength;

    MorseKeyer::effWpm = 60000 / (31 * ditLength + 4 * interCharacterSpace + interWordSpace); ///  effective wpm with lengthened spaces = Farnsworth speed
}

void MorseKeyer::keyTransmitter()
{
    if (MorseKeyer::keyTx == true)
    {
        MORSELOGLN("MorseKeyer::keyTrans() 4");
        digitalWrite(keyerPin, HIGH);           // turn the LED on, key transmitter, or whatever
    }
}

void MorseKeyer::unkeyTransmitter()
{
    digitalWrite(keyerPin, LOW);      // stop keying Tx
}

///////////////////
// we use the paddle for iambic keying
/////

boolean MorseKeyer::doPaddleIambic()
{
    return internal::doPaddleIambic(MorseKeyer::leftKey, MorseKeyer::rightKey);
}

boolean internal::doPaddleIambic(boolean dit, boolean dah)
{
    boolean paddleSwap;                      // temp storage if we need to swap left and right
    static long ktimer;                      // timer for current element (dit or dah)
    static long curtistimer;                 // timer for early paddle latch in Curtis mode B+
    static long latencytimer;                // timer for "muting" paddles for some time in state INTER_ELEMENT
    unsigned int pitch;

    if (!MorsePreferences::prefs.didah)
    {              // swap left and right values if necessary!
        paddleSwap = dit;
        dit = dah;
        dah = paddleSwap;
    }

    switch (keyerState)
    {                                         // this is the keyer state machine
        case IDLE_STATE:
            // display the interword space, if necessary
            if (millis() > Decoder::interWordTimer)
            {
//                MorseDisplay::printToScroll(REGULAR, " ");                       // output a blank
                Decoder::interWordTimer = 4294967000;  // almost the biggest possible unsigned long number :-) - do not output extra spaces!
                MorseKeyer::onWordEnd();
                return false;
            }

            // Was there a paddle press?
            if (dit || dah)
            {
                internal::updatePaddleLatch(dit, dah);  // trigger the paddle latches
                MorseKeyer::onWordEndDitDah();
                Decoder::treeptr = 0;
                if (dit)
                {
                    internal::setDITstate();          // set next state
                    DIT_FIRST = true;          // first paddle pressed after IDLE was a DIT
                }
                else
                {
                    internal::setDAHstate();
                    DIT_FIRST = false;         // first paddle was a DAH
                }
            }
            else
            {
                if (millis() > MorseGenerator::genTimer)
                {
                    MorseKeyer::onWordEndNDitDah();
                }
                return false;           // we return false if there was no paddle press in IDLE STATE - Arduino can do other tasks for a bit
            }
            break;

        case DIT:
            /// first we check that we have waited as defined by ACS settings
            if (MorsePreferences::prefs.ACSlength > 0 && (millis() <= Decoder::acsTimer))
            { // if we do automatic character spacing, and haven't waited for (3 or whatever) dits...
                break;
            }
            clearPaddleLatches();                           // always clear the paddle latches at beginning of new element
            keyerControl |= DIT_LAST;                        // remember that we process a DIT

            ktimer = ditLength;                              // prime timer for dit
            switch (MorsePreferences::prefs.keyermode)
            {
                case IAMBICB:
                {
                    curtistimer = 2 + (ditLength * MorsePreferences::prefs.curtisBDotTiming / 100);
                    break;                         // enhanced Curtis mode B starts checking after some time
                }
                case NONSQUEEZE:
                {
                    curtistimer = 3;
                    break;
                }
                default:
                {
                    curtistimer = ditLength;        // no early paddle checking in Curtis mode A Ultimatic mode oder Non-squeeze
                    break;
                }
            }
            keyerState = KEY_START;                          // set next state of state machine
            break;

        case DAH:
            if (MorsePreferences::prefs.ACSlength > 0 && (millis() <= Decoder::acsTimer))
            {
                // if we do automatic character spacing, and haven't waited for 3 dits...
                break;
            }
            clearPaddleLatches();                          // clear the paddle latches
            keyerControl &= ~(DIT_LAST);                    // clear dit latch  - we are not processing a DIT

            ktimer = dahLength;
            switch (MorsePreferences::prefs.keyermode)
            {
                case IAMBICB:
                {
                    curtistimer = 2 + (dahLength * MorsePreferences::prefs.curtisBTiming / 100); // enhanced Curtis mode B starts checking after some time
                    break;
                }
                case NONSQUEEZE:
                {
                    curtistimer = 3;
                    break;
                }
                default:
                {
                    curtistimer = dahLength;        // no early paddle checking in Curtis mode A or Ultimatic mode
                    break;
                }
            }
            keyerState = KEY_START;                          // set next state of state machine
            break;

        case KEY_START:
            // Assert key down, start timing, state shared for dit or dah
            pitch = MorseSound::notes[MorsePreferences::prefs.sidetoneFreq];
            if ((MorseMachine::isMode(MorseMachine::echoTrainer) || MorseMachine::isMode(MorseMachine::loraTrx))
                    && MorsePreferences::prefs.echoToneShift != 0)
            {
                pitch = (MorsePreferences::prefs.echoToneShift == 1 ? pitch * 18 / 17 : pitch * 17 / 18); /// one half tone higher or lower, as set in parameters in echo trainer mode
            }
            //pwmTone(pitch, MorsePreferences::prefs.sidetoneVolume, true);
            //keyTransmitter();
            MorseGenerator::keyOut(true, true, pitch, MorsePreferences::prefs.sidetoneVolume);
            ktimer += millis();                     // set ktimer to interval end time
            curtistimer += millis();                // set curtistimer to curtis end time
            keyerState = KEYED;                     // next state
            break;

        case KEYED:
            // Wait for timers to expire
            if (millis() > ktimer)
            {                // are we at end of key down ?
                //digitalWrite(keyerPin, LOW);        // turn the LED off, unkey transmitter, or whatever
                //pwmNoTone();                      // stop side tone
                MorseGenerator::keyOut(false, true, 0, 0);
                ktimer = millis() + ditLength;    // inter-element time
                latencytimer = millis() + ((MorsePreferences::prefs.latency - 1) * ditLength / 8);
                keyerState = INTER_ELEMENT;       // next state
            }
            else if (millis() > curtistimer)
            {
                // in Curtis mode we check paddle as soon as Curtis time is off
                if (keyerControl & DIT_LAST)
                {
                    // last element was a dit
                    internal::updatePaddleLatch(false, dah); // not sure here: we only check the opposite paddle - should be ok for Curtis B
                }
                else
                {
                    internal::updatePaddleLatch(dit, false); // but we remain in the same state until element time is off!
                }
            }
            break;

        case INTER_ELEMENT:
            //if ((MorsePreferences::prefs.keyermode != NONSQUEEZE) && (millis() < latencytimer)) {     // or should it be MorsePreferences::prefs.keyermode > 2 ? Latency for Ultimatic mode?
            if (millis() < latencytimer)
            {
                if (keyerControl & DIT_LAST)
                {
                    // last element was a dit
                    internal::updatePaddleLatch(false, dah); // not sure here: we only check the opposite paddle - should be ok for Curtis B
                }
                else
                {
                    internal::updatePaddleLatch(dit, false);
                }
            }
            else
            {
                internal::updatePaddleLatch(dit, dah);          // latch paddle state while between elements
                if (millis() > ktimer)
                {               // at end of INTER-ELEMENT
                    switch (keyerControl)
                    {
                        case 3:                                         // both paddles are latched
                        case 7:
                        {
                            switch (MorsePreferences::prefs.keyermode)
                            {
                                case NONSQUEEZE:
                                {
                                    if (DIT_FIRST)
                                    {                     // when first element was a DIT
                                        internal::setDITstate();            // next element is a DIT again
                                    }
                                    else
                                    {
                                        // but when first element was a DAH
                                        internal::setDAHstate();            // the next element is a DAH again!
                                    }
                                    break;
                                }
                                case ULTIMATIC:
                                {
                                    if (DIT_FIRST)
                                    {                     // when first element was a DIT
                                        internal::setDAHstate();            // next element is a DAH
                                    }
                                    else
                                    {
                                        // but when first element was a DAH
                                        internal::setDITstate();            // the next element is a DIT!
                                    }
                                    break;
                                }
                                default:
                                {
                                    if (keyerControl & DIT_LAST)
                                    {     // Iambic: last element was a dit - this is case 7, really
                                        internal::setDAHstate();               // next element will be a DAH
                                    }
                                    else
                                    {
                                        // and this is case 3 - last element was a DAH
                                        internal::setDITstate();               // the next element is a DIT
                                    }
                                }
                            }
                            break;
                            // dit only is latched, regardless what was last element
                        }
                        case 1:
                        case 5:
                        {
                            internal::setDITstate();
                            break;
                            // dah only latched, regardless what was last element
                        }
                        case 2:
                        case 6:
                        {
                            internal::setDAHstate();
                            break;
                            // none latched, regardless what was last element
                        }
                        case 0:
                        case 4:
                        {
                            keyerState = IDLE_STATE;               // we are at the end of the character and go back into IDLE STATE
                            /*
                             * display the decoded morse character(s)
                             */
                            String symbol = Decoder::getMorsedChar();
                            if (symbol != "") {
                                onCharacter(symbol);
                            }

                            if (MorseMachine::isMode(MorseMachine::loraTrx))
                            {
                                MorseLoRaCW::cwForLora(0);
                            }

                            MorsePreferences::fireCharSeen(false);

                            if (MorsePreferences::prefs.ACSlength > 0)
                            {
                                Decoder::acsTimer = millis() + MorsePreferences::prefs.ACSlength * ditLength; // prime the ACS timer
                            }

                            if (MorseMachine::isMode(MorseMachine::morseKeyer) || MorseMachine::isMode(MorseMachine::loraTrx)
                                    || MorseMachine::isMode(MorseMachine::morseTrx))
                            {
                                Decoder::interWordTimer = millis() + 5 * ditLength;
                            }
                            else
                            {
                                Decoder::interWordTimer = millis() + interWordSpace; // prime the timer to detect a space between characters
                            }

                            // nominally 7 dit-lengths, but we are not quite so strict here in keyer or TrX mode,
                            // use the extended time in echo trainer mode to allow longer space between characters,
                            // like in listening
                            keyerControl = 0;                          // clear all latches completely before we go to IDLE
                            break;
                        }
                    } // switch keyerControl : evaluation of flags
                }
            } // end of INTER_ELEMENT
    } // end switch keyerState - end of state machine

    if (keyerControl & 3)
    {                                               // any paddle latch?
        return true;                                                      // we return true - we processed a paddle press
    }
    else
    {
        return false;                                                     // when paddle latches are cleared, we return false
    }
} /////////////////// end function doPaddleIambic()

//// this function checks the paddles (touch or external), returns true when a paddle has been activated,
///// and sets the global variable leftKey and rightKey accordingly

boolean MorseKeyer::checkPaddles()
{
    static boolean oldL = false, newL, oldR = false, newR;
    int left, right;
    static long lTimer = 0, rTimer = 0;
    const int debDelay = 750;       // debounce time = 0,75  ms

    /* intral and external paddle are now working in parallel - the parameter MorsePreferences::prefs.extPaddle is used to indicate reverse polarity of external paddle
     */
    left = MorsePreferences::prefs.useExtPaddle ? rightPin : leftPin;
    right = MorsePreferences::prefs.useExtPaddle ? leftPin : rightPin;
    sensor = internal::readSensors(LEFT, RIGHT);
    newL = (sensor >> 1) | (!digitalRead(left));
    newR = (sensor & 0x01) | (!digitalRead(right));

    if ((MorsePreferences::prefs.keyermode == NONSQUEEZE) && newL && newR)
    {
        return (leftKey || rightKey);
    }

    if (newL != oldL)
    {
        lTimer = micros();
    }

    if (newR != oldR)
    {
        rTimer = micros();
    }

    if (micros() - lTimer > debDelay)
    {
        if (newL != leftKey)
        {
            leftKey = newL;
        }
    }

    if (micros() - rTimer > debDelay)
    {
        if (newR != rightKey)
        {
            rightKey = newR;
        }
    }

    oldL = newL;
    oldR = newR;

    return (leftKey || rightKey);
}

///
/// Keyer subroutines
///

// update the paddle latches in keyerControl
void internal::updatePaddleLatch(boolean dit, boolean dah)
{
    if (dit)
    {
        MorseKeyer::keyerControl |= DIT_L;
    }

    if (dah)
    {
        MorseKeyer::keyerControl |= DAH_L;
    }
}

// clear the paddle latches in keyer control
void MorseKeyer::clearPaddleLatches()
{
    keyerControl &= ~(DIT_L + DAH_L);   // clear both paddle latch bits
}

// functions to set DIT and DAH keyer states
void internal::setDITstate()
{
    keyerState = DIT;
    Decoder::treeptr = Decoder::CWtree[Decoder::treeptr].dit;
    if (MorseMachine::isMode(MorseMachine::loraTrx))
    {
        MorseLoRaCW::cwForLora(1);                         // build compressed string for LoRA
    }
}

void internal::setDAHstate()
{
    keyerState = DAH;
    Decoder::treeptr = Decoder::CWtree[Decoder::treeptr].dah;
    if (MorseMachine::isMode(MorseMachine::loraTrx))
    {
        MorseLoRaCW::cwForLora(2);
    }
}

/// function to read sensors:
/// read both left and right twice, repeat reading if it returns 0
/// return a binary value, depending on a (adaptable?) threshold:
/// 0 = nothing touched,  1= right touched, 2 = left touched, 3 = both touched
/// binary:   00          01                10                11

uint8_t internal::readSensors(int left, int right)
{
    //static boolean first = true;
    uint8_t v, lValue, rValue;

    while (!(v = touchRead(left)))
    {
        ;                                       // ignore readings with value 0
    }

    lValue = v;
    while (!(v = touchRead(right)))
    {
        ;                                       // ignore readings with value 0
    }

    rValue = v;
    while (!(v = touchRead(left)))
    {
        ;                                       // ignore readings with value 0
    }

    lValue = (lValue + v) / 2;
    while (!(v = touchRead(right)))
    {
        ;                                       // ignore readings with value 0
    }

    rValue = (rValue + v) / 2;

    if (lValue < (MorsePreferences::prefs.tLeft + 10))
    {           //adaptive calibration
        //if (first) MORSELOGLN("p-tLeft " + String(MorsePreferences::prefs.tLeft));
        //if (first) MORSELOG("lValue: "); if (first) MORSELOGLN(lValue);
        //printOnScroll(0, INVERSE_BOLD, 0,  String(lValue) + " " + String(MorsePreferences::prefs.tLeft));
        MorsePreferences::prefs.tLeft = (7 * MorsePreferences::prefs.tLeft + ((lValue + lUntouched) / SENS_FACTOR)) / 8;
        //MORSELOG("MorsePreferences::prefs.tLeft: "); MORSELOGLN(MorsePreferences::prefs.tLeft);
    }
    if (rValue < (MorsePreferences::prefs.tRight + 10))
    {           //adaptive calibration
        //if (first) MORSELOGLN("p-tRight " + String(MorsePreferences::prefs.tRight));
        //if (first) MORSELOG("rValue: "); if (first) MORSELOGLN(rValue);
        //printOnScroll(1, INVERSE_BOLD, 0,  String(rValue) + " " + String(MorsePreferences::prefs.tRight));
        MorsePreferences::prefs.tRight = (7 * MorsePreferences::prefs.tRight + ((rValue + rUntouched) / SENS_FACTOR)) / 8;
        //MORSELOG("MorsePreferences::prefs.tRight: "); MORSELOGLN(MorsePreferences::prefs.tRight);
    }
    //first = false;
    return (lValue < MorsePreferences::prefs.tLeft ? 2 : 0) + (rValue < MorsePreferences::prefs.tRight ? 1 : 0);
}

void internal::initSensors()
{
    int v;
    lUntouched = rUntouched = 60;       /// new: we seek minimum
    for (int i = 0; i < 8; ++i)
    {
        while (!(v = touchRead(LEFT)))
        {
            ;                                       // ignore readings with value 0
        }
        lUntouched += v;
        //lUntouched = _min(lUntouched, v);
        while (!(v = touchRead(RIGHT)))
        {
            ;                                       // ignore readings with value 0
        }
        rUntouched += v;
        //rUntouched = _min(rUntouched, v);
    }
    lUntouched /= 8;
    rUntouched /= 8;
    MorsePreferences::prefs.tLeft = lUntouched - 9;
    MorsePreferences::prefs.tRight = rUntouched - 9;
}

void MorseKeyer::changeSpeed(int t)
{
    MorsePreferences::prefs.wpm += t;
    MorsePreferences::prefs.wpm = constrain(MorsePreferences::prefs.wpm, 5, 60);
    MorseKeyer::updateTimings();
    MorseDisplay::displayCWspeed();                     // update display of CW speed
    MorsePreferences::charCounter = 0;                                    // reset character counter
}
