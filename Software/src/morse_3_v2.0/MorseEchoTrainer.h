#ifndef MORSEECHOTRAINER_H_
#define MORSEECHOTRAINER_H_

#include <Arduino.h>

namespace MorseEchoTrainer {


    ////////////////////////////////////////////////////////////////////
    ///// Variables for Echo Trainer Mode
    /////

    String echoResponse = "";
    enum echoStates { START_ECHO, SEND_WORD, REPEAT_WORD, GET_ANSWER, COMPLETE_ANSWER, EVAL_ANSWER };
    echoStates echoTrainerState = START_ECHO;
    String echoTrainerPrompt, echoTrainerWord;
    boolean echoStop = false;                         // for maxSequence

}




#endif /* MORSEECHOTRAINER_H_ */
