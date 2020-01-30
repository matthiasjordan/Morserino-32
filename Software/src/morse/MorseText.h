#ifndef MORSETEXT_H_
#define MORSETEXT_H_

namespace MorseText
{

    typedef struct {
            const String internal;
            const String code;
            const String prosign;
    } MorseChar;

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

    extern Config config;

    extern const MorseChar morseChars[];

    extern void (*onGeneratorNewWord)(String);

    void start(GEN_TYPE genType);
    Config* getConfig();

    void proceed();
    String getCurrentWord();
    String generateWord();
    void setNextWordIsEndSequence();
    void setRepeatLast();
    int findChar(char c);                                 // at which position is the character in CWchars?
    String utf8umlaut(String s);
    String internalToProSigns(String &input);
    String proSignsToInternal(String &input);

}

#endif /* MORSETEXT_H_ */
