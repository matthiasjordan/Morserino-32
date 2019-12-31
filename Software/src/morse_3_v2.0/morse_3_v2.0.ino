
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








// positions: [3] 1 0 2 [3] 1 0 2 [3]
// [3] is the positions where my rotary switch detends
// ==> right, count up
// <== left,  count down






// defines for keyer modi
//

#define    IAMBICA      1          // Curtis Mode A
#define    IAMBICB      2          // Curtis Mode B (with enhanced Curtis timing, set as parameter
#define    ULTIMATIC    3          // Ultimatic mode
#define    NONSQUEEZE   4          // Non-squeeze mode of dual-lever paddles - simulate a single-lever paddle


//// for adjusting preferences








///////////////////////////////////
//// Other Global VARIABLES ////////////
/////////////////////////////////

unsigned int lUntouched = 0;                        // sensor values (in untouched state) will be stored here
unsigned int rUntouched = 0;





//// not any longer defined in preferences:

  




///////////////////////////////////////////////////////////////////////////////
//
//  Iambic Keyer State Machine Defines
 
enum KSTYPE {IDLE_STATE, DIT, DAH, KEY_START, KEYED, INTER_ELEMENT };





  //CWword.reserve(144);
  //clearText.reserve(50);
boolean active = false;                           // flag for trainer mode






/////////////////// Variables for Koch modes


String kochChars;

////// variables for CW decoder

boolean keyTx = false;             // when state is set by manual key or touch paddle, then true!
                                   // we use this to decide if Tx should be keyed or not





////////////////////////////////////////////////////////////////////
// encoder subroutines
/// interrupt service routine - needs to be positioned BEFORE all other functions, including setup() and loop()
/// interrupt service routine

void IRAM_ATTR isr ()  {                    // Interrupt service routine is executed when a HIGH to LOW transition is detected on CLK
    MorseRotaryEncoder::isr();
}



int IRAM_ATTR checkEncoder() {
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
  digitalWrite(Vext,LOW);
  #endif

  
  // set up the encoder - we need external pull-ups as the pins used do not have built-in pull-ups!
  pinMode(PinCLK,INPUT_PULLUP);
  pinMode(PinDT,INPUT_PULLUP);  
  pinMode(keyerPin, OUTPUT);        // we can use the built-in LED to show when the transmitter is being keyed
  pinMode(leftPin, INPUT);          // external keyer left paddle
  pinMode(rightPin, INPUT);         // external keyer right paddle

  /// enable deep sleep
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, (esp_sleep_ext1_wakeup_mode_t) 0); //1 = High, 0 = Low
  analogSetAttenuation(ADC_0db);


// we MUST reset the OLED RST pin for 50 ms! for the old board only, but as it does not hurt we do it anyway
//#if BOARDVERSION == 2
  pinMode(OLED_RST,OUTPUT);
  digitalWrite(OLED_RST, LOW);     // set GPIO16 low to reset OLED
  delay(50); 
  digitalWrite(OLED_RST, HIGH);    // while OLED is running, must set GPIO16 in high
//# endif

 // init display
  
  MorseDisplay::init();

  MorseSound::setup();
  
  //call ISR when any high/low changed seen
  //on any of the enoder pins
  attachInterrupt (digitalPinToInterrupt(PinDT), isr, CHANGE);   
  attachInterrupt (digitalPinToInterrupt(PinCLK), isr, CHANGE);
 
MorseRotaryEncoder::setup();

/// set up for encoder button
  pinMode(modeButtonPin, INPUT);
  pinMode(volButtonPin, INPUT_PULLUP);               // external pullup for all GPIOS > 32 with ESP32-LORA
                                                     // wake up also works without external pullup! Interesting!
  
  // Setup button timers (all in milliseconds / ms)
  // (These are default if not set, but changeable for convenience)
  modeButton.debounceTime   = 11;   // Debounce timer in ms
  modeButton.multiclickTime = 220;  // Time limit for multi clicks
  modeButton.longClickTime  = 350; // time until "held-down clicks" register

  volButton.debounceTime   = 11;   // Debounce timer in ms
  volButton.multiclickTime = 220;  // Time limit for multi clicks
  volButton.longClickTime  = 350; // time until "held-down clicks" register



  // to calibrate sensors, we record the values in untouched state
  initSensors();
  
  // read preferences from non-volatile storage
  // if version cannot be read, we have a new ESP32 and need to write the preferences first
  MorsePreferences::readPreferences("morserino");

  Koch::setup();


  MorseLoRa::setup();



  /// set up quickstart - this should only be done once at startup - after successful quickstart we disable it to allow normal menu operation

  
MorsePlayerFile::setup();
  MorseDisplay::displayStartUp();

    ///delay(2500);  //// just to be able to see the startup screen for a while - is contained in displayStartUp()

  ////

  menu_();
} /////////// END setup()











///////////////////////// THE MAIN LOOP - do this OFTEN! /////////////////////////////////

void loop() {
// static uint64_t loopC = 0;
   int t;

   boolean activeOld = active;
   checkPaddles();
   switch (MorseMachine::getMode()) {
      case MorseMachine::morseKeyer:    if (doPaddleIambic(MorseKeyer::leftKey, MorseKeyer::rightKey)) {
                               return;                                                        // we are busy keying and so need a very tight loop !
                          }
                          break;
      case MorseMachine::loraTrx:      if (doPaddleIambic(MorseKeyer::leftKey, MorseKeyer::rightKey)) {
                               return;                                                        // we are busy keying and so need a very tight loop !
                          }
                          MorseGenerator::generateCW();
                          break;
      case MorseMachine::morseTrx:      if (doPaddleIambic(MorseKeyer::leftKey, MorseKeyer::rightKey)) {
                               return;                                                        // we are busy keying and so need a very tight loop !
                          }  
                          Decoder::doDecode();
                          if (Decoder::speedChanged) {
                            Decoder::speedChanged = false;
                            displayCWspeed();
                          }
                          break;    
      case MorseMachine::morseGenerator:  if ((autoStop == stop1) || MorseKeyer::leftKey  || MorseKeyer::rightKey)   {                                    // touching a paddle starts and stops the generation of code
                          // for debouncing:
                          while (checkPaddles() )
                              ;                                                           // wait until paddles are released

                          if (effectiveAutoStop) {
                            active = (autoStop == off);
                            switch (autoStop) {
                              case off : {
                                  break;
                                  //
                                }
                              case stop1: {
                                  autoStop = stop2;
                                  break;
                                }
                              case stop2: {
                                  MorseDisplay::printToScroll(REGULAR, "\n");
                                  autoStop = off;
                                  break;
                                }
                            }
                          }
                          else {
                            active = !active;
                            autoStop = off;
                          }

                          //delay(100);
                          } /// end squeeze
                          
                          ///// check stopFlag triggered by maxSequence
                          if (stopFlag) {
                            active = stopFlag = false;
                          }
                          if (activeOld != active) {
                            if (!active) {
                               MorseGenerator::keyOut(false, true, 0, 0);
                               MorseDisplay::printOnStatusLine(true, 0, "Continue w/ Paddle");
                            }
                          else {
                               //cleanStartSettings();        
                               generatorState = KEY_UP; 
                               genTimer = millis()-1;           // we will be at end of KEY_DOWN when called the first time, so we can fetch a new word etc...          
                            }
                          }
                          if (active)
                            MorseGenerator::generateCW();
                          break;
      case MorseMachine::echoTrainer:                             ///// check stopFlag triggered by maxSequence
                          if (stopFlag) {
                            active = stopFlag = false;
                            MorseGenerator::keyOut(false, true, 0, 0);
                            MorseDisplay::printOnStatusLine(true, 0, "Continue w/ Paddle");
                          }
                          if (!active && (MorseKeyer::leftKey  || MorseKeyer::rightKey))   {                       // touching a paddle starts  the generation of code
                              // for debouncing:
                              while (checkPaddles() )
                                  ;                                                           // wait until paddles are released
                              active = !active;
             
                              cleanStartSettings();
                          } /// end touch to start
                          if (active)
                          switch (echoTrainerState) {
                            case  START_ECHO:   
                            case  SEND_WORD:
                            case  REPEAT_WORD:  echoResponse = ""; MorseGenerator::generateCW();
                                                break;
                            case  EVAL_ANSWER:  echoTrainerEval();
                                                break;
                            case  COMPLETE_ANSWER:                    
                            case  GET_ANSWER:   if (doPaddleIambic(MorseKeyer::leftKey, MorseKeyer::rightKey))
                                                    return;                             // we are busy keying and so need a very tight loop !
                                                break;
                            }                              
                            break;
      case MorseMachine::morseDecoder: doDecode();
                         if (speedChanged) {
                            speedChanged = false;
                            displayCWspeed();
                          }
      default:            break;
            
                        
  } // end switch and code depending on state of metaMorserino

/// if we have time check for button presses

    modeButton.Update();
    volButton.Update();
    
    switch (volButton.clicks) {
      case 1:   if (MorseMachine::isEncoderMode(MorseMachine::scrollMode)) {
                    if (MorseMachine::isMode(MorseMachine::morseDecoder)) {
                        encoderState = speedSettingMode;
                    }
                    else {
                        encoderState = volumeSettingMode;
                    }
                    relPos = maxPos;
                    MorseDisplay::refreshScrollArea((bottomLine + 1 + relPos) % NoOfLines);
                    MorseDisplay::displayScrollBar(false);
                } else if (encoderState == volumeSettingMode && morseState != morseDecoder) {          //  single click toggles encoder between speed and volume
                  encoderState = speedSettingMode;
                  MorsePreferences::writeVolume();
                  displayCWspeed();
                  MorseDisplay::displayVolume();
                }
                else {
                  encoderState = volumeSettingMode;
                  displayCWspeed();
                  MorseDisplay::displayVolume();
                }
                break;
      case -1:  if (encoderState == scrollMode) {
                    encoderState = (morseState == morseDecoder ? volumeSettingMode : speedSettingMode);
                    relPos = maxPos;
                    MorseDisplay::refreshScrollArea((bottomLine + 1 + relPos) % NoOfLines);
                    MorseDisplay::displayScrollBar(false);
                }       
                else {
                    encoderState = scrollMode;
                    MorseDisplay::displayScrollBar(true);
                }
                break;
    }
   
    switch (modeButton.clicks) {                                // actions based on enocder button
       case -1:   menu_();                                       // long click exits current mode and goes to top menu
                  return;
       case 1:    if (morseState == morseGenerator || morseState == echoTrainer) {//  start/stop in trainer modi, in others does nothing currently
                  active = !active;
                  if (!active) {
                        //digitalWrite(keyerPin, LOW);           // turn the LED off, unkey transmitter, or whatever
                        //pwmNoTone(); 
                        MorseGenerator::keyOut(false, true, 0, 0);
                        MorseDisplay::printOnStatusLine(true, 0, "Continue w/ Paddle");
                  }
                  else {
                    cleanStartSettings();
                  }
                        
              }
              break;
       case 2:  setupPreferences(MorsePreferences::prefs.menuPtr);                               // double click shows the preferences menu (true would select a specific option only)
                MorseDisplay::clear();                                  // restore display
                displayTopLine();
                if (morseState == morseGenerator || morseState == echoTrainer) 
                    stopFlag = true;                                  // we stop what we had been doing
                else
                    stopFlag = false;
                //startFirst = true;
                //firstTime = true;
     default: break;
    }
    
/// and we have time to check the encoder
     if ((t = checkEncoder())) {
        //Serial.println("t: " + String(t));
        MorseUI::click();
        switch (encoderState) {
          case speedSettingMode:  
                                  changeSpeed(t);
                                  break;
          case volumeSettingMode: 
                                  MorsePreferences::prefs.sidetoneVolume += (t*10)+11;
                                  MorsePreferences::prefs.sidetoneVolume = constrain(MorsePreferences::prefs.sidetoneVolume, 11, 111) -11;
                                  //Serial.println(MorsePreferences::prefs.sidetoneVolume);
                                  displayVolume();
                                  break;
          case scrollMode:
                                  if (t == 1 && relPos < maxPos ) {        // up = scroll towards bottom
                                    ++relPos;
                                    MorseDisplay::refreshScrollArea((bottomLine + 1 + relPos) % NoOfLines);
                                  }
                                  else if (t == -1 && relPos > 0) {
                                    --relPos;
                                    MorseDisplay::refreshScrollArea((bottomLine + 1 + relPos) % NoOfLines);
                                  }
                                  //encoderPos = 0;
                                  //portEXIT_CRITICAL(&mux);
                                  MorseDisplay::displayScrollBar(true);
                                  break;
          }
    } // encoder 
    MorseSystem::checkShutDown(false);         // check for time out
    
}     /////////////////////// end of loop() /////////


void cleanStartSettings() {
    clearText = "";
    CWword = "";
    echoTrainerState = START_ECHO;
    generatorState = KEY_UP; 
    keyerState = IDLE_STATE;
    interWordTimer = 4294967000;                 // almost the biggest possible unsigned long number :-) - do not output a space at the beginning
    genTimer = millis()-1;                       // we will be at end of KEY_DOWN when called the first time, so we can fetch a new word etc... 
    wordCounter = 0;                             // reset word counter for maxSequence
    startFirst = true;
    displayTopLine();
}





// toggle polarity of paddles
void togglePolarity () {
      MorsePreferences::prefs.didah = !MorsePreferences::prefs.didah;
     //displayPolarity();
}
  





//////// Display the status line in CW Keyer Mode
//////// Layout of top line:
//////// Tch ul 15 WpM
//////// 0    5    0

void displayTopLine() {
    MorseDisplay::clearStatusLine();

  // printOnStatusLine(true, 0, (MorsePreferences::prefs.useExtPaddle ? "X " : "T "));          // we do not show which paddle is in use anymore
  if (morseState == morseGenerator) 
      MorseDisplay::printOnStatusLine(true, 1,  MorsePreferences::prefs.wordDoubler ? "x2" : "  ");
  else {
    switch (MorsePreferences::prefs.keyermode) {
      case IAMBICA:   MorseDisplay::printOnStatusLine(false, 2,  "A "); break;          // Iambic A (no paddle eval during dah)
      case IAMBICB:   MorseDisplay::printOnStatusLine(false, 2,  "B "); break;          // orig Curtis B mode: paddle eval during element
      case ULTIMATIC: MorseDisplay::printOnStatusLine(false, 2,  "U "); break;          // Ultimatic Mode
      case NONSQUEEZE: MorseDisplay::printOnStatusLine(false, 2,  "N "); break;         // Non-squeeze mode
    }
  }

  displayCWspeed();                                     // update display of CW speed
  if ((morseState == loraTrx ) || (morseState == morseGenerator  && MorsePreferences::prefs.loraTrainerMode == true))
      dispLoraLogo();

  MorseDisplay::displayVolume();                                     // sidetone volume
  MorseDisplay::display();
}


//////// Display the current CW speed
/////// pos 7-8, "Wpm" on 10-12

void displayCWspeed () {
  if (( morseState == morseGenerator || morseState ==  echoTrainer )) 
      sprintf(numBuffer, "(%2i)", effWpm);   
  else sprintf(numBuffer, "    ");
  
  MorseDisplay::printOnStatusLine(false, 3,  numBuffer);                                         // effective wpm
  
  sprintf(numBuffer, "%2i", MorsePreferences::prefs.wpm);
  MorseDisplay::printOnStatusLine(encoderState == speedSettingMode ? true : false, 7,  numBuffer);
  MorseDisplay::printOnStatusLine(false, 10,  "WpM");
  MorseDisplay::display();
}


/// function to read sensors:
/// read both left and right twice, repeat reading if it returns 0
/// return a binary value, depending on a (adaptable?) threshold:
/// 0 = nothing touched,  1= right touched, 2 = left touched, 3 = both touched
/// binary:   00          01                10                11

uint8_t readSensors(int left, int right) {
  //static boolean first = true;
  uint8_t v, lValue, rValue;
  
  while ( !(v=touchRead(left)) )
    ;                                       // ignore readings with value 0
  lValue = v;
   while ( !(v=touchRead(right)) )
    ;                                       // ignore readings with value 0
  rValue = v;
  while ( !(v=touchRead(left)) )
    ;                                       // ignore readings with value 0
  lValue = (lValue+v) /2;
   while ( !(v=touchRead(right)) )
    ;                                       // ignore readings with value 0
  rValue = (rValue+v) /2;

  if (lValue < (MorsePreferences::prefs.tLeft+10))     {           //adaptive calibration
      //if (first) Serial.println("p-tLeft " + String(MorsePreferences::prefs.tLeft));
      //if (first) Serial.print("lValue: "); if (first) Serial.println(lValue);
      //printOnScroll(0, INVERSE_BOLD, 0,  String(lValue) + " " + String(MorsePreferences::prefs.tLeft));
      MorsePreferences::prefs.tLeft = ( 7*MorsePreferences::prefs.tLeft +  ((lValue+lUntouched) / SENS_FACTOR) ) / 8;
      //Serial.print("MorsePreferences::prefs.tLeft: "); Serial.println(MorsePreferences::prefs.tLeft);
  }
  if (rValue < (MorsePreferences::prefs.tRight+10))     {           //adaptive calibration
      //if (first) Serial.println("p-tRight " + String(MorsePreferences::prefs.tRight));
      //if (first) Serial.print("rValue: "); if (first) Serial.println(rValue);
      //printOnScroll(1, INVERSE_BOLD, 0,  String(rValue) + " " + String(MorsePreferences::prefs.tRight));
      MorsePreferences::prefs.tRight = ( 7*MorsePreferences::prefs.tRight +  ((rValue+rUntouched) / SENS_FACTOR) ) / 8;
      //Serial.print("MorsePreferences::prefs.tRight: "); Serial.println(MorsePreferences::prefs.tRight);
  }
  //first = false;
  return ( lValue < MorsePreferences::prefs.tLeft ? 2 : 0 ) + (rValue < MorsePreferences::prefs.tRight ? 1 : 0 );
}


void initSensors() {
  int v;
  lUntouched = rUntouched = 60;       /// new: we seek minimum
  for (int i=0; i<8; ++i) {
      while ( !(v=touchRead(LEFT)) )
        ;                                       // ignore readings with value 0
        lUntouched += v;
        //lUntouched = _min(lUntouched, v);
       while ( !(v=touchRead(RIGHT)) )
        ;                                       // ignore readings with value 0
        rUntouched += v;
        //rUntouched = _min(rUntouched, v);
  }
  lUntouched /= 8;
  rUntouched /= 8;
  MorsePreferences::prefs.tLeft = lUntouched - 9;
  MorsePreferences::prefs.tRight = rUntouched - 9;
}











///////// evaluate the response in Echo Trainer Mode
void echoTrainerEval() {
    delay(interCharacterSpace / 2);
    if (echoResponse == echoTrainerWord) {
      echoTrainerState = SEND_WORD;
      MorseDisplay::printToScroll(BOLD,  "OK");
      if (MorsePreferences::prefs.echoConf) {
          pwmTone(440,  MorsePreferences::prefs.sidetoneVolume, false);
          delay(97);
          pwmNoTone();
          pwmTone(587,  MorsePreferences::prefs.sidetoneVolume, false);
          delay(193);
          pwmNoTone();
      }
      delay(interWordSpace);
      if (MorsePreferences::prefs.speedAdapt)
          changeSpeed(1);
    } else {
      echoTrainerState = REPEAT_WORD;
      if (generatorMode != KOCH_LEARN || echoResponse != "") {
          MorseDisplay::printToScroll(BOLD, "ERR");
          if (MorsePreferences::prefs.echoConf) {
              pwmTone(311,  MorsePreferences::prefs.sidetoneVolume, false);
              delay(193);
              pwmNoTone();
          }
      }
      delay(interWordSpace);
      if (MorsePreferences::prefs.speedAdapt)
          changeSpeed(-1);
    }
    echoResponse = "";
    clearPaddleLatches();
}   // end of function



void changeSpeed( int t) {
  MorsePreferences::prefs.wpm += t;
  MorsePreferences::prefs.wpm = constrain(MorsePreferences::prefs.wpm, 5, 60);
  updateTimings();
  displayCWspeed();                     // update display of CW speed
  charCounter = 0;                                    // reset character counter
}














///////////////// a test function for adjusting audio levels

void audioLevelAdjust() {
    uint16_t i, maxi, mini;
    uint16_t testData[1216];

    display.clear();
    printOnScroll(0, BOLD, 0, "Audio Adjust");
    printOnScroll(1, REGULAR, 0, "End with RED");
    keyTx = true;
    keyOut(true,  true, 698, 0);                                  /// we generate a side tone, f=698 Hz, also on line-out, but with vol down on speaker
    while (true) {
        volButton.Update();
        if (volButton.clicks)
            break;                                                /// pressing the red button gets you out of this mode!
        for (i = 0; i < goertzel_n ; ++i)
            testData[i] = analogRead(audioInPin);                 /// read analog input
        maxi = mini = testData[0];
        for (i = 1; i< goertzel_n ; ++i) {
            if (testData[i] < mini)
              mini = testData[i];
            if (testData[i] > maxi)
              maxi = testData[i];
        }
        int a, b, c;
        a = map(mini, 0, 4096, 0, 125);
        b = map(maxi, 0, 4000, 0, 125);
        c = b - a;
        clearLine(2);
        display.drawRect(5, SCROLL_TOP + 2 * LINE_HEIGHT +5, 102, LINE_HEIGHT-8);
        display.drawRect(30, SCROLL_TOP + 2 * LINE_HEIGHT +5, 52, LINE_HEIGHT-8);
        display.fillRect(a, SCROLL_TOP + 2 * LINE_HEIGHT + 7 , c, LINE_HEIGHT -11);
        display.display();
    } // end while
    keyOut(false,  true, 698, 0);                                  /// stop keying
    keyTx = true;
}





