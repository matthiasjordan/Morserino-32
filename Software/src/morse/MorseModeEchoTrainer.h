#ifndef MORSEMODEECHOTRAINER_H_
#define MORSEMODEECHOTRAINER_H_

#include <Arduino.h>
#include "MorseMode.h"

class MorseModeEchoTrainer: public MorseMode
{
    public:
        typedef struct
        {
                boolean generateStartSequence;
                boolean showPrompt;
                boolean showFailedWord;
        } Config;

        boolean menuExec(String mode) override;
        void onPreferencesChanged() override;
        boolean loop() override;
        boolean togglePause() override;
        void onFetchNewWord();
        void startEcho();


        enum echoStates
        {
            START_ECHO, SEND_WORD, REPEAT_WORD, GET_ANSWER, COMPLETE_ANSWER, EVAL_ANSWER
        };

        void setState(echoStates newState);
        boolean isState(echoStates state);

    private:
        ////////////////////////////////////////////////////////////////////
        ///// Variables for Echo Trainer Mode
        /////


        String echoResponse = "";
        String echoTrainerWord;
        boolean echoStop;                         // for maxSequence
        boolean active;                           // flag for trainer mode
        int repeats;
        echoStates echoTrainerState;


        void echoTrainerEval();

        void storeCharInResponse(String symbol);
        echoStates getState();
        void changeSpeed(int t);
        unsigned long onGeneratorWordEnd();
        void onGeneratorNewWord(String newWord);
        void onLastWord();

        boolean onKeyerWordEnd();
        void onKeyerWordEndDitDah();
        void onKeyerWordEndNDitDah();

};

extern MorseModeEchoTrainer morseModeEchoTrainer;

#endif /* MORSEMODEECHOTRAINER_H_ */
