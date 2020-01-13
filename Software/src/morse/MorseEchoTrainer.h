#ifndef MORSEECHOTRAINER_H_
#define MORSEECHOTRAINER_H_

#include <Arduino.h>

namespace MorseEchoTrainer
{

    ////////////////////////////////////////////////////////////////////
    ///// Variables for Echo Trainer Mode
    /////

    extern String echoResponse;
    enum echoStates
    {
        START_ECHO, SEND_WORD, REPEAT_WORD, GET_ANSWER, COMPLETE_ANSWER, EVAL_ANSWER
    };
    extern String echoTrainerWord;
    extern boolean echoStop;                         // for maxSequence
    extern boolean active;                           // flag for trainer mode
    extern int repeats;


    boolean menuExec(String mode);
    void startEcho();
    boolean loop();
    void onPreferencesChanged();

    void echoTrainerEval();

    void storeCharInResponse(String symbol);
    boolean isState(echoStates state);
    void setState(echoStates newState);
    echoStates getState();
    void changeSpeed(int t);
    unsigned long onGeneratorWordEnd();
    void onGeneratorNewWord(String newWord);
    void onFetchNewWord();
    void onLastWord();

}

#endif /* MORSEECHOTRAINER_H_ */
