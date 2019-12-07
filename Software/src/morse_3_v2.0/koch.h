#ifndef KOCH_H
#define KOCH_H

/// Koch stuff
///////////////////////////////////////////////////

#include <Arduino.h>

namespace Koch
{

    void updateKochChars(boolean lcwoKochSeq);

    uint8_t wordIsKoch(String thisWord);

}

#endif
