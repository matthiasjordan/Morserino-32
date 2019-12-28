#include "MorseUI.h"
#include "MorsePreferences.h"
#include "MorseSound.h"

using namespace MorseUI;

void MorseUI::click() {
    MorseSound::pwmClick(MorsePreferences::prefs.sidetoneVolume);         /// click
}
