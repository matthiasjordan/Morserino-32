#ifndef MORSEGENERATOR_H_
#define MORSEGENERATOR_H_

#include "MorsePreferences.h"

namespace MorseGenerator
{
    // the things we can generate in generator mode

    enum MORSE_TYPE
    {
        KEY_DOWN, KEY_UP, WORD_BREAK
    };
    //   State Machine Defines

    enum AutoStopState
    {
        off, stop1, stop2
    };

    enum Timing
    {
        quick, tx, rx
    };

    enum WordEndMethod {
        shrug, LF, flush
    };

    typedef struct generator_config
    {
        boolean sendCWToLoRa;
        boolean printSpaceAfterWord;
        WordEndMethod wordEndMethod;
        boolean printDitDah;
        boolean printChar;
        Timing timing;
        boolean key;
        boolean clearBufferBeforPrintChar;
        boolean printSpaceAfterChar;
        FONT_ATTRIB printCharStyle;
        boolean autoStop; // If to stop after each word in generator modes
        uint8_t effectiveTrainerDisplay;
    } Config;

    extern AutoStopState autoStopState;

    extern unsigned char generatorState; // should be MORSE_TYPE instead of uns char
    extern unsigned long genTimer;                         // timer used for generating morse code in trainer mode
    extern String CWword;

    extern String clearText;

    extern int repeats;
    extern uint8_t wordCounter;                          // for maxSequence

//    extern uint8_t effectiveTrainerDisplay;

    extern boolean stopFlag;                         // for maxSequence
    extern boolean firstTime;                         /// for word doubler mode

    void setup();
    void setStart();
    Config* getConfig();
    boolean menuExec(String mode);
    void loop();
    void startTrainer();
    void generateCW();
    void keyOut(boolean on, boolean fromHere, int f, int volume);
    void setupHeadCopying();
    void onPreferencesChanged();
    void setNextWordvvvA(); // to indicate that we want vvvA
    void setSendCWToLoRa(boolean mode);

}

#endif /* MORSEGENERATOR_H_ */
