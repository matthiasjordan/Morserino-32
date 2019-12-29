#include "MorseKeyer.h"
#include "MorsePreferences.h"

using namespace MorseKeyer;


void updateTimings() {
  ditLength = 1200 / MorsePreferences::prefs.wpm;                    // set new value for length of dits and dahs and other timings
  dahLength = 3 * ditLength;
  interCharacterSpace =  MorsePreferences::prefs.interCharSpace *  ditLength;
  //interWordSpace = _max(p_interWordSpace * ditLength, (p_interCharSpace+6)*ditLength);
  interWordSpace = _max(MorsePreferences::prefs.interWordSpace, MorsePreferences::prefs.interCharSpace+4) * ditLength;

  effWpm = 60000 / (31 * ditLength + 4 * interCharacterSpace + interWordSpace );  ///  effective wpm with lengthened spaces = Farnsworth speed
}


void keyTransmitter() {
  if (MorsePreferences::prefs.keyTrainerMode == 0 || morseState == echoTrainer || morseState == loraTrx )
      return;                              // we never key Tx under these conditions
  if (MorsePreferences::prefs.keyTrainerMode == 1 && morseState == morseGenerator)
      return;                              // we key only in keyer mode; in all other case we key TX
  if (keyTx == false)
      return;                              // do not key when input is from tone decoder
   digitalWrite(keyerPin, HIGH);           // turn the LED on, key transmitter, or whatever
}
