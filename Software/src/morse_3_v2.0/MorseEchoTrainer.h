#ifndef MORSEECHOTRAINER_H_
#define MORSEECHOTRAINER_H_

#include <Arduino.h>

namespace MorseEchoTrainer {


    ////////////////////////////////////////////////////////////////////
    ///// Variables for Echo Trainer Mode
    /////

    extern String echoResponse;
    enum echoStates { START_ECHO, SEND_WORD, REPEAT_WORD, GET_ANSWER, COMPLETE_ANSWER, EVAL_ANSWER };
    extern String echoTrainerPrompt;
    extern String echoTrainerWord;
    extern boolean echoStop;                         // for maxSequence
    extern boolean active;                           // flag for trainer mode

    void echoTrainerEval();

    void storeCharInResponse(String symbol);
    boolean isState(echoStates state);
    void setState(echoStates newState);
    echoStates getState();
    void changeSpeed( int t);

}




#endif /* MORSEECHOTRAINER_H_ */
