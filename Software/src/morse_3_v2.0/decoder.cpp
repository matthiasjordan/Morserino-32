#include "decoder.h"

#include "MorsePreferences.h"

using namespace Decoder;

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

boolean straightKey() {            // return true if a straight key was closed, or a touch paddle touched
if ((morseState == morseDecoder) && ((!digitalRead(straightPin)) || leftKey || rightKey) )
    return true;
else return false;
}

boolean checkTone() {              /// check if we have a tone signal at A6 with Gortzel's algorithm, and apply some noise blanking as well
                                   /// the result will be in globale variable filteredState
                                   /// we return true when we detected a change in state, false otherwise!

  float magnitude ;

  static boolean realstate = false;
  static boolean realstatebefore = false;
  static unsigned long lastStartTime = 0;

  uint16_t testData[1216];         /// buffer for freq analysis - max. 608 samples; you could increase this (and n) to a max of 1216, for sample time 10 ms, and bw 88 Hz

///// check straight key first before you check audio in.... (unless we are in transceiver mode)
///// straight key is connected to external paddle connector (tip), i.e. the same as the left pin (dit normally)

if (straightKey() ) {
    realstate = true;
    //Serial.println("Straight Key!");
    //keyTx = true;
    }
else {
    realstate = false;
    //keyTx = false;
    for (int index = 0; index < goertzel_n ; index++)
        testData[index] = analogRead(audioInPin);
    //Serial.println("Read and stored analog values!");
    for (int index = 0; index < goertzel_n ; index++) {
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

    if (magnitude > magnitudelimit_low) {
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
  if ((millis() - lastStartTime) > nbtime) {
    if (realstate != filteredState) {
      filteredState = realstate;
    }
  }
  realstatebefore = realstate;

 if (filteredState == filteredStateBefore)
  return false;                                 // no change detected in filteredState
 else {
    filteredStateBefore = filteredState;
    return true;                                // change detected in filteredState
 }
}   /// end checkTone()


void doDecode() {
  float lacktime;
  int wpm;

    switch(decoderState) {
      case INTERELEMENT_: if (checkTone()) {
                              ON_();
                              decoderState = HIGH_;
                          } else {
                              lowDuration = millis() - startTimeLow;                        // we record the length of the pause
                              lacktime = 2.2;                                               ///  when high speeds we have to have a little more pause before new letter
                              //if (MorsePreferences::prefs.wpm > 35) lacktime = 2.7;
                              //  else if (MorsePreferences::prefs.wpm > 30) lacktime = 2.6;
                              if (lowDuration > (lacktime * ditAvg)) {
                                displayMorse();                                             /// decode the morse character and display it
                                wpm = (MorsePreferences::prefs.wpm + (int) (7200 / (dahAvg + 3*ditAvg))) / 2;     //// recalculate speed in wpm
                                if (MorsePreferences::prefs.wpm != wpm) {
                                  MorsePreferences::prefs.wpm = wpm;
                                  speedChanged = true;
                                }
                                decoderState = INTERCHAR_;
                              }
                          }
                          break;
      case INTERCHAR_:    if (checkTone()) {
                              ON_();
                              decoderState = HIGH_;
                          } else {
                              lowDuration = millis() - startTimeLow;             // we record the length of the pause
                              lacktime = 5;                 ///  when high speeds we have to have a little more pause before new word
                              if (MorsePreferences::prefs.wpm > 35) lacktime = 6;
                                else if (MorsePreferences::prefs.wpm > 30) lacktime = 5.5;
                              if (lowDuration > (lacktime * ditAvg)) {
                                   printToScroll(REGULAR, " ");                       // output a blank
                                   decoderState = LOW_;
                              }
                          }
                          break;
      case LOW_:          if (checkTone()) {
                              ON_();
                              decoderState = HIGH_;
                          }
                          break;
      case HIGH_:         if (checkTone()) {
                              OFF_();
                              decoderState = INTERELEMENT_;
                          }
                          break;
    }
}

void ON_() {                                  /// what we do when we just detected a rising flank, from low to high
   unsigned long timeNow = millis();
   lowDuration = timeNow - startTimeLow;             // we record the length of the pause
   startTimeHigh = timeNow;                          // prime the timer for the high state

   keyOut(true, false, notes[MorsePreferences::prefs.sidetoneFreq], MorsePreferences::prefs.sidetoneVolume);

   drawInputStatus(true);

   if (lowDuration < ditAvg * 2.4)                    // if we had an inter-element pause,
      recalculateDit(lowDuration);                    // use it to adjust speed
}

void OFF_() {                                 /// what we do when we just detected a falling flank, from high to low
  unsigned long timeNow = millis();
  unsigned int threshold = (int) ( ditAvg * sqrt( dahAvg / ditAvg));

  //Serial.print("threshold: ");
  //Serial.println(threshold);
  highDuration = timeNow - startTimeHigh;
  startTimeLow = timeNow;

  if (highDuration > (ditAvg * 0.5) && highDuration < (dahAvg * 2.5)) {    /// filter out VERY short and VERY long highs
      if (highDuration < threshold) { /// we got a dit -
            treeptr = CWtree[treeptr].dit;
            //Serial.print(".");
            recalculateDit(highDuration);
      }
      else  {        /// we got a dah
            treeptr = CWtree[treeptr].dah;
            //Serial.print("-");
            recalculateDah(highDuration);
      }
  }
  //pwmNoTone();                     // stop side tone
  //digitalWrite(keyerPin, LOW);      // stop keying Tx
  keyOut(false, false, 0, 0);
  ///////
  drawInputStatus(false);

}

void drawInputStatus( boolean on) {
  if (on)
    display.setColor(BLACK);
  else
      display.setColor(WHITE);
  display.fillRect(1, 1, 20, 13);
  display.display();
}



void recalculateDit(unsigned long duration) {       /// recalculate the average dit length
  ditAvg = (4*ditAvg + duration) / 5;
  //Serial.print("ditAvg: ");
  //Serial.println(ditAvg);
  //nbtime =ditLength / 5;
  nbtime = constrain(ditAvg/5, 7, 20);
  //Serial.println(nbtime);
}

void recalculateDah(unsigned long duration) {       /// recalculate the average dah length
  //static uint8_t rot = 0;
  //static unsigned long collector;

  if (duration > 2* dahAvg)   {                       /// very rapid decrease in speed!
      dahAvg = (dahAvg + 2* duration) / 3;            /// we adjust faster, ditAvg as well!
      ditAvg = ditAvg/2 + dahAvg/6;
  }
  else {
      dahAvg = (3* ditAvg + dahAvg + duration) / 3;
  }
    //Serial.print("dahAvg: ");
    //Serial.println(dahAvg);

}


void keyOut(boolean on,  boolean fromHere, int f, int volume) {
  //// generate a side-tone with frequency f when on==true, or turn it off
  //// differentiate external (decoder, sometimes cw_generate) and internal (keyer, sometimes Cw-generate) side tones
  //// key transmitter (and line-out audio if we are in a suitable mode)

  static boolean intTone = false;
  static boolean extTone = false;

  static int intPitch, extPitch;

// Serial.println("keyOut: " + String(on) + String(fromHere));
  if (on) {
      if (fromHere) {
        intPitch = f;
        intTone = true;
        pwmTone(intPitch, volume, true);
        keyTransmitter();
      } else {                    // not from here
        extTone = true;
        extPitch = f;
        if (!intTone)
          pwmTone(extPitch, volume, false);
        }
  } else {                      // key off
        if (fromHere) {
          intTone = false;
          if (extTone)
            pwmTone(extPitch, volume, false);
          else
            pwmNoTone();
          digitalWrite(keyerPin, LOW);      // stop keying Tx
        } else {                 // not from here
          extTone = false;
          if (!intTone)
            pwmNoTone();
        }
  }   // end key off
}
