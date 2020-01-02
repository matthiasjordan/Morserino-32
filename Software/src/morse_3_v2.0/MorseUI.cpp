#include "MorseUI.h"
#include "MorsePreferences.h"
#include "MorseSound.h"
#include "MorseDisplay.h"
#include "MorseKeyer.h"
#include "decoder.h"
#include "MorseGenerator.h"

using namespace MorseUI;

// define the buttons for the clickbutton library
ClickButton MorseUI::modeButton(modeButtonPin);  // initialize mode button
ClickButton MorseUI::volButton(volButtonPin);    // external pullup for this one

void MorseUI::setup()
{
    // Setup button timers (all in milliseconds / ms)
    // (These are default if not set, but changeable for convenience)
    modeButton.debounceTime = 11;   // Debounce timer in ms
    modeButton.multiclickTime = 220;  // Time limit for multi clicks
    modeButton.longClickTime = 350; // time until "held-down clicks" register

    volButton.debounceTime = 11;   // Debounce timer in ms
    volButton.multiclickTime = 220;  // Time limit for multi clicks
    volButton.longClickTime = 350; // time until "held-down clicks" register
}

void MorseUI::click()
{
    MorseSound::pwmClick(MorsePreferences::prefs.sidetoneVolume);         /// click
}

///////////////// a test function for adjusting audio levels

void MorseUI::audioLevelAdjust()
{
    uint16_t i, maxi, mini;
    uint16_t testData[1216];

    MorseDisplay::clear();
    MorseDisplay::printOnScroll(0, BOLD, 0, "Audio Adjust");
    MorseDisplay::printOnScroll(1, REGULAR, 0, "End with RED");
    MorseKeyer::keyTx = true;
    MorseGenerator::keyOut(true, true, 698, 0);        /// we generate a side tone, f=698 Hz, also on line-out, but with vol down on speaker
    while (true)
    {
        volButton.Update();
        if (volButton.clicks)
            break;                                                /// pressing the red button gets you out of this mode!
        for (i = 0; i < Decoder::goertzel_n; ++i)
            testData[i] = analogRead(audioInPin);                 /// read analog input
        maxi = mini = testData[0];
        for (i = 1; i < Decoder::goertzel_n; ++i)
        {
            if (testData[i] < mini)
                mini = testData[i];
            if (testData[i] > maxi)
                maxi = testData[i];
        }
        MorseDisplay::showVolumeBar(mini, maxi);
    } // end while
    MorseGenerator::keyOut(false, true, 698, 0);                                  /// stop keying
    MorseKeyer::keyTx = true;
}
