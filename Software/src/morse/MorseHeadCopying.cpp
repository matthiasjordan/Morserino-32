#include "MorseHeadCopying.h"
#include "MorseGenerator.h"
#include "MorseMachine.h"
#include "MorseDisplay.h"
#include "MorseKeyer.h"
#include "MorseMenu.h"
#include "MorsePlayerFile.h"

using namespace MorseHeadCopying;

void MorseHeadCopying::setup()
{

}

boolean MorseHeadCopying::menuExec(String mode)
{
    if (mode == "a")
    {
        MorseGenerator::setupHeadCopying();
//        MorsePreferences::currentOptions = MorsePreferences::headOptions;
    }
    else if (mode == "player")
    {
        MorseGenerator::setupHeadCopying();
//        MorsePreferences::currentOptions = MorsePreferences::headOptions;
        MorsePlayerFile::openAndSkip();
    }

    MorseGenerator::startTrainer();

    return true;
}
