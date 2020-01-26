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

    enum Timing
    {
        quick, tx, rx
    };

    enum WordEndMethod
    {
        space, flush, LF, spaceAndFlush, nothing
    };

    typedef struct generator_config
    {
            boolean sendCWToLoRa;
            WordEndMethod wordEndMethod;
            boolean printDitDah;
            boolean printChar;
            Timing timing;
            boolean key;
            boolean clearBufferBeforPrintChar;
            boolean printSpaceAfterChar;
            FONT_ATTRIB printCharStyle;
            uint8_t effectiveTrainerDisplay;

            void (*onFetchNewWord)(); // Called when the generator fetches a new word from MorseText
            unsigned long (*onGeneratorWordEnd)(); // Called when the generator just sent the last character of the word
            void (*onLastWord)(); // Called when the generator finished the last word
    } Config;

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
    void startTrainer();
    void generateCW();
    void keyOut(boolean on, boolean fromHere, int f, int volume);
    void setNextWordvvvA(); // to indicate that we want vvvA
    void setSendCWToLoRa(boolean mode);
    void handleEffectiveTrainerDisplay(uint8_t mode);

}

#endif /* MORSEGENERATOR_H_ */
