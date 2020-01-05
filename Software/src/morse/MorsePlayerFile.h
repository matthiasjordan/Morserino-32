#ifndef MORSEPLAYERFILE_H_
#define MORSEPLAYERFILE_H_

#include "FS.h"

namespace MorsePlayerFile
{

    void setup();
    String getWord();
    void skipWords(uint32_t count);
    File openForWriting();
    void openAndSkip();

}

#endif /* MORSEPLAYERFILE_H_ */
