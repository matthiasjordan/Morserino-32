#ifndef MORSEGENERATOR_H_
#define MORSEGENERATOR_H_

#include "MorsePreferences.h"

namespace MorseGenerator
{

    enum GEN_TYPE
    {
        NA, RANDOMS, ABBREVS, WORDS, CALLS, MIXED, PLAYER, KOCH_MIXED, KOCH_LEARN
    };
    // the things we can generate in generator mode

    enum MORSE_TYPE
    {
        KEY_DOWN, KEY_UP, WORD_BREAK
    };
    //   State Machine Defines

    enum AutoStopModes
    {
        off, stop1, stop2
    };

    enum Timing {
        quick, tx, rx
    };

    typedef struct generator_config {
            boolean sendCWToLoRa;
            boolean printSpaceAfterWord;
            boolean printLFAfterWord;
            boolean printDitDah;
            Timing timing;
            boolean key;
    } Config;

    extern AutoStopModes autoStop;

    extern unsigned char generatorState; // should be MORSE_TYPE instead of uns char
    extern unsigned long genTimer;                         // timer used for generating morse code in trainer mode
    extern String CWword;

    extern String clearText;

    extern int repeats;
    extern uint8_t wordCounter;                          // for maxSequence

    extern GEN_TYPE generatorMode;          // trainer: what symbol (groups) are we going to send?            0 -  5

    extern uint8_t effectiveTrainerDisplay;

    extern boolean stopFlag;                         // for maxSequence
    extern boolean effectiveAutoStop;                 // If to stop after each word in generator modes
    extern boolean firstTime;                         /// for word doubler mode

    void setup();
    boolean menuExec(String mode);
    void startTrainer();
    void generateCW();
    void keyOut(boolean on, boolean fromHere, int f, int volume);
    void setupHeadCopying();

    void setNextWordvvvA(); // to indicate that we want vvvA
    void setSendCWToLoRa(boolean mode);

}

#endif /* MORSEGENERATOR_H_ */
