#ifndef MORSESOUND_H_
#define MORSESOUND_H_

#include <Arduino.h>

namespace MorseSound {

    void pwmTone(unsigned int frequency, unsigned int volume, boolean lineOut);
    void pwmNoTone();
    void pwmClick(unsigned int volume);

}



#endif /* MORSESOUND_H_ */
