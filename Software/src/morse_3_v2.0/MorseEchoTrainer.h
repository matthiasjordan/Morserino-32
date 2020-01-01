#ifndef MORSEECHOTRAINER_H_
#define MORSEECHOTRAINER_H_

#include <Arduino.h>

namespace MorseEchoTrainer {


    ////////////////////////////////////////////////////////////////////
    ///// Variables for Echo Trainer Mode
    /////

    String echoResponse = "";
    enum echoStates { START_ECHO, SEND_WORD, REPEAT_WORD, GET_ANSWER, COMPLETE_ANSWER, EVAL_ANSWER };
    String echoTrainerPrompt;
    String echoTrainerWord;
    boolean echoStop = false;                         // for maxSequence
    boolean active = false;                           // flag for trainer mode

    void echoTrainerEval();

    void storeCharInResponse(String symbol);
    boolean isState(echoStates state);
    void setState(echoStates newState);
    echoStates getState();
    void changeSpeed( int t);

}




#endif /* MORSEECHOTRAINER_H_ */
