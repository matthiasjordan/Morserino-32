#ifndef MORSETEXT_H_
#define MORSETEXT_H_

namespace MorseText
{

    enum GEN_TYPE
    {
        NA, RANDOMS, ABBREVS, WORDS, CALLS, MIXED, PLAYER, KOCH_MIXED, KOCH_LEARN
    };

    typedef struct
    {
            boolean generateStartSequence;
            uint8_t repeatEach = 1;
            GEN_TYPE generatorMode;          // trainer: what symbol (groups) are we going to send?            0 -  5
    } Config;

    extern void (*onGeneratorNewWord)(String);

    void start(GEN_TYPE genType);
    void start(MorseText::Config *config);

    void setTextSource(GEN_TYPE genType);
    void setGenerateStartSequence(boolean newValue);
    void setRepeatEach(uint8_t n);

    void onPreferencesChanged();
    void proceed();
    String getCurrentWord();
    String generateWord();
    void setNextWordIsEndSequence();
    void setRepeatLast();
}

#endif /* MORSETEXT_H_ */
