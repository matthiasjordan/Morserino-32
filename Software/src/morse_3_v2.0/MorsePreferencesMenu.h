#ifndef MORSEMENU_H_
#define MORSEMENU_H_

#include "MorsePreferences.h"

namespace MorsePreferencesMenu {

    boolean setupPreferences(uint8_t atMenu);
    boolean setupPreferences(uint8_t atMenu);
    void displayKeyerPreferencesMenu(int pos);
    boolean adjustKeyerPreference(MorsePreferences::prefPos pos);
}




#endif /* MORSEMENU_H_ */
