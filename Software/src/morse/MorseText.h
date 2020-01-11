#ifndef MORSETEXT_H_
#define MORSETEXT_H_


namespace MorseText {

    enum GEN_TYPE
    {
        NA, RANDOMS, ABBREVS, WORDS, CALLS, MIXED, PLAYER, KOCH_MIXED, KOCH_LEARN
    };

    extern GEN_TYPE generatorMode;

    void start(GEN_TYPE genType);
    void proceed();
    String getCurrentWord();
    String generateWord();

}


#endif /* MORSETEXT_H_ */
