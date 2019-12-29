#include <Arduino.h>

#include "MorseGenerator.h"
#include "MorseDisplay.h"
#include "MorseMachine.h"
#include "MorsePreferences.h"
#include "MorseLoRa.h"
#include "MorseKeyer.h"
#include "MorseSound.h"
#include "MorseEchoTrainer.h"


using namespace MorseGenerator;

namespace MorseGenerator::internal {
    void fetchNewWord();
    void keyOut(boolean on,  boolean fromHere, int f, int volume);
    void dispGeneratedChar();
    String CWwordToClearText(String cwword);
    String encodeProSigns( String &input );

}

void generateCW () {          // this is called from loop() (frequently!)  and generates CW

  static int l;
  static char c;
  boolean silentEcho;

  switch (generatorState) {                                             // CW generator state machine - key is up or down
    case KEY_UP:
            if (millis() < genTimer)                                    // not yet at end of the pause: just wait
                return;                                                 // therefore we return to loop()
             // here we continue if the pause has been long enough
            if (startFirst == true)
                CWword = "";
            l = CWword.length();

            if (l==0)  {                                               // fetch a new word if we have an empty word
                if (clearText.length() > 0) {                          // this should not be reached at all.... except when display word by word
                  //Serial.println("Text left: " + clearText);
                  if (MorseMachine::isMode(loraTrx) || (MorseMachine::isMode(morseGenerator) && effectiveTrainerDisplay == DISPLAY_BY_WORD) ||
                        ( MorseMachine::isMode(echoTrainer) && MorsePreferences::prefs.echoDisplay != CODE_ONLY)) {
                      MorseDisplay::printToScroll(BOLD,MorseDisplay::cleanUpProSigns(clearText));
                      clearText = "";
                  }
                }
                internal::fetchNewWord();
                //Serial.println("New Word: " + CWword);
                if (CWword.length() == 0)                             // we really should have something here - unless in trx mode; in this case return
                  return;
                if ((MorseMachine::isMode(echoTrainer))) {
                    MorseDisplay::printToScroll(REGULAR, "\n");
                }
            }
            c = CWword[0];                                            // retrieve next element from CWword; if 0, we were at end of character
            CWword.remove(0,1);
            if (c == '0' || !CWword.length())  {                      // a character just had been finished //// is there an error here?
                   if (c == '0') {
                      c = CWword[0];                                  // retrieve next element from CWword;
                      CWword.remove(0,1);
                      if (MorseMachine::isMode(morseGenerator) && MorsePreferences::prefs.loraTrainerMode == 1)
                          MorseLoRa::cwForLora(0);                             // send end of character to lora
                      }
            }   /// at end of character

            //// insert code here for outputting only on display, and not as morse characters - for echo trainer
            //// genTimer vy short (1ms?)
            //// no keyOut()
            if (MorseMachine::isMode(echoTrainer) && MorsePreferences::prefs.echoDisplay == DISP_ONLY)
                genTimer = millis() + 2;      // very short timing
            else if (MorseMachine::isMode(loraTrx))
                genTimer = millis() + (c == '1' ? MorseKeyer::ditLength : MorseKeyer::dahLength);           // start a dit or a dah, acording to the next element
            else
                genTimer = millis() + (c == '1' ? rxDitLength : rxDahLength);
            if (MorseMachine::isMode(morseGenerator) && MorsePreferences::prefs.loraTrainerMode == 1)             // send the element to LoRa
                c == '1' ? MorseLoRa::cwForLora(1) : MorseLoRa::cwForLora(2) ;
            /// if Koch learn character we show dit or dah
            if (generatorMode == KOCH_LEARN)
                MorseDisplay::printToScroll(REGULAR, c == '1' ? "."  : "-");

            silentEcho = (MorseMachine::isMode(echoTrainer) && MorsePreferences::prefs.echoDisplay == DISP_ONLY); // echo mode with no audible prompt

            if (silentEcho || stopFlag)                                             // we finished maxSequence and so do start output (otherwise we get a short click)
              ;
            else  {
                internal::keyOut(true, (!MorseMachine::isMode(loraTrx)), notes[MorsePreferences::prefs.sidetoneFreq], MorsePreferences::prefs.sidetoneVolume);
            }
            /* // replaced by the lines above, to also take care of maxSequence
            if ( ! (morseState == echoTrainer && MorsePreferences::prefs.echoDisplay == DISP_ONLY))

                        keyOut(true, (morseState != loraTrx), notes[MorsePreferences::prefs.sidetoneFreq], MorsePreferences::prefs.sidetoneVolume);
            */
            generatorState = KEY_DOWN;                              // next state = key down = dit or dah

            break;
    case KEY_DOWN:
            if (millis() < genTimer)                                // if not at end of key down we need to wait, so we just return to loop()
                return;
            //// otherwise we continue here; stop keying,  and determine the length of the following pause: inter Element, interCharacter or InterWord?

            internal::keyOut(false, (!MorseMachine::isMode(loraTrx)), 0, 0);
            if (! CWword.length())   {                                 // we just ended the the word, ...  //// intercept here in Echo Trainer mode
 //             // display last character - consider echo mode!
                if (MorseMachine::isMode(morseGenerator))
                    autoStop = effectiveAutoStop ? stop1 : off;
                internal::dispGeneratedChar();
                if (MorseMachine::isMode(echoTrainer)) {
                    switch (MorseEchoTrainer::echoTrainerState) {
                        case MorseEchoTrainer::START_ECHO:  MorseEchoTrainer::echoTrainerState = MorseEchoTrainer::SEND_WORD;
                                          genTimer = millis() + MorseKeyer::interCharacterSpace + (MorsePreferences::prefs.promptPause * MorseKeyer::interWordSpace);
                                          break;
                        case MorseEchoTrainer::REPEAT_WORD:
                                          // fall through
                        case MorseEchoTrainer::SEND_WORD:   if (MorseEchoTrainer::echoStop)
                                                break;
                                          else {
                                              MorseEchoTrainer::echoTrainerState = MorseEchoTrainer::GET_ANSWER;
                                                if (MorsePreferences::prefs.echoDisplay != CODE_ONLY) {
                                                    MorseDisplay::printToScroll(REGULAR, " ");
                                                    MorseDisplay::printToScroll(INVERSE_REGULAR, ">");    /// add a blank after the word on the display
                                                }
                                                ++repeats;
                                                genTimer = millis() + MorsePreferences::prefs.responsePause * MorseKeyer::interWordSpace;
                                          }
                        default:          break;
                    }
                }
                else {
                      genTimer = millis() + (MorseMachine::isMode(loraTrx) ? rxInterWordSpace : MorseKeyer::interWordSpace) ;              // we need a pause for interWordSpace
                      if (MorseMachine::isMode(morseGenerator) && MorsePreferences::prefs.loraTrainerMode == 1) {                                   // in generator mode and we want to send with LoRa
                          MorseLoRa::cwForLora(0);
                          MorseLoRa::cwForLora(3);                           // as we have just finished a word
                          //Serial.println("cwForLora(3);");
                          MorseLoRa::sendWithLora();                         // finalise the string and send it to LoRA
                          delay(MorseKeyer::interCharacterSpace+MorseKeyer::ditLength);             // we need a slightly longer pause otherwise the receiving end might fall too far behind...
                          }
                }
             }
             else if ((c = CWword[0]) == '0') {                                                                        // we are at end of character
//              // display last character
//              // genTimer small if in echo mode and no code!
                 internal::dispGeneratedChar();
                if (MorseMachine::isMode(echoTrainer) && MorsePreferences::prefs.echoDisplay == DISP_ONLY)
                    genTimer = millis() +1;
                else
                    genTimer = millis() + (MorseMachine::isMode(loraTrx) ? rxInterCharacterSpace : MorseKeyer::interCharacterSpace);          // pause = intercharacter space
             }
             else  {                                                                                                   // we are in the middle of a character
                genTimer = millis() + (MorseMachine::isMode(loraTrx) ? rxDitLength : MorseKeyer::ditLength);                              // pause = interelement space
             }
             generatorState = KEY_UP;                               // next state = key up = pause
             break;
  }   /// end switch (generatorState)
}


void fetchNewWord() {
  int rssi, rxWpm, rv;

//Serial.println("startFirst: " + String((startFirst ? "true" : "false")));
//Serial.println("firstTime: " + String((firstTime ? "true" : "false")));
    if (MorseMachine::isMode(loraTrx)) {                                // we check the rxBuffer and see if we received something
       MorseDisplay::updateSMeter(0);                                         // at end of word we set S-meter to 0 until we receive something again
       //Serial.print("end of word - S0? ");
       startFirst = false;
       ////// from here: retrieve next CWword from buffer!
        if (MorseLoRa::loRaBuReady()) {
            MorseDisplay::printToScroll(BOLD, " ");
            uint8_t header = MorseLoRa::decodePacket(&rssi, &rxWpm, &CWword);
            //Serial.println("Header: " + (String) header);
            //Serial.println("CWword: " + (String) CWword);
            //Serial.println("Speed: " + (String) rxWpm);

            if ((header >> 6) != 1)                             // invalid protocol version
              return;
            if ((rxWpm < 5) || (rxWpm >60))                    // invalid speed
              return;
            clearText = internal::CWwordToClearText(CWword);
            //Serial.println("clearText: " + (String) clearText);
            //Serial.println("RX Speed: " + (String)rxWpm);
            //Serial.println("RSSI: " + (String)rssi);

            rxDitLength = 1200 /   rxWpm ;                      // set new value for length of dits and dahs and other timings
            rxDahLength = 3* rxDitLength ;                      // calculate the other timing values
            rxInterCharacterSpace = 3 * rxDitLength;
            rxInterWordSpace = 7 * rxDitLength;
            MorseDisplay::vprintOnStatusLine(true, 4, "%2ir", rxWpm);
            MorseDisplay::printOnStatusLine(true, 9, "s");
            MorseDisplay::updateSMeter(rssi);                                 // indicate signal strength of new packet
       }
       else return;                                             // we did not receive anything

    } // end if loraTrx
    else {

    //if (morseState != echoTrainer)
    if ((MorseMachine::isMode(morseGenerator)) && !effectiveAutoStop) {
        MorseDisplay::printToScroll(REGULAR, " ");    /// in any case, add a blank after the word on the display
    }

    if (generatorMode == KOCH_LEARN) {
        startFirst = false;
        echoTrainerState = SEND_WORD;
    }
    if (startFirst == true)  {                                 /// do the intial sequence in trainer mode, too
        clearText = "vvvA";
        startFirst = false;
    } else if (MorseMachine::isMode(morseGenerator) && MorsePreferences::prefs.wordDoubler == true && firstTime == false) {
        clearText = echoTrainerWord;
        firstTime = true;
    } else if (MorseMachine::isMode(echoTrainer)) {
        interWordTimer = 4294967000;                   /// interword timer should not trigger something now
        //Serial.println("echoTrainerState: " + String(echoTrainerState));
        switch (echoTrainerState) {
            case  REPEAT_WORD:  if (MorsePreferences::prefs.echoRepeats == 7 || repeats <= MorsePreferences::prefs.echoRepeats)
                                    clearText = echoTrainerWord;
                                else {
                                    clearText = echoTrainerWord;
                                    if (generatorMode != KOCH_LEARN) {
                                        MorseDisplay::printToScroll(INVERSE_REGULAR, MorseDisplay::cleanUpProSigns(clearText));    //// clean up first!
                                        MorseDisplay::printToScroll(REGULAR, " ");
                                    }
                                    goto randomGenerate;
                                }
                                break;
            //case  START_ECHO:
            case  SEND_WORD:    goto randomGenerate;
            default:            break;
        }   /// end special cases for echo Trainer
    } else {

      randomGenerate:       repeats = 0;
                            if (((MorseMachine::isMode(morseGenerator)) || (MorseMachine::isMode(echoTrainer))) && (MorsePreferences::prefs.maxSequence != 0) &&
                                    (generatorMode != KOCH_LEARN))  {                                           // a case for maxSequence
                                ++ wordCounter;
                                int limit = 1 + MorsePreferences::prefs.maxSequence;
                                if (wordCounter == limit) {
                                  clearText = "+";
                                    echoStop = true;
                                }
                                else if (wordCounter == (limit+1)) {
                                    stopFlag = true;
                                    echoStop = false;
                                    wordCounter = 1;
                                }
                            }
                            if (clearText != "+") {
                                switch (generatorMode) {
                                      case  RANDOMS:  clearText = getRandomChars(MorsePreferences::prefs.randomLength, MorsePreferences::prefs.randomOption);
                                                      break;
                                      case  CALLS:    clearText = getRandomCall(MorsePreferences::prefs.callLength);
                                                      break;
                                      case  ABBREVS:  clearText = getRandomAbbrev(MorsePreferences::prefs.abbrevLength);
                                                      break;
                                      case  WORDS:    clearText = getRandomWord(MorsePreferences::prefs.wordLength);
                                                      break;
                                      case  KOCH_LEARN:clearText = (String) kochChars.charAt(MorsePreferences::prefs.kochFilter - 1);
                                                      break;
                                      case  MIXED:    rv = random(4);
                                                      switch (rv) {
                                                        case  0:  clearText = getRandomWord(MorsePreferences::prefs.wordLength);
                                                                  break;
                                                        case  1:  clearText = getRandomAbbrev(MorsePreferences::prefs.abbrevLength);
                                                                    break;
                                                        case  2:  clearText = getRandomCall(MorsePreferences::prefs.callLength);
                                                                  break;
                                                        case  3:  clearText = getRandomChars(1,OPT_PUNCTPRO);        // just a single pro-sign or interpunct
                                                                  break;
                                                      }
                                                      break;
                                      case KOCH_MIXED:rv = random(3);
                                                      switch (rv) {
                                                        case  0:  clearText = getRandomWord(MorsePreferences::prefs.wordLength);
                                                                  break;
                                                        case  1:  clearText = getRandomAbbrev(MorsePreferences::prefs.abbrevLength);
                                                                    break;
                                                        case  2:  clearText = getRandomChars(MorsePreferences::prefs.randomLength, OPT_KOCH);        // Koch option!
                                                                  break;
                                                      }
                                                      break;
                                      case PLAYER:    if (MorsePreferences::prefs.randomFile)
                                                          skipWords(random(MorsePreferences::prefs.randomFile+1));
                                                      clearText = getWord();
                                                      /*
                                                      if (clearText == String()) {        /// at end of file: go to beginning again
                                                        MorsePreferences::prefs.fileWordPointer = 0;
                                                        file.close(); file = SPIFFS.open("/player.txt");
                                                      }
                                                      ++MorsePreferences::prefs.fileWordPointer;
                                                      */
                                                      clearText = cleanUpText(clearText);
                                                      break;
                                      case NA: break;
                                    }   // end switch (generatorMode)
                            }
                            firstTime = false;
      }       /// end if else - we either already had something in trainer mode, or we got a new word

      CWword = generateCWword(clearText);
      echoTrainerWord = clearText;
    } /// else (= not in loraTrx mode)
} // end of fetchNewWord()



void MorseGenerator::internal::keyOut(boolean on,  boolean fromHere, int f, int volume) {
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
        MorseSound::pwmTone(intPitch, volume, true);
        MorseKeyer::keyTransmitter();
      } else {                    // not from here
        extTone = true;
        extPitch = f;
        if (!intTone)
            MorseSound::pwmTone(extPitch, volume, false);
        }
  } else {                      // key off
        if (fromHere) {
          intTone = false;
          if (extTone)
              MorseSound::pwmTone(extPitch, volume, false);
          else
              MorseSound::pwmNoTone();
          digitalWrite(keyerPin, LOW);      // stop keying Tx
        } else {                 // not from here
          extTone = false;
          if (!intTone)
              MorseSound::pwmNoTone();
        }
  }   // end key off
}



/// when generating CW, we display the character (under certain circumstances)
/// add code to display in echo mode when parameter is so set
/// MorsePreferences::prefs.echoDisplay 1 = CODE_ONLY 2 = DISP_ONLY 3 = CODE_AND_DISP

void MorseGenerator::internal::dispGeneratedChar() {
  static String charString;
  charString.reserve(10);

  if (generatorMode == KOCH_LEARN || MorseMachine::isMode(loraTrx) || (MorseMachine::isMode(morseGenerator) && effectiveTrainerDisplay == DISPLAY_BY_CHAR) ||
                    ( MorseMachine::isMode(echoTrainer) && MorsePreferences::prefs.echoDisplay != CODE_ONLY ))
                    //&& echoTrainerState != SEND_WORD
                    //&& echoTrainerState != REPEAT_WORD))

      {       /// we need to output the character on the display now
        charString = clearText.charAt(0);                   /// store first char of clearText in charString
        clearText.remove(0,1);                              /// and remove it from clearText
        if (generatorMode == KOCH_LEARN) {
            MorseDisplay::printToScroll(REGULAR,"");                      // clear the buffer first
        }
        MorseDisplay::printToScroll(MorseMachine::isMode(loraTrx) ? BOLD : REGULAR, MorseDisplay::cleanUpProSigns(charString));
        if (generatorMode == KOCH_LEARN)
            MorseDisplay::printToScroll(REGULAR," ");                      // output a space
      }   //// end display_by_char

      ++charCounter;                         // count this character

     // if we have seen 12 chars since changing speed, we write the config to Preferences
     if (charCounter == 12) {
        pref.begin("morserino", false);             // open the namespace as read/write
        pref.putUChar("wpm", MorsePreferences::prefs.wpm);
        pref.end();
     }
}



String MorseGenerator::internal::CWwordToClearText(String cwword) {             // decode the Morse code character in cwword to clear text
  int ptr = 0;
  String result;
  result.reserve(40);
  String symbol;
  symbol.reserve(6);


  result = "";
  for (int i = 0; i < cwword.length(); ++i) {
      char c = cwword[i];
      switch (c) {
          case '1': ptr = CWtree[ptr].dit;
                    break;
          case '2': ptr = CWtree[ptr].dah;
                    break;
          case '0': symbol = CWtree[ptr].symb;

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


String MorseGenerator::internal::encodeProSigns( String &input ) {
    /// clean up clearText   -   S <as>,  - A <ka> - N <kn> - K <sk> - H ch - V <ve>;
    input.replace("<as>", "S");
    input.replace("<ka>","A");
    input.replace("<kn>","N");
    input.replace("<sk>","K");
    input.replace("<ve>","V");
    input.replace("<ch>","H");
    input.replace("<err>","E");
    input.replace("¬", "U");
    //Serial.println(input);
    return input;
}

