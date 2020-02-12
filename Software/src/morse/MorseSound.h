#ifndef MORSESOUND_H_
#define MORSESOUND_H_

#include <Arduino.h>

namespace MorseSound
{
    const int notes[] =
        {0, 233, 262, 294, 311, 349, 392, 440, 466, 523, 587, 622, 698, 784, 880, 932};

    void setup();
    void pwmTone(unsigned int frequency, unsigned int volume, boolean lineOut);
    void pwmNoTone();
    void pwmClick(unsigned int volume);
    void soundSignalOK();
    void soundSignalERR();

}

#endif /* MORSESOUND_H_ */
