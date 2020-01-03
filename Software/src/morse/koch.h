#ifndef KOCH_H
#define KOCH_H

/// Koch stuff
///////////////////////////////////////////////////

#include <Arduino.h>

namespace Koch
{

    extern String kochChars;

    void setup();
    void updateKochChars(boolean lcwoKochSeq);

    uint8_t wordIsKoch(String thisWord);
    String getChar(uint8_t maxKochLevel);
    String getRandomChars(int maxLength);
    String getRandomWord();
    String getRandomAbbrev();
    boolean isKochActive();
    void setKochActive(boolean newActive);
    void createKochWords(uint8_t maxl, uint8_t koch);
    void createKochAbbr(uint8_t maxl, uint8_t koch);
    String filterNonKoch(String input);
}

#endif
