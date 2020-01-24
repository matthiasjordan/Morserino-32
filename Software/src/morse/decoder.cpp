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

#include "decoder.h"

#include "MorsePreferences.h"
#include "MorseMachine.h"
#include "MorseKeyer.h"
#include "MorseGenerator.h"
#include "MorseDisplay.h"
#include "MorseModeEchoTrainer.h"
#include "MorseSound.h"

using namespace Decoder;

const struct linklist Decoder::CWtree[67] = { //
        {"", 1, 2},            // 0
                {"e", 3, 4},         // 1
                {"t", 5, 6},          // 2
//
                {"i", 7, 8},        // 3
                {"a", 9, 10},        // 4
                {"n", 11, 12},       // 5
                {"m", 13, 14},       // 6
//
                {"s", 15, 16},       // 7
                {"u", 17, 18},       // 8
                {"r", 19, 20},       // 9
                {"w", 21, 22},       //10
                {"d", 23, 24},       //11
                {"k", 25, 26},      //12
                {"g", 27, 28},      //13
                {"o", 29, 30},       //14
//---------------------------------------------
                {"h", 31, 32},       // 15
                {"v", 33, 34},      // 16
                {"f", 63, 63},      // 17
                {"ü", 35, 36},      // 18 german ue
                {"l", 37, 38},      // 19
                {"ä", 39, 63},      // 20 german ae
                {"p", 63, 40},      // 21
                {"j", 63, 41},      // 22
                {"b", 42, 43},      // 23
                {"x", 44, 63},      // 24
                {"c", 63, 45},      // 25
                {"y", 46, 63},      // 26
                {"z", 47, 48},      // 27
                {"q", 63, 63},      // 28
                {"ö", 49, 63},      // 29 german oe
                {"<ch>", 50, 51},        // 30 !!! german "ch"
//---------------------------------------------
                {"5", 64, 63},      // 31
                {"4", 63, 63},      // 32
                {"<ve>", 63, 52},      // 33  or <sn>, sometimes "*"
                {"3", 63, 63},       // 34
                {"*", 53, 63, },      // 35 ¬ used for all unidentifiable characters ¬
                {"2", 63, 63},      // 36
                {"<as>", 63, 63},         // 37 !! <as>
                {"*", 54, 63},      // 38
                {"+", 63, 55},      // 39
                {"*", 56, 63},      // 40
                {"1", 57, 63},      // 41
                {"6", 63, 58},      // 42
                {"=", 63, 63},      // 43
                {"/", 63, 63},      // 44
                {"<ka>", 59, 60},        // 45 !! <ka>
                {"<kn>", 63, 63},        // 46 !! <kn>
                {"7", 63, 63},      // 47
                {"*", 63, 61},      // 48
                {"8", 62, 63},      // 49
                {"9", 63, 63},      // 50
                {"0", 63, 63},      // 51
//
                {"<sk>", 63, 63},        // 52 !! <sk>
                {"?", 63, 63},      // 53
                {"\"", 63, 63},      // 54
                {".", 63, 63},      // 55
                {"@", 63, 63},      // 56
                {"\'", 63, 63},      // 57
                {"-", 63, 63},      // 58
                {";", 63, 63},      // 59
                {"!", 63, 63},      // 60
                {",", 63, 63},      // 61
                {":", 63, 63},      // 62
//
                {"*", 63, 63},       // 63 Default for all unidentified characters
                {"*", 65, 63},       // 64
                {"*", 66, 63},       // 65
                {"<err>", 66, 63}      // 66 !! Error - backspace
        };

uint8_t wpmDecoded;

boolean Decoder::filteredState = false;
boolean Decoder::filteredStateBefore = false;
void (*Decoder::storeCharInResponse)(String);


DECODER_STATES Decoder::decoderState = LOW_;

unsigned long Decoder::ditAvg, Decoder::dahAvg; /// average values of dit and dah lengths to decode as dit or dah and to adapt to speed change

byte Decoder::treeptr = 0;                          // pointer used to navigate within the linked list representing the dichotomic tree

unsigned long Decoder::acsTimer = 0;            // timer to use for automatic character spacing (ACS)
boolean Decoder::speedChanged = true;

unsigned long Decoder::interWordTimer = 0;      // timer to detect interword spaces
int Decoder::goertzel_n = 152;   //// you can use:         152, 304, 456 or 608 - thats the max buffer reserved in checktone()

////// variables for Morse Decoder - the more global ones. rest is further down...
////////////////////////////
/// variables for morse decoder
///////////////////////////////
uint32_t magnitudelimit;                                   // magnitudelimit_low = ( p_goertzelBandwidth? 80000 : 30000);
uint32_t magnitudelimit_low;                               // magnitudelimit = magnitudelimit_low;

///////////////////////////////////////////////////////////
// The sampling frq will be 106.000 on ESp32             //
// because we need the tone in the center of the bins    //
// I set the tone to 698 Hz                              //
// then n the number of samples which give the bandwidth //
// can be (106000 / tone) * 1 or 2 or 3 or 4 etc         //
// init is 106000/698 = 152 *4 = 608 samples             //
// 152 will give you a bandwidth around 700 hz           //
// 304 will give you a bandwidth around 350 hz           //
// 608 will give you a bandwidth around 175 hz           //
///////////////////////////////////////////////////////////

float coeff;
float Q1 = 0;
float Q2 = 0;
float sine;
float cosine;
const float sampling_freq = 106000.0;
const float target_freq = 698.0; /// adjust for your needs see above
///// resulting bandwidth: 700, 350, 233 or 175 Hz, respectively
float bw;

///////////////////////////////////////
// Noise Blanker time which          //
// will be computed based on speed?? //
///////////////////////////////////////
int nbtime = 7;  /// ms noise blanker

unsigned long startTimeHigh;
unsigned long highDuration;
//long lasthighduration;
//long hightimesavg;
//long lowtimesavg;
long startTimeLow;
long lowDuration;
boolean stop = false;

volatile uint8_t dit_rot = 0;
volatile unsigned long dit_collector = 0;

namespace internal
{
    void ON_();
    void OFF_();

    void recalculateDit(unsigned long duration);
    void recalculateDah(unsigned long duration);
    String encodeProSigns(String &input);
    boolean straightKey();
    boolean checkTone();
    void doDecode();
}

void Decoder::setupGoertzel()
{                 /// pre-compute some values that are compute-imntensive and won't change anyway
    uint8_t bw = MorsePreferences::prefs.goertzelBandwidth;
    goertzel_n = (bw == 0 ? 152 : 608);                 // update Goertzel parameters depending on chosen bandwidth
    magnitudelimit_low = (bw ? 160000 : 40000);          // values found by experimenting
    magnitudelimit = magnitudelimit_low;

    bw = (sampling_freq / goertzel_n); //348

    int k;
    float omega;
    k = (int) (0.5 + ((goertzel_n * target_freq) / sampling_freq)); // 2
    omega = (2.0 * PI * k) / goertzel_n;                           //0,041314579
    sine = sin(omega);                                              // 0,007210753
    cosine = cos(omega);                                            // 0,999999739
    coeff = 2.0 * cosine;                                           // 1,999999479
}

boolean Decoder::menuExec(String mode)
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

boolean Decoder::loop() {
    Decoder::doDecodeShow();
    return false;
}

void Decoder::startDecoder()
{
    Decoder::storeCharInResponse = 0;
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

uint8_t Decoder::getDecodedWpm()
{
    return wpmDecoded;
}

/////////////////////////////////////   MORSE DECODER ///////////////////////////////

////////////////////////////
///// Routines for morse decoder - to a certain degree based on code by Hjalmar Skovholm Hansen OZ1JHM - copyleft licence
////////////////////////////

//void setupMorseDecoder() {
//  /// here we will do the init for decoder mode
//  //trainerMode = false;
//  encoderState = volumeSettingMode;
//
//  display.clear();
//  printOnScroll(1, REGULAR, 0, "Start Decoder" );
//  delay(750);
//  display.clear();
//  displayTopLine();
//  drawInputStatus(false);
//  printToScroll(REGULAR,"");      // clear the buffer
//
//  speedChanged = true;
//  displayCWspeed();
//  displayVolume();
//
//  /// set up variables for Goertzel Morse Decoder
//  setupGoertzel();
//  filteredState = filteredStateBefore = false;
//  decoderState = LOW_;
//  ditAvg = 60;
//  dahAvg = 180;
//}

//const float sampling_freq = 106000.0;
//const float target_freq = 698.0; /// adjust for your needs see above
//const int goertzel_n = 304; //// you can use:         152, 304, 456 or 608 - thats the max buffer reserved in checktone()//

#define straightPin leftPin

boolean internal::straightKey()
{            // return true if a straight key was closed, or a touch paddle touched
    if ((MorseMachine::isMode(MorseMachine::morseDecoder)) && ((!digitalRead(straightPin)) || MorseKeyer::leftKey || MorseKeyer::rightKey))
        return true;
    else
        return false;
}

boolean internal::checkTone()
{              /// check if we have a tone signal at A6 with Gortzel's algorithm, and apply some noise blanking as well
    /// the result will be in globale variable filteredState
    /// we return true when we detected a change in state, false otherwise!

    float magnitude;

    static boolean realstate = false;
    static boolean realstatebefore = false;
    static unsigned long lastStartTime = 0;

    uint16_t testData[1216]; /// buffer for freq analysis - max. 608 samples; you could increase this (and n) to a max of 1216, for sample time 10 ms, and bw 88 Hz

///// check straight key first before you check audio in.... (unless we are in transceiver mode)
///// straight key is connected to external paddle connector (tip), i.e. the same as the left pin (dit normally)

//    Serial.println("Decoder::checkTone() 1");

    if (straightKey())
    {
        Serial.println("Decoder::checkTone() 2");
        realstate = true;
        //Serial.println("Straight Key!");
        //keyTx = true;
    }
    else
    {
//        Serial.println("Decoder::checkTone() 3");
        realstate = false;
        //keyTx = false;
        for (int index = 0; index < Decoder::goertzel_n; index++)
            testData[index] = analogRead(audioInPin);
        //Serial.println("Read and stored analog values!");
        for (int index = 0; index < Decoder::goertzel_n; index++)
        {
            float Q0;
            Q0 = coeff * Q1 - Q2 + (float) testData[index];
            Q2 = Q1;
            Q1 = Q0;
        }
        //Serial.println("Calculated Q1 and Q2!");

        float magnitudeSquared = (Q1 * Q1) + (Q2 * Q2) - (Q1 * Q2 * coeff); // we do only need the real part //
        magnitude = sqrt(magnitudeSquared);
        Q2 = 0;
        Q1 = 0;

        //Serial.println("Magnitude: " + String(magnitude) + " Limit: " + String(magnitudelimit));   //// here you can measure magnitude for setup..

        ///////////////////////////////////////////////////////////
        // here we will try to set the magnitude limit automatic //
        ///////////////////////////////////////////////////////////

        if (magnitude > magnitudelimit_low)
        {
            magnitudelimit = (magnitudelimit + ((magnitude - magnitudelimit) / 6)); /// moving average filter
        }

        if (magnitudelimit < magnitudelimit_low)
            magnitudelimit = magnitudelimit_low;

        ////////////////////////////////////
        // now we check for the magnitude //
        ////////////////////////////////////

        if (magnitude > magnitudelimit * 0.6) // just to have some space up
            realstate = true;
        else
            realstate = false;
    }

    /////////////////////////////////////////////////////
    // here we clean up the state with a noise blanker //
    // (debouncing)                                    //
    /////////////////////////////////////////////////////

    if (realstate != realstatebefore)
        lastStartTime = millis();
    if ((millis() - lastStartTime) > nbtime)
    {
        if (realstate != Decoder::filteredState)
        {
            Decoder::filteredState = realstate;
        }
    }
    realstatebefore = realstate;
    Serial.println("Decoder::checkTone() 10");


    if (Decoder::filteredState == Decoder::filteredStateBefore) {
        Serial.println("Decoder::checkTone() 11");
        return false;                                 // no change detected in filteredState
    }
    else
    {
        Serial.println("Decoder::checkTone() 12");
        Decoder::filteredStateBefore = Decoder::filteredState;
        return true;                                // change detected in filteredState
    }
}   /// end checkTone()

void internal::doDecode()
{
    float lacktime;
    int wpm;

//    Serial.println("Decoder::doDecode() 1");
    switch (decoderState)
    {
        case INTERELEMENT_:
            if (internal::checkTone())
            {
                internal::ON_();
                decoderState = HIGH_;
            }
            else
            {
                lowDuration = millis() - startTimeLow;                        // we record the length of the pause
                lacktime = 2.2;                                  ///  when high speeds we have to have a little more pause before new letter
                //if (MorsePreferences::prefs.wpm > 35) lacktime = 2.7;
                //  else if (MorsePreferences::prefs.wpm > 30) lacktime = 2.6;
                if (lowDuration > (lacktime * ditAvg))
                {
                    displayMorse();                                             /// decode the morse character and display it
                    wpm = (wpmDecoded + (int) (7200 / (dahAvg + 3 * ditAvg))) / 2;     //// recalculate speed in wpm
                    if (wpmDecoded != wpm)
                    {
                        wpmDecoded = wpm;
                        speedChanged = true;
                    }
                    decoderState = INTERCHAR_;
                }
            }
            break;
        case INTERCHAR_:
            if (internal::checkTone())
            {
                internal::ON_();
                decoderState = HIGH_;
            }
            else
            {
                lowDuration = millis() - startTimeLow;             // we record the length of the pause
                lacktime = 5;                 ///  when high speeds we have to have a little more pause before new word
                if (wpmDecoded > 35)
                    lacktime = 6;
                else if (wpmDecoded > 30)
                    lacktime = 5.5;
                if (lowDuration > (lacktime * ditAvg))
                {
                    MorseDisplay::printToScroll(REGULAR, " ");                       // output a blank
                    decoderState = LOW_;
                }
            }
            break;
        case LOW_:
            if (internal::checkTone())
            {
                internal::ON_();
                decoderState = HIGH_;
            }
            break;
        case HIGH_:
            if (internal::checkTone())
            {
                internal::OFF_();
                decoderState = INTERELEMENT_;
            }
            break;
    }
}

void Decoder::doDecodeShow()
{
    internal::doDecode();
    if (Decoder::speedChanged)
    {
        Decoder::speedChanged = false;
        MorseDisplay::displayCWspeed();
    }
}

void internal::ON_()
{                                  /// what we do when we just detected a rising flank, from low to high
    unsigned long timeNow = millis();
    lowDuration = timeNow - startTimeLow;             // we record the length of the pause
    startTimeHigh = timeNow;                          // prime the timer for the high state

    MorseGenerator::keyOut(true, false, MorseSound::notes[MorsePreferences::prefs.sidetoneFreq], MorsePreferences::prefs.sidetoneVolume);

    MorseDisplay::drawInputStatus(true);

    if (lowDuration < Decoder::ditAvg * 2.4)                    // if we had an inter-element pause,
        internal::recalculateDit(lowDuration);                    // use it to adjust speed
}

void internal::OFF_()
{                                 /// what we do when we just detected a falling flank, from high to low
    unsigned long timeNow = millis();
    unsigned int threshold = (int) (Decoder::ditAvg * sqrt(Decoder::dahAvg / Decoder::ditAvg));

    //Serial.print("threshold: ");
    //Serial.println(threshold);
    highDuration = timeNow - startTimeHigh;
    startTimeLow = timeNow;

    if (highDuration > (Decoder::ditAvg * 0.5) && highDuration < (Decoder::dahAvg * 2.5))
    {    /// filter out VERY short and VERY long highs
        if (highDuration < threshold)
        { /// we got a dit -
            Decoder::treeptr = Decoder::CWtree[Decoder::treeptr].dit;
            //Serial.print(".");
            internal::recalculateDit(highDuration);
        }
        else
        {        /// we got a dah
            Decoder::treeptr = Decoder::CWtree[Decoder::treeptr].dah;
            //Serial.print("-");
            internal::recalculateDah(highDuration);
        }
    }
    //pwmNoTone();                     // stop side tone
    //digitalWrite(keyerPin, LOW);      // stop keying Tx
    MorseGenerator::keyOut(false, false, 0, 0);
    ///////
    MorseDisplay::drawInputStatus(false);

}

void internal::recalculateDit(unsigned long duration)
{       /// recalculate the average dit length
    Decoder::ditAvg = (4 * Decoder::ditAvg + duration) / 5;
    //Serial.print("ditAvg: ");
    //Serial.println(ditAvg);
    //nbtime =ditLength / 5;
    nbtime = constrain(Decoder::ditAvg / 5, 7, 20);
    //Serial.println(nbtime);
}

void internal::recalculateDah(unsigned long duration)
{       /// recalculate the average dah length
    //static uint8_t rot = 0;
    //static unsigned long collector;

    if (duration > 2 * Decoder::dahAvg)
    {                       /// very rapid decrease in speed!
        Decoder::dahAvg = (Decoder::dahAvg + 2 * duration) / 3;            /// we adjust faster, ditAvg as well!
        Decoder::ditAvg = Decoder::ditAvg / 2 + Decoder::dahAvg / 6;
    }
    else
    {
        Decoder::dahAvg = (3 * Decoder::ditAvg + Decoder::dahAvg + duration) / 3;
    }
    //Serial.print("dahAvg: ");
    //Serial.println(dahAvg);

}

/// display decoded morse code (and store it in echoTrainer
void Decoder::displayMorse()
{
    String symbol;
    symbol.reserve(6);
    if (treeptr == 0)
        return;
    symbol = CWtree[treeptr].symb;
    //Serial.println("Symbol: " + symbol + " treeptr: " + String(treeptr));
    MorseDisplay::printToScroll(REGULAR, symbol);
    if (storeCharInResponse != 0) {
        storeCharInResponse(symbol);
    }
    treeptr = 0;                                    // reset tree pointer
}   /// end of displayMorse()

void Decoder::interWordTimerOff()
{
    interWordTimer = 4294967000;                   /// interword timer should not trigger something now
}

String Decoder::CWwordToClearText(String cwword)
{             // decode the Morse code character in cwword to clear text
    int ptr = 0;
    String result;
    result.reserve(40);
    String symbol;
    symbol.reserve(6);

    result = "";
    for (int i = 0; i < cwword.length(); ++i)
    {
        char c = cwword[i];
        switch (c)
        {
            case '1':
                ptr = CWtree[ptr].dit;
                break;
            case '2':
                ptr = CWtree[ptr].dah;
                break;
            case '0':
                symbol = CWtree[ptr].symb;

                ptr = 0;
                result += symbol;
                break;
        }
    }
    symbol = CWtree[ptr].symb;
    //Serial.println("Symbol: " + symbol + " ptr: " + String(ptr));
    result += symbol;
    return internal::encodeProSigns(result);
}

String internal::encodeProSigns(String &input)
{
    /// clean up clearText   -   S <as>,  - A <ka> - N <kn> - K <sk> - H ch - V <ve>;
    input.replace("<as>", "S");
    input.replace("<ka>", "A");
    input.replace("<kn>", "N");
    input.replace("<sk>", "K");
    input.replace("<ve>", "V");
    input.replace("<ch>", "H");
    input.replace("<err>", "E");
    input.replace("¬", "U");
    //Serial.println(input);
    return input;
}

