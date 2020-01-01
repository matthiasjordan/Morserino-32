#ifndef MORSEGENERATOR_H_
#define MORSEGENERATOR_H_

#include "MorsePreferences.h"

namespace MorseGenerator {

    enum GEN_TYPE { NA, RANDOMS, ABBREVS, WORDS, CALLS, MIXED, PLAYER, KOCH_MIXED, KOCH_LEARN };              // the things we can generate in generator mode

    enum MORSE_TYPE {KEY_DOWN, KEY_UP };                    //   State Machine Defines

    enum AutoStopModes {off, stop1, stop2}  autoStop = off;

    unsigned char generatorState; // should be MORSE_TYPE instead of uns char
    unsigned long genTimer;                         // timer used for generating morse code in trainer mode
    boolean startFirst = true;                        // to indicate that we are starting a new sequence in the trainer modi
    String CWword = "";


       String clearText = "";

       int repeats = 0;
       uint8_t wordCounter = 0;                          // for maxSequence

       int rxDitLength = 0;                    // set new value for length of dits and dahs and other timings
       int rxDahLength = 0;
       int rxInterCharacterSpace = 0;
       int rxInterWordSpace = 0;

       GEN_TYPE generatorMode = RANDOMS;          // trainer: what symbol (groups) are we going to send?            0 -  5

       uint8_t effectiveTrainerDisplay = MorsePreferences::prefs.trainerDisplay;

       boolean stopFlag = false;                         // for maxSequence
       boolean effectiveAutoStop = false;                 // If to stop after each word in generator modes
       boolean firstTime = true;                         /// for word doubler mode

       void generateCW();
       void keyOut(boolean on,  boolean fromHere, int f, int volume);
       void setupHeadCopying();

}




#endif /* MORSEGENERATOR_H_ */
