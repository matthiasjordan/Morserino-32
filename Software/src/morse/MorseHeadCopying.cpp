#include "MorseHeadCopying.h"
#include "MorseGenerator.h"
#include "MorseMachine.h"
#include "MorseDisplay.h"
#include "MorseKeyer.h"
#include "MorseMenu.h"
#include "MorsePlayerFile.h"

using namespace MorseHeadCopying;

namespace internal {
    void setupHeadCopying();

}


void MorseHeadCopying::setup()
{

}



boolean MorseHeadCopying::menuExec(String mode)
{
    MorseGenerator::startTrainer();
    internal::setupHeadCopying();

    if (mode == "a")
    {
//        MorsePreferences::currentOptions = MorsePreferences::headOptions;
    }
    else if (mode == "player")
    {
//        MorsePreferences::currentOptions = MorsePreferences::headOptions;
        MorsePlayerFile::openAndSkip();
    }

    MorseDisplay::getConfig()->autoFlush = false;

    return true;
}

void internal::setupHeadCopying()
{
    MorseGenerator::Config *genCon = MorseGenerator::getConfig();
    genCon->autoStop = true;
    genCon->effectiveTrainerDisplay = DISPLAY_BY_CHAR;
    genCon->wordEndMethod = MorseGenerator::LF;

}
