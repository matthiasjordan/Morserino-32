#ifndef MORSESYSTEM_H_
#define MORSESYSTEM_H_

#include <Arduino.h>

namespace MorseSystem
{

    boolean menuExec(String mode);

    void resetTOT();
    void checkShutDown(boolean enforce);

    int16_t batteryVoltage();
    void shutMeDown();

}

#endif /* MORSESYSTEM_H_ */
