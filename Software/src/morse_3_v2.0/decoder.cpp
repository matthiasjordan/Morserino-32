/*
 * decoder.cpp
 *
 *  Created on: 07.12.2019
 *      Author: mj
 */

#include "decoder.h"
#include "prefs.h"

////// variables for Morse Decoder - the more global ones. rest is further down...
////////////////////////////
/// variables for morse decoder
///////////////////////////////
uint32_t magnitudelimit;                                   // magnitudelimit_low = ( p_goertzelBandwidth? 80000 : 30000);
uint32_t magnitudelimit_low;                               // magnitudelimit = magnitudelimit_low;

boolean speedChanged = true;
boolean filteredState = false;
boolean filteredStateBefore = false;

/// state machine for decoding CW
enum DECODER_STATES
{
    LOW_, HIGH_, INTERELEMENT_, INTERCHAR_
};
DECODER_STATES decoderState = LOW_;

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
int goertzel_n = 152;   //// you can use:         152, 304, 456 or 608 - thats the max buffer reserved in checktone()
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

unsigned long ditAvg, dahAvg;     /// average values of dit and dah lengths to decode as dit or dah and to adapt to speed change

volatile uint8_t dit_rot = 0;
volatile unsigned long dit_collector = 0;

void setupGoertzel()
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
