#include "MorseUI.h"
#include "MorsePreferences.h"
#include "MorseSound.h"

using namespace MorseUI;

void MorseUI::encoderClick() {
    MorseSound::pwmClick(MorsePreferences::prefs.sidetoneVolume);         /// click
}
