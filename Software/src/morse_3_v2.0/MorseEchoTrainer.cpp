#include "MorseEchoTrainer.h"
#include "MorsePreferences.h"
#include "MorseKeyer.h"
#include "MorseSound.h"
#include "MorseDisplay.h"
#include "MorseGenerator.h"




using namespace MorseEchoTrainer;

String MorseEchoTrainer::echoResponse = "";
echoStates echoTrainerState = START_ECHO;
boolean MorseEchoTrainer::echoStop = false;                         // for maxSequence
boolean MorseEchoTrainer::active = false;                           // flag for trainer mode
String MorseEchoTrainer::echoTrainerWord;



boolean MorseEchoTrainer::isState(echoStates state) {
    return echoTrainerState == state;
}

void MorseEchoTrainer::setState(echoStates newState) {
    echoTrainerState = newState;
}

echoStates MorseEchoTrainer::getState() {
    return echoTrainerState;
}


void MorseEchoTrainer::storeCharInResponse(String symbol) {
    symbol.replace("<as>", "S");
    symbol.replace("<ka>", "A");
    symbol.replace("<kn>", "N");
    symbol.replace("<sk>", "K");
    symbol.replace("<ve>", "V");
    symbol.replace("<ch>", "H");
    echoResponse.concat(symbol);
}


///////// evaluate the response in Echo Trainer Mode
void MorseEchoTrainer::echoTrainerEval() {
    delay(MorseKeyer::interCharacterSpace / 2);
    if (echoResponse == echoTrainerWord) {
      echoTrainerState = SEND_WORD;
      MorseDisplay::printToScroll(BOLD,  "OK");
      if (MorsePreferences::prefs.echoConf) {
          MorseSound::pwmTone(440,  MorsePreferences::prefs.sidetoneVolume, false);
          delay(97);
          MorseSound::pwmNoTone();
          MorseSound::pwmTone(587,  MorsePreferences::prefs.sidetoneVolume, false);
          delay(193);
          MorseSound::pwmNoTone();
      }
      delay(MorseKeyer::interWordSpace);
      if (MorsePreferences::prefs.speedAdapt)
          changeSpeed(1);
    } else {
      echoTrainerState = REPEAT_WORD;
      if (MorseGenerator::generatorMode != MorseGenerator::KOCH_LEARN || echoResponse != "") {
          MorseDisplay::printToScroll(BOLD, "ERR");
          if (MorsePreferences::prefs.echoConf) {
              MorseSound::pwmTone(311,  MorsePreferences::prefs.sidetoneVolume, false);
              delay(193);
              MorseSound::pwmNoTone();
          }
      }
      delay(MorseKeyer::interWordSpace);
      if (MorsePreferences::prefs.speedAdapt)
          changeSpeed(-1);
    }
    echoResponse = "";
    MorseKeyer::clearPaddleLatches();
}   // end of function


void MorseEchoTrainer::changeSpeed( int t) {
  MorsePreferences::prefs.wpm += t;
  MorsePreferences::prefs.wpm = constrain(MorsePreferences::prefs.wpm, 5, 60);
  MorseKeyer::updateTimings();
  MorseDisplay::displayCWspeed();                     // update display of CW speed
  MorsePreferences::charCounter = 0;                                    // reset character counter
}
